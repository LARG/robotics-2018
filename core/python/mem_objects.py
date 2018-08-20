"""Initializes references to some memory objects."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import memory
import geometry
import core
from functools import partial


def init():
    """Initialize references to memory objects."""
    global world_objects, abs_parts, rel_parts, opponent_models
    world_objects = ObjectAccessor(memory.world_objects.getObjPtr, core.NUM_WORLD_OBJS)
    abs_parts = ObjectAccessor(partial(core.pythonC.getPose3DPtr, memory.body_model.abs_parts_), core.BodyPart.NUM_PARTS)
    rel_parts = ObjectAccessor(partial(core.pythonC.getPose3DPtr, memory.body_model.rel_parts_), core.BodyPart.NUM_PARTS)
    opponent_models = ObjectAccessor(memory.opponent_mem.getModel, core.MAX_OPP_MODELS_IN_MEM)
    update()


def update():
    """Update max_vel pointer."""
    global close_ball, max_vel
    x = memory.walk_param.main_params_.speedMax.translation.x
    y = memory.walk_param.main_params_.speedMax.translation.y
    max_vel = geometry.Point2D(x, y)


class ObjectAccessor:
    def __init__(self, func, max_index):
        self.func = func
        self.max_index = max_index
        self.objects = {i: None for i in range(self.max_index)}

    def __getitem__(self, i):
        obj = self.objects[i]
        if not obj:
            self.objects[i] = obj = self.func(i)
        return obj
