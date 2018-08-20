"""Simple keeper behavior."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import core
import commands
import mem_objects
from state_machine import Node, S, T, LoopingStateMachine
import UTdebug


class BlockLeft(Node):
    def run(self):
        UTdebug.log(15, "Blocking left")


class BlockRight(Node):
    def run(self):
        UTdebug.log(15, "Blocking right")


class BlockCenter(Node):
    def run(self):
        UTdebug.log(15, "Blocking right")


class Blocker(Node):
    def run(self):
        ball = mem_objects.world_objects[core.WO_BALL]
        commands.setHeadPan(ball.bearing, 0.1)
        if ball.distance < 500:
            UTdebug.log(15, "Ball is close, blocking!")
            if ball.bearing > 30 * core.DEG_T_RAD:
                choice = "left"
            elif ball.bearing < -30 * core.DEG_T_RAD:
                choice = "right"
            else:
                choice = "center"
            self.postSignal(choice)


class Playing(LoopingStateMachine):
    def setup(self):
        blocker = Blocker()
        blocks = {"left": BlockLeft(),
                  "right": BlockRight(),
                  "center": BlockCenter()
                  }
        for name in blocks:
            b = blocks[name]
            self.add_transition(blocker, S(name), b, T(5), blocker)
