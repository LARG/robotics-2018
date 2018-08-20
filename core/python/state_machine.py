#!/usr/bin/env python

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from task import Task, BaseTask
import util, UTdebug
import copy

class SimpleStateMachine(object):
  def __init__(self, *states):
    if not states:
      raise Exception("State machine can't be empty")
    self.states = dict()
    self.stateValues = set()
    for i in range(len(states)):
      state = states[i]
      self.states[i] = state
      self.stateValues.add(state)
      setattr(self, state, i)
    self.initial = states[0]
    self.state = self.initial
    self._timer = util.Timer()
    self.debug = False
    self.transition(self.initial)

  def timeSinceTransition(self):
    return self._timer.elapsed()

  def framesSinceTransition(self):
    return util.currentFrame() - self.startFrame

  def transition(self, state):
    oldstate = self.state
    if isinstance(state, int):
      istate = state
      if istate not in self.states:
        raise Exception("Invalid state index: %i" % istate)
      self.state = self.states[istate]
    elif isinstance(state, basestring):
      if state not in self.stateValues:
        raise Exception("State does not exist in the state machine: %s" % state)
      self.state = state
    else:
      raise Exception("Invalid state passed: %s" % str(state))
    self._timer.start()
    self.startFrame = util.currentFrame()
    if self.debug:
      print("transitioned from %s to %s" % (oldstate, self.state))

  def inState(self, *states):
    for state in states:
      if state in self.states and self.states[state] == self.state:
        return True
      if state == self.state:
        return True
    return False

  def isFirstFrameInState(self):
    return self.startFrame == util.currentFrame()

  def numStates(self):
    return len(self.states)

class StateMachine(Task):
  def __init__(self, submachine=None, **kwargs):
    self.submachine = submachine
    if self.submachine:
      self.setup = self.submachine.setup
      self.submachine._adt = self._adt
      self.submachine.trans = self.trans
      self.submachine._parent = self
      self.submachine.setStart = self.setStart
      self.submachine.setFinish = self.setFinish
    super(StateMachine, self).__init__(**kwargs)

  def reset(self):
    super(StateMachine, self).reset()
    self._node = None
    self._finishNode = None
    self._tnodes = {}
    self._presetup()
    self.setup()
    self._postsetup()

  def getNode(self, task):
    if not isinstance(task, BaseTask): raise Exception("Invalid task: %s" % task)
    if isinstance(task, Node): return task
    if task not in self._tnodes:
      if isinstance(task, StateMachine):
        self._tnodes[task] = MachineNode(task)
      else:
        self._tnodes[task] = TaskNode(task)
    return self._tnodes[task]
  
  def setup(self): pass

  def _add_transition(self, *args):
    si, ei, ti = 0, 1, 2
    if not self._node: self._node = self.getNode(args[si])
    while ti < len(args):
      source = self.getNode(args[si])
      event = args[ei]
      target = self.getNode(args[ti])
      target._parent = source._parent = target.machine = source.machine = event.machine = self
      if hasattr(event, '__call__'): event = event()
      source.events += [event]
      event.source = source
      event.target = target
      si += 2
      ei += 2
      ti += 2
      self._finishNode = target

  def setStart(self, node):
    if node == None: self._node = None
    else: self._node = self.getNode(node)

  def setFinish(self, node):
    if node == None or node == False: self._finishNode = None
    else: self._finishNode = self.getNode(node)

  trans = _adt = _add_transition
  add_transition = trans

  def run(self):
    newNode = True
    while newNode:
      newNode=  False
      if not self._node: return
      for e in self._node.events:
        if not e.started(): e.start()
        if not e.fired() and e.ready():
          signal = self._node.outSignal()
          e.target.receiveSignal(signal)
          e.fire()
          self.trace("Fired %s on node %s" % (str(e), str(self._node)))
          e.reset()
          self._node.finish()
          self._node = e.target
          self._node.reset()
          if self._node == self._finishNode:
            self.finish()
          else:
            newNode = True
          break
    self._node.processFrame()
  
  def __repr__(self):
    if self.submachine:
      return self.submachine.__class__.__name__ + "<M>"
    else:
      return self.__class__.__name__ + "<M>"
  
  def _presetup(self): pass
  def _postsetup(self): pass

class Event(object):
  def __init__(self, name=None):
    self._name = name or self.__class__.__name__
    self.source = None
    self.target = None
    self.reset()

  def getTime(self):
    return self._timer.elapsed()

  def negation(self):
    return NegationEvent(self)

  def started(self):
    return self._started

  def start(self):
    self._started = True

  def fired(self):
    return self._fired

  def fire(self):
    self._fired = True

  def reset(self):
    self._started = False
    self._fired = False
    self._timer = util.Timer()

  def __str__(self):
    return self.__repr__()

  def __repr__(self):
    return self._name

class LoopingStateMachine(StateMachine):
  def _postsetup(self):
    self.setFinish(None)

class NegationEvent(Event):
  def __init__(self, event, name=None):
    super(NegationEvent, self).__init__(name=name or event.__class__.__name__)
    self.event = event

  def ready(self):
    return not self.event.ready()

  def __repr__(self):
    if self.__class__ == NegationEvent or self.__class__.__name__ == "NegatedEventType":
      return "~" + repr(self.event)
    return self.__class__.__name__
 
def Negate(eventType):
  class NegatedEventType(NegationEvent):
    def __init__(self, **kwargs):
      super(self.__class__, self).__init__(eventType(**kwargs))
  return NegatedEventType

class SignalEvent(Event):
  def __init__(self, signal):
    super(SignalEvent, self).__init__()
    self.signal = signal

  def ready(self):
    value = self.source.outSignal() == self.signal
    if value:
      self.target.receiveSignal(self.signal)
    return value

class CompletionEvent(Event):
  def __init__(self, time=None):
    super(CompletionEvent, self).__init__()
    self.time = time

  def ready(self):
    if self.time:
      return self.source.complete() and self.source.getTime() > self.time
    return self.source.complete()

class FailureEvent(Event):
  def ready(self):
    return self.source.failure()

class NullEvent(Event):
  def ready(self):
    return True

class TimeEvent(Event):
  def __init__(self, time):
    super(TimeEvent, self).__init__()
    self.time = time

  def ready(self):
    return self.source.getTime() > self.time

class IterationEvent(Event):
  def __init__(self, iterations):
    super(IterationEvent, self).__init__()
    self.iterations = iterations

  def ready(self):
    return self.source.iterations() > self.iterations

def S(signal = None):
  return SignalEvent(signal)

I = IterationEvent
F = FailureEvent
N = NullEvent
T = TimeEvent
C = CompletionEvent

class Node(Task):
  def __init__(self, adapter=False, **kwargs):
    self.events = []
    self.machine = None
    self._inSignal = None
    super(Node, self).__init__(**kwargs)

  def processFrame(self):
    if self._subtask and self._subtask.finished() and self._queue:
      UTdebug.log(97, "finalizing queued completion for %s" % (str(self)))
      self.finish()
      return
    super(Node, self).processFrame()
    if self.aborted(): return

  def run(self): pass

  def reset(self):
    super(Node, self).reset()
    self._failure = False
    self._complete = False
    self._queue = False
    self._outSignal = None
    self._startFrame = None
    self._subtask = None

  def postFailure(self):
    self._failure = True

  def failure(self):
    return self._failure

  def finish(self):
    self._complete = True
    self._inSignal = None
    super(Node, self).finish()

  postCompleted = finish

  def queueCompleted(self):
    self._queue = True

  def complete(self):
    return self._complete

  def iterations(self):
    return self._iterations

  def outSignal(self):
    return self._outSignal

  def inSignal(self):
    return self._inSignal

  def postSignal(self, signal):
    self._outSignal = signal

  def receiveSignal(self, signal):
    self._inSignal = signal

class TaskNode(Node):
  def __init__(self, task):
    if not isinstance(task, BaseTask):
      raise Exception("Invalid task passed to TaskNode")
    self.task = task
    self.task._parent = self
    super(TaskNode, self).__init__()

  def run(self):
    self.task.processFrame()
    if self.task.finished(): self.finish()

  def reset(self):
    super(TaskNode, self).reset()
    if not self._initializing:
      self.task.reset()

  def __repr__(self):
    return "TaskNode(%s)" % self.task.__repr__()

class MachineNode(Node):
  def __init__(self, machine=None, **kwargs):
    super(MachineNode, self).__init__(**kwargs)
    if machine: 
      self.submachine = machine
    else:
      self.submachine = StateMachine(self)
    self.submachine._parent = self

  def run(self):
    if self.submachine.finished():
      self.finish()
    else:
      self.submachine.processFrame()

  def setup(self): pass

  def reset(self):
    super(MachineNode, self).reset()
    if not self._initializing:
      self.submachine.reset()

  def __repr__(self):
    return "MachineNode(%s)" % self.submachine.__repr__()

class EventNode(Node):
  def __init__(self, node):
    super(EventNode, self).__init__()
    self.node = node

  def run(self):
    self.runSignal()
    self.finish()

class FailureNode(EventNode):
  def runSignal(self):
    self.node.postFailure()

class CompletedNode(EventNode):
  def runSignal(self):
    self.node.finish()

class SignalNode(EventNode):
  def __init__(self, node, signal):
    super(SignalNode, self).__init__(node)
    self.signal = signal

  def runSignal(self):
    self.node.postSignal(self.signal)
