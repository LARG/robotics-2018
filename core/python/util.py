"""Utilities for behavior code."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import core
import mem_objects
# import time
# import math
import copy
import UTdebug

from memory import vision_frame_info, sensors, odometry
from memory import robot_state, walk_request, game_state, behavior_mem

deepcopy = copy.deepcopy


def currentFrame():
    """Return ID of current vision frame."""
    return vision_frame_info.frame_id


def getPoseJoint(joint, pose, reversed=False, reverseRolls=False):
    if joint not in pose: return None
    if not reversed: return pose[joint]

    if joint == core.LHipYawPitch or joint == core.RHipYawPitch:
        return pose[joint]

    val = 0
    # get the joint value for the joint on the opposite side
    if joint >= core.LShoulderPitch and joint <= core.LElbowRoll:
        # left arm
        val = pose[joint + (core.RShoulderPitch - core.LShoulderPitch)]
    elif joint >= core.LHipYawPitch and joint <= core.LAnkleRoll:
        # left leg
        val = pose[joint + (core.RHipYawPitch - core.LHipYawPitch)]
    elif joint >= core.RHipYawPitch and joint <= core.RAnkleRoll:
        # right leg
        val = pose[joint - (core.RHipYawPitch - core.LHipYawPitch)]
    elif joint >= core.RShoulderPitch and joint <= core.RElbowRoll:
        # right arm
        val = pose[joint - (core.RShoulderPitch - core.LShoulderPitch)]

    if reverseRolls:
        # reverse the roll directions
        directionReversedJoints = [core.LHipRoll, core.RHipRoll,
                                   core.LAnkleRoll, core.RAnkleRoll,
                                   core.LShoulderRoll, core.RShoulderRoll,
                                   core.LElbowRoll, core.RElbowRoll,
                                   core.LElbowYaw, core.RElbowYaw]
        for i in range(1, len(directionReversedJoints)):
            if joint == directionReversedJoints[i]:
                return -val
    return val


class Dynamic(object): pass


class Timer(object):
    def __init__(self):
        self.reset()

    def start(self):
        self._start = vision_frame_info.seconds_since_start

    def stop(self):
        self._elapsed += vision_frame_info.seconds_since_start - self._start

    def reset(self):
        self._elapsed = 0.0
        self.start()

    def elapsed(self):
        return vision_frame_info.seconds_since_start - self._start + self._elapsed


def isStanding():
    """Return True if robot is currently standing, else False."""
    if core.instance.type_ == core.CORE_TOOLSIM: return True
    headZ = mem_objects.abs_parts[core.BodyPart.head].translation.z
    return headZ > 400


def gettingUp():
    return odometry.getting_up_side_ != core.Getup().NONE


def setFallCounter():
    tilt = sensors.getValue(core.angleY)
    roll = sensors.getValue(core.angleX)
    tiltFalling = core.DEG_T_RAD * 45
    rollFalling = core.DEG_T_RAD * 45
    if (abs(tilt) > tiltFalling):
        if tilt > 0:
            # on stomach
            if walk_request.tilt_fallen_counter_ < 0:
                walk_request.tilt_fallen_counter_ = 1
            else:
                walk_request.tilt_fallen_counter_ = walk_request.tilt_fallen_counter_ + 1
        else:
            # on back
            if walk_request.tilt_fallen_counter_ > 0:
                walk_request.tilt_fallen_counter_ = -1
            else:
                walk_request.tilt_fallen_counter_ = walk_request.tilt_fallen_counter_ - 1
    else:
        walk_request.tilt_fallen_counter_ = 0
    if (abs(roll) > rollFalling):
        if roll > 0:
            # on stomach
            if walk_request.roll_fallen_counter_ < 0:
                walk_request.roll_fallen_counter_ = 1
            else:
                walk_request.roll_fallen_counter_ = walk_request.roll_fallen_counter_ + 1
        else:
            # on back
            if walk_request.roll_fallen_counter_ > 0:
                walk_request.roll_fallen_counter_ = -1
            else:
                walk_request.roll_fallen_counter_ = walk_request.roll_fallen_counter_ - 1
    else:
        walk_request.roll_fallen_counter_ = 0

    UTdebug.log(10, "fall counter: ", walk_request.tilt_fallen_counter_, walk_request.roll_fallen_counter_)
    return tilt, roll, tiltFalling, rollFalling


def checkFallen():
    tilt, roll, tiltFalling, rollFalling = setFallCounter()

    UTdebug.log(80, "tilt,roll:", tilt, roll, "fallen counter: ",
                walk_request.tilt_fallen_counter_, walk_request.roll_fallen_counter_)

    # Check if sensors think we have fallen
    if (abs(walk_request.tilt_fallen_counter_) > 2 or abs(walk_request.roll_fallen_counter_) > 2):
        state = game_state.state()

        # set fall direction
        if (roll > rollFalling):
            odometry.fall_direction_ = core.Fall.RIGHT
            # speech:say("right");
        elif (roll < -rollFalling):
            odometry.fall_direction_ = core.Fall.LEFT
            # speech:say("left");
        elif (tilt > tiltFalling):
            odometry.fall_direction_ = core.Fall.FORWARD
            # speech:say("forward");
        elif (tilt < -tiltFalling):
            odometry.fall_direction_ = core.Fall.BACKWARD
            # speech:say("backward");
        else:
            # default option??
            UTdebug.log(0, "default fall is forward")
            odometry.fall_direction_ = core.Fall.FORWARD
            # speech:say("default");

        UTdebug.log(30, "fall detected in direction", odometry.fall_direction_, tilt, roll)

        # Get up only in playing or ready
        if state in [core.READY, core.PLAYING, core.MANUAL_CONTROL]:
            # and not keeper (keeper will decide on its own when to do get up)
            if (robot_state.WO_SELF != core.KEEPER):
                return True
            else:
                # if keeper is not diving, we can still call get up
                if (behavior_mem.keeperDiving == core.Dive.NONE):
                    return True
                else:
                    # wait for dive to tell us to get up
                    return False
    odometry.fall_direction_ = core.Fall.NONE
    return False


def checkTemperatures():
    """Log temperature for each joint."""
    for i in range(core.NUM_JOINTS):
        temp = sensors.getJointTemperature(i)
        if (temp > 65):
            # figure out stiffness
            stiff = 1.0
            if (temp > 75):
                pct = (temp - 75.0) / 10.0
                stiff = 1 - pct
                if (stiff < 0):
                    stiff = 0
            UTdebug.log(10, "Joint ", i, core.getJointName(i), " has temperature ", temp, " stiffness: ", stiff)


def calculateSlope(pt1, pt2):
    dx = pt2.x - pt1.x
    if abs(dx) < 0.01:
        dx = 0.01
    slope = (pt2.y - pt1.y) / dx
    return slope


def calculateSlopeFromPointAngle(pt, angle):
    pt2 = pt + core.Point2D(100, angle, core.POLAR)
    return calculateSlope(pt, pt2)


def setFallenCounter():
    pass


def sign(value):
    return 1 if value > 0 else -1
