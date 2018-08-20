"""Functions that request different motions."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import core
import cfgstiff
import math
from memory import walk_request, joint_commands

walk_max_vel_y = 90
walk_max_vel_x = 360

rswalk_max_vel_y = 200.0
rswalk_max_vel_x = 330.0
rswalk_max_vel_rot = 0.87


def stand():
    """Request basic bent-knee stand."""
    walk_request.stand()


def standStraight():
    """Request tall straight stand."""
    walk_request.standStraight()


def setWalkVelocity(velX, velY, velTheta):
    """Request walk at certain velocity."""
    scale = max(math.sqrt(velX * velX + velY * velY), 1.0)
    velX /= scale
    velY /= scale
    walk_request.setWalk(velX, velY, velTheta)


def setStiffness(cfg=cfgstiff.One, time=0.3):
    """Set stiffness for all joints."""
    if isAtStiffness(cfg):
        return

    for i in range(core.NUM_JOINTS):
        joint_commands.setJointStiffness(i, cfg[i])
    joint_commands.send_stiffness_ = True
    joint_commands.stiffness_time_ = time * 1000.0


def setHeadPanTilt(pan=0, tilt=-21, time=2.0, isChange=False):
    """Set robot's head pan (left-right) and tilt (up-down)."""
    setHeadTilt(tilt)
    setHeadPan(pan, time, isChange)


def setHeadTilt(tilt=-22):
    """Set head tilt (up-down)."""
    joint_commands.setHeadTilt(core.DEG_T_RAD * tilt, 200.0, False)


def setHeadPan(target_pos, target_time=2.0, isChange=None):
    """Set head pan (left-right)."""
    if isChange is None:
        isChange = False
    joint_commands.setHeadPan(target_pos, target_time * 1000.0, isChange)


def isAtStiffness(cfg):
    """Check that joints are at given stiffness values."""
    for i in range(core.NUM_JOINTS):
        stiff = core.joint_stiffness[i]
        error = abs(stiff - cfg[i])
        if error > 0.05:
            return False
    return True
