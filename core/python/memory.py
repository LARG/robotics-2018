"""Initialize references to memory blocks."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import sys
import core


class BlockWrapper(object):
    def __init__(self, block):
        self.block = block

    def __getattr__(self, name):
        cblock = self.block()
        return getattr(cblock, name)

__module__ = sys.modules[__name__]


def init():
    """Initialize references to memory blocks."""
    global behavior_mem, camera_block, game_state, kick_request, odometry, robot_state, sensors
    global vision_frame_info, walk_param, kick_params, walk_request, walk_response, world_objects, team_packets, opponent_mem
    global behavior_params, joint_commands, processed_sonar, al_walk_param, walk_info, robot_vision, body_model
    global robot_info, speech, localization_mem, image
    global joint_angles, audio_processing

    def behavior_mem(): return core.pythonC.behavior_
    def camera_block(): return core.pythonC.camera_block_
    def game_state(): return core.pythonC.game_state_
    def kick_request(): return core.pythonC.kick_request_
    def odometry(): return core.pythonC.odometry_
    def robot_state(): return core.pythonC.robot_state_
    def sensors(): return core.pythonC.sensors_
    def vision_frame_info(): return core.pythonC.vision_frame_info_
    def walk_param(): return core.pythonC.walk_param_
    def kick_params(): return core.pythonC.kick_params_
    def walk_request(): return core.pythonC.walk_request_
    def walk_response(): return core.pythonC.walk_response_
    def world_objects(): return core.pythonC.world_objects_
    def team_packets(): return core.pythonC.team_packets_
    def opponent_mem(): return core.pythonC.opponents_
    def behavior_params(): return core.pythonC.behavior_params_
    def joint_commands(): return core.pythonC.joint_commands_
    def processed_sonar(): return core.pythonC.vision_processed_sonar_
    def al_walk_param(): return core.pythonC.al_walk_param_
    def walk_info(): return core.pythonC.walk_info_
    def robot_vision(): return core.pythonC.robot_vision_
    def body_model(): return core.pythonC.body_model_
    def robot_info(): return core.pythonC.robot_info_
    def speech(): return core.pythonC.speech_
    def localization_mem(): return core.pythonC.localization_
    def joint_angles(): return core.pythonC.joint_angles_
    def image(): return core.pythonC.image_
    def audio_processing(): return core.pythonC.audio_processing_

    __blocks__ = [
        "behavior_mem", "camera_block", "game_state", "kick_request",
        "odometry", "robot_state", "sensors", "vision_frame_info", "walk_param",
        "kick_params", "walk_request", "walk_response", "world_objects",
        "team_packets", "opponent_mem", "behavior_params", "joint_commands",
        "processed_sonar", "al_walk_param", "walk_info", "robot_vision",
        "body_model", "robot_info", "speech", "localization_mem", "image",
        "joint_angles", "audio_processing"
    ]

    # If we're running logs from the tool then the memory block pointers change with each frame,
    # so we need to use the BlockWrapper class to look them up on demand. If we're on the nao,
    # the pointers never change and so it's more efficient to look them up on initialization.
    for b in __blocks__:
        f = getattr(__module__, b)
        if core.TOOL:
            setattr(__module__, b, BlockWrapper(f))
        else:
            setattr(__module__, b, f())


init()
