#!/usr/bin/env python

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import util
import UTdebug
import core

class BaseTask(object):
  def __init__(self, **kwargs):
    self._kwargs = kwargs
    self._parent = None
    if 'name' in kwargs:
      self._name = kwargs['name']
    else:
      self._name = None
    self._initializing = True
    self.reset()
    self._initializing = False

  def _startrun(self):
    if not self._started:
      self.start()
    if not self._finished:
      UTdebug.log(99, "%s: run()" % str(self))
      return self.run()

  def reset(self):
    if not self._initializing:
      self.trace("Reset")
    self._started = False
    self._finished = False
    self._aborted = False
    self._timer = util.Timer()
    self._iterations = 0
    self._frames = 0
    for k, v in self._kwargs.iteritems():
      if k == 'name': continue
      setattr(self, k, v)

  def start(self):
    if self._started: return
    self.trace("Starting")
    self._timer.start()
    self._started = True
    self._startFrame = util.currentFrame()
    self._iterations += 1

  def processFrame(self):
    self._frames += 1
    if UTdebug.TIME: UTdebug.stimer(str(self))
    self._startrun()
    if UTdebug.TIME: UTdebug.ttimer(str(self))

  def started(self):
    return self._started

  def finish(self):
    if self._finished: return
    self.trace("Finishing")
    self._finished = True

  def finished(self):
    return self._finished

  def abort(self):
    self.trace("Aborting")
    self._aborted = True
    self.finish()

  def aborted(self):
    return self._aborted

  def resetTime(self):
    self._timer.reset()

  def getTime(self):
    return self._timer.elapsed()
  
  def getFrames(self):
    return util.currentFrame() - self._startFrame

  def getIterations(self):
    return self._iterations

  def setParent(self, parent):
    self._parent = parent

  def trace(self, msg):
    UTdebug.taskTrace(msg, self)

  def __repr__(self):
    if self._name: return self._name
    return self.__class__.__name__

  def __str__(self):
    return self.__repr__()

  def getDepth(self):
    node = self
    depth = 0
    seen = set()
    while node._parent is not None:
      if node in seen: raise Exception("Parent-child cycle detected.")
      seen.add(node)
      depth += 1
      node = node._parent
    return depth

class Task(BaseTask):
  def __init__(self, **kwargs):
    super(Task, self).__init__(**kwargs)
    self._subtask = None
    self.chainIndex = None
    self._subtask = None
    self._chain = None
    self.postInit()

  def postInit(self): pass

  def setChain(self, chain):
    self._chain = chain
    for c in self._chain:
      c._parent = self

  def setSubtask(self, subtask):
    self._subtask = subtask
    self._subtask._parent = self

  def run(self):
    if not self._chain: return
    for s in self._chain:
      if not s.finished():
        s._startrun()
        self._subtask = s
        return
    self.finish()

  def processFrame(self):
    self._frames += 1
    if UTdebug.TIME: UTdebug.stimer(str(self))
    if self.aborted(): 
      if UTdebug.TIME: UTdebug.ttimer(str(self))
      return
    if self._subtask and not self._subtask.finished():
      self._subtask.processFrame()
    else:
      sub = self._startrun()
      if sub:
        if isinstance(sub, list): self.setChain(sub)
        else: self.setSubtask(sub)
    if UTdebug.TIME: UTdebug.ttimer(str(self))

  def abort(self):
    BaseTask.abort(self)
    if self._subtask: self._subtask.abort()

class MultiTask(BaseTask):
  def __init__(self, *tasks, **kwargs):
    self._subtasks = []
    self._chains = []
    self._chainIndexes = []
    self.idpChains = False
    if len(tasks) and 'name' not in kwargs: 
      kwargs['name'] = "Multi(" + ",".join(map(repr,tasks)) + ")"
    for t in tasks:
      self._chains.append([t])
      t._parent = self

    super(MultiTask, self).__init__(**kwargs)
    self.postInit()

  def postInit(self): pass

  def reset(self):
    super(MultiTask, self).reset()
    if self._initializing: return
    for chain in self._chains:
      for c in chain:
        c.reset()

  def run(self):
    if not self._chains: return
    if not self._subtasks:
      self._subtasks = [None] * len(self._chains)
    if not self._chainIndexes:
      self._chainIndexes = [0] * len(self._chains)
    ci = 0
    finished = True
    for c in self._chains:
      si = 0
      for s in c:
        self._chainIndexes[ci] = si
        if not s.finished():
          s._startrun()
          self._subtasks[ci] = s
          finished = False
          break
        si += 1
      ci += 1
    if finished: self.finish()

  def processFrame(self):
    self._frames += 1
    if core.instance.type_ == core.CORE_TOOL:
      self.trace("process frame")
    if UTdebug.TIME: UTdebug.stimer(str(self))
    if self.aborted():
      if UTdebug.TIME: UTdebug.ttimer(str(self))
      return
    someFinished = False
    allFinished = True
    for s in self._subtasks:
      if not s:
        someFinished = True
      elif s.finished():
        someFinished = True
      else:
        allFinished = False
        s.processFrame()
    if someFinished and self.idpChains: self._startrun()
    elif allFinished: self._startrun()
    if UTdebug.TIME: UTdebug.ttimer(str(self))

  def setChains(self, chains):
    self._chains = chains
    for chain in self._chains:
      for c in chain:
        c._parent = self

  def abort(self):
    BaseTask.abort(self)
    for s in self._subtasks:
      if s: s.abort()

  def __repr__(self):
    if self._name: return self._name
    return super(MultiTask, self).__repr__()

class NullTask(Task):
  def run(self):
    self.finish()
