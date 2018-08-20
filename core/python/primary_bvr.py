#!/usr/bin/env python
"""Main high level behavior processFrame calls."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import core
from task import Task
import pose
import commands
import cfgstiff
import UTdebug
from memory import behavior_mem, game_state, robot_state, vision_frame_info
from memory import walk_request, kick_request
import util


lastState = None
currentState = None
currentTask = None


def processFrame():
    global currentState, lastState, currentTask
    commands.setHeadTilt()
    lastState = currentState
    currentState = game_state.state()

    if currentState == core.PLAYING and lastState != currentState and lastState != core.PENALISED:
        behavior_mem.timePlayingStarted = vision_frame_info.seconds_since_start

    if util.currentFrame() % 30 == 0:
        util.checkTemperatures()

    if areDistinct(currentState, lastState):
        if currentTask:
            currentTask.finish()
        currentTask = createStateTask(currentState)

    if util.checkFallen() and robot_state.WO_SELF != core.WO_TEAM_COACH and not game_state.isPenaltyKick:
        commands.setStiffness(cfgstiff.Zero)
        kick_request.abortKick()
        walk_request.noWalk()
        return

    currentTask.processFrame()


def areDistinct(state1, state2):
    if state1 == core.INITIAL and state2 == core.FINISHED:
        return False
    if state1 == core.FINISHED and state2 == core.INITIAL:
        return False
    if state1 == state2:
        return False
    return True


def createStateTask(state):
    if UTdebug.TRACE:
        states = ['undef', 'initial', 'ready', 'set', 'playing', 'testing',
                  'penalised', 'finished', 'falling', 'bottom', 'top', 'test',
                  'manual']
        print('Starting state:', states[state])

    if state == core.INITIAL:
        return Initial()
    if state == core.FINISHED:
        return Finished()
    if state == core.READY:
        return Ready()
    if state == core.PLAYING:
        return Playing()
    if state == core.TESTING:
        return Testing()
    if state == core.PENALISED:
        return Penalised()
    if state == core.SET:
        return Set()
    if state == core.FALLING:
        return Falling()
    if state == core.MANUAL_CONTROL:
        return ManualControl()
    raise Exception("Invalid state: %i" % state)


class Initial(pose.Sit): pass


class Ready(pose.Sit): pass
class Set(pose.Sit): pass
class Playing(pose.Sit): pass
class Penalised(pose.StandStraight): pass
class Finished(pose.Sit): pass
class Set(pose.Sit): pass
class Falling(Task): pass


def load(bvr):
    """Import desired behavior module."""
    import importlib
    m = importlib.import_module('behaviors.' + bvr)
    global Ready, Set, Playing, Testing
    if hasattr(m, 'Ready'): Ready = m.Ready
    if hasattr(m, 'Set'): Set = m.Set
    if hasattr(m, 'Playing'): Playing = m.Playing
    if hasattr(m, 'Testing'): Testing = m.Testing


class ManualControl(Task):

    def reset(self):
        self.otimer = util.Timer()
        self.stance = -1
        super(ManualControl, self).reset()

    def run(self):
        commands.setHeadTilt()
        commands.setStiffness()
        if behavior_mem.test_odom_new:
            self.otimer.reset()
            behavior_mem.test_odom_new = False
        if self.otimer.elapsed() > behavior_mem.test_odom_walk_time:
            if self.stance != core.Poses.SITTING:
                self.stance = core.Poses.SITTING
                return pose.Sit()
            return

        vel_x = behavior_mem.test_odom_fwd
        vel_y = behavior_mem.test_odom_side
        vel_theta = behavior_mem.test_odom_turn
        reqstance = behavior_mem.test_stance
        if reqstance != self.stance:
            self.stance = reqstance
            if reqstance == core.Poses.SITTING:
                return pose.Sit()
            elif not util.isStanding():
                return pose.Stand()
        if self.stance == core.Poses.STANDING:
            commands.setWalkVelocity(vel_x, vel_y, vel_theta)
