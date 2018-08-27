"""Functions used for initializing and running vision thread."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import os
import sys
import traceback
import core
core.init()  # this has to be run before anything else can be imported
import memory
import mem_objects
# import sys, os, traceback
import logging
logging.disable(logging.ERROR)
import lights
import primary_bvr as behavior
import cfgwalk
import cfgmap
import UTdebug
import cfglocalization


def init():
    """Initialize global information."""
    global firstFrame
    firstFrame = True
    initMemory()
    initNonMemory()
    print("Python initialized")


def runBehavior(bvr):
    """Load particular behavior file."""
    print("Running " + bvr + " behavior")
    behavior.load(bvr)


def initMemory():
    """Initialize memory objects."""
    memory.init()
    mem_objects.init()
    cfgwalk.initWalk()


def initNonMemory(initLoc=True):
    """Initialize non-memory globals."""
    core.CONFIG_ID = cfgmap.get_config_id()
    cfgwalk.initWalk()
    if initLoc:
        core.localizationC.reInit()
    core.localizationC.loadParams(cfglocalization.params)
    core.opponentsC.reInit()


def processFrame():
    """Perform computations for each frame of python behavior."""
    try:
        global firstFrame
        if firstFrame:
            memory.world_objects.init(memory.robot_state.team_)
            core.visionC.initSpecificModule()
            initNonMemory(initLoc=False)
            memory.speech.say("Vision")
            firstFrame = False

        core.visionC.processFrame()
        core.localizationC.processFrame()
        core.opponentsC.processFrame()
        processBehaviorFrame()
        lights.processFrame()
        core.instance.publishData()
    except:
        handle()


behaviorLoaded = False


def processBehaviorFrame():
    """Run loaded behavior processFrame."""
    global behaviorLoaded
    if not behaviorLoaded:
        runBehavior("sample")
        behaviorLoaded = True
    try:
        if memory.robot_state.WO_SELF != core.WO_TEAM_COACH:
            mem_objects.update()
        if core.TOOL:
            initMemory()
        core.pythonC.updatePercepts()
        behavior.processFrame()
    except:
        handle()


def handle():
    """Write Python errors to file for debugging."""
    lines = traceback.format_exception(*(sys.exc_info()))
    message = ''.join('!! ' + line for line in lines)
    if core.instance.type_ == core.CORE_TOOLSIM:
        print(message)
    else:
        UTdebug.log(0, 'PYTHON ERROR:')
        UTdebug.log(0, message)
    memory.speech.say("python")
    core.pythonC.is_ok_ = False
    with open(os.path.expanduser('~/error.txt'), 'w') as f:
        f.write(message)
