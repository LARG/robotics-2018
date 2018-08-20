"""Sample behavior."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import memory
import pose
import commands
import cfgstiff
from task import Task
from state_machine import Node, C, T, StateMachine


class Ready(Task):
    def run(self):
        commands.standStraight()
        if self.getTime() > 5.0:
            memory.speech.say("ready to play")
            self.finish()


class Playing(StateMachine):
    class Stand(Node):
        def run(self):
            commands.stand()
            if self.getTime() > 5.0:
                memory.speech.say("playing stand complete")
                self.finish()

    class Walk(Node):
        def run(self):
            commands.setWalkVelocity(0.5, 0, 0)

    class Off(Node):
        def run(self):
            commands.setStiffness(cfgstiff.Zero)
            if self.getTime() > 2.0:
                memory.speech.say("turned off stiffness")
                self.finish()

    def setup(self):
        stand = self.Stand()
        walk = self.Walk()
        sit = pose.Sit()
        off = self.Off()
        self.trans(stand, C, walk, T(5.0), sit, C, off)
