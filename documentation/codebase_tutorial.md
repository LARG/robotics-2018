# Nao Tutorial

* * *

This tutorial is intended to provide some familiarity with the basics of the Austin Villa codebase for the purpose of controlling robot behaviors, vision, and localization. Any feedback or requests for clarification may be sent to [Josiah Hanna](https://github.com/jpha226).

Please refer to the [Nao Intro](class_setup.md) to run the initial robot setup before proceeding with this tutorial.

* * *

### Hello World

This section will walk you through creating a simple Hello World program on the Nao. This assumes you've completed the [nao setup](class_setup.md).

1. Open five terminal windows - this isn't always necessary, but will help with debugging.
2. SSH into the nao with the first one, and run `nao stop`. When the nao is stopped, run `naoqi`
3. In the second and third terminal windows, SSH into the nao and run `bin/motion` and `bin/vision`, respectively.
4. In the fourth window, run the tool: `~/nao/trunk/bin/tool`.
5. In the fifth window, navigate to `~/nao/trunk/core/python` on your machine.
6. Open `behaviors/main.py`. This script defines the different states that the robot is in, which are listed in the Files window in the tool. You are going to edit the `Playing` state.
7. Add a definition for a `run` method that makes two calls: `memory.speech.say('Hello, World!')` and `self.finish()`. The first command will call the text to voice engine. The second command will indicate that this task has been completed.
10. Upload your python scripts to the robot by clicking the "Send Python" button on the Files window in the tool. Alternatively, you can run the script `~/nao/trunk/build/copy_robot [Robot IP] python`
11. Restart python on the robot by clicking the "Restart Python" button in the Files window.
12. Run the main behavior by choosing "main" in the Files window and clicking "Run behavior"
13. Put the robot in `Playing` by clicking the "Playing" button in the Files window.
14. Your robot should say "Hello, World!". If it doesn't or runs into an error, check the terminal you ran `bin/vision` in to see any error output.

### Basic Tips

1. All of your work for this class will be executed via the `vision` binary. So to read the output of `cout/printf` or `print` you will need to view the console output from `bin/vision` when ssh'd into the nao.
2. Joint and Sensor enums are defined in `core/common/RobotInfo.h`, and robot dimensions in `core/common/RobotDimensions.h`. Joint values can be accessed in python using the Joint enum, like so: `core.joint_values[core.HeadPan]`. Similarly for sensor values: `core.sensor_values[core.centerButton]`.
3. To do any body movements, you need to turn stiffness on first: `commands.setStiffness()`
4. To walk, use `commands.setWalkVelocity(x, y, t)` where `x` is forward/backward [-1,1], `y` is left/right [-1,1], and `t` is turning angle [-1,1].
5.To move the head left or right, use `commands.setHeadPan(theta, time)` where `theta` is the absolute angle [-pi/2,pi/2], `time` is the amount of time to take for the command to finish. Pass a third parameter `True` to use a relative head angle.

### Code Structure

The UT Austin Villa codebase is organized into a system of modules and memory blocks. Modules can request or create memory blocks which can then be shared with other modules. Memory blocks are used for passing information between modules and processes through shared memory, as well as for streaming from the robot to the tool and for logging. Data is organized into frames. The Motion process is capable of reading sensors at 100Hz, and thus processes frames at this rate as well. The Vision process can read camera images at 30Hz. The Vision process controls the larger part of the codebase, including behaviors, so most code runs at this rate. Each frame, the robot must read in images, process images, run behaviors that react to the processed images and sensors, and update localization. Throughout the codebase you will see code written with the frame-by-frame paradigm.

### Behaviors

Behaviors organize the high level tasks that the robot will carry out. Behaviors rely on underlying vision- and motion-related modules for low level computations such as kinematics and object detection. Behaviors are written in Python because of its flexibility and ease of use. These can be found in `~/nao/trunk/core/python`.

All Python scripts originate in `init.py`. This script is responsible for requesting low-level computations from C++ modules, as requesting processing from python behaviors. After performing initial computations, the primary behavior's `processFrame` method is called, which then delegates to whatever secondary behavior is being used.

You may find it useful to organize separate behaviors for separate tasks and then call these behaviors from the tool. For example, in one assignment you must program the robot to follow the ball with its gaze. You might create a behavior named `gaze.py` in the directory `$NAO_HOME/core/python/behaviors`. After creating this file you will see `gaze` appear in the Behavior dropdown in the Files window of the tool. Clicking the "Run Behavior" button with this behavior selected will cause the behavior to run.

#### Tasks

Tasks are a basic construct that is provided by the codebase. Tasks are organized hierarchically, so a single task may consist of multiple subtasks. To set the behavior of a task, define its `run` method. To call the task, use its `processFrame` method.

#### State Machine

State machines are a basic construct provided by the codebase that you may find useful to take advantage of. They come in two flavors: `SimpleStateMachine` and `StateMachine`. The simple version takes a list of state names as its constructor argument and creates a structure with those state names as member variables. The main purpose of the `SimpleStateMachine` is to help track which state you're in and how long you've been in that state. When more advanced transition graphs are needed, the `StateMachine` class is appropriate.

`StateMachine` connects `Task` objects through transitions defined by `Event` objects. Sometimes it's necessary for a `Task` to access methods that are specific to state machines. This can be achieved by using the derived class `Node` in lieu of `Task`.

The state machine comes with a set of basic events and a Node definition from `state_machine.py`. Transitions can be defined in a state machine's `setup` method using the `self.trans`.

The basic event types are as follows:

1.  Null Event (`N`) - This event always fires.
2.  Completion Event (`C`) - This event fires when its node's `complete()` method returns `True`. By default, calling `finish()` on a node will cause the node to be complete.
3.  Iteration Event (`I(i)`) - This event fires after `i` frames have passed.
4.  Time Event (`T(t)`) - This event fires after `t` seconds have elapsed.
5.  Signal Event (`S(s)`) - This event fires after a signal `s` is posted with the `postSignal(s)` method.

Each frame the state machine will check all events originating from the current node. When an event fires the state machine will transition to the event's successive node. Thus, a statement like `self.trans(start, T(3.0), StandNode())` will tell the state machine to transition from the `start` node to a standing node after remaining in `start` for 3 seconds. Naming nodes is necessary when a particular node needs to be referenced multiple times. In some cases a node is referenced only once, so these can be chained together in the `trans` method: `self.trans(start, T(3.0), StandNode(), T(3.0), finish)`. The method will take any number of arguments, and expects them in the order "Node, Event, Node, Event, Node, ..." corresponding to their organization in the event chain.

The sample Playing state machine in `behaviors/sample.py` gives a quick example of a state machine implementation. This machine will stand, walk forward for a few seconds, then sit down.

#### Reading Sensors

At some point it will be necessary to access sensor values in your behavior scripts. These can be accessed directly through the `joint_values` and `sensor_values` arrays in the `core` module:
```python
import core
print "My head yaw value is %f!" % core.joint_values[core.HeadYaw]
```

Additionally, high level commands such as walking and standing can be called from the behavior scripts. These are available in `commands.py`.

### Vision

Unlike behaviors, vision processing is restricted to the C++ portion of the codebase. This is due to efficiency concerns; high-level languages like Python don't provide the low-level functionality available in C++ for manipulating memory, and these features are necessary for processing all of the pixels retrieved from the cameras each frame. 

By default your robots will run at 320x240 resolution for both the top and bottom camera. The vision module processes both cameras in sequence, starting with the top. Objects that are found by the top camera may be overwritten by the bottom. When an object is successfully found, the vision module is responsible for filling out information in the appropriate `WorldObject` instance. For example, we might place the following in the `core/vision/ImageProcessor.cpp` file:

```cpp
void ImageProcessor::detectBall() {
  int imageX, imageY;
  if(!findBall(imageX, imageY)) return; // function defined elsewhere that fills in imageX, imageY by reference
  WorldObject* ball = &vblocks_.world_object->objects_[WO_BALL];

  ball->imageCenterX = imageX;
  ball->imageCenterY = imageY;

  Position p = cmatrix_.getWorldPosition(imageX, imageY);
  ball->visionBearing = cmatrix_.bearing(p);
  ball->visionElevation = cmatrix_.elevation(p);
  ball->visionDistance = cmatrix_.groundDistance(p);

  ball->seen = true;
}
```

A behavior script may then check the `seen` variable for the ball object as follows:

```python
import core
ball = memory.world_objects.getObjPtr(core.WO_BALL)
if ball.seen:
  walkTowardBall()
```

Segmented color can be accessed via the segmented image arrays in the `RobotVisionBlock`. The ImageProcessor class provides a method for accessing the proper array for the current camera being processed. For example, the following code would produce a count of all orange pixels in the current image:

```cpp
auto total = 0;
// Process from left to right
for(int x = 0; x < 320; x++) {
  // Process from top to bottom
  for(int y = 0; y < 240; y++) {
    // Retrieve the segmented color of the pixel at (x,y)
    auto c = getSegImg()[y * iparams_.width + x];
    if(c == c_ORANGE)
      total++;
  }
}
printf("total orange pixels: %i\n", total);
```

### Creating a new memory block

The following checklist is provided as a rough guide for creating an entirely new memory block. This guide assumes you are adding the block for vision processing. Analogous steps may be taken for other modules. It is unlikely (though not impossible) that you will need to add an entirely new memory block for this course.

0. Create the block in the [memory](../core/memory) directory.
1. Add a line near the bottom of [generate_block_operations.py](../build/core/generate_block_operations.py) specifying the id of your block (first) and the class name of the block (second). For example:
  "vision_objects":"VisionObjectsBlock"
2. Add pointers, initializers, and get/add calls to MemoryCache.h and MemoryCache.cpp
3. Add to VisionBlocks.h
4. Add to vision/VisionModule::specifyMemoryDependency and vision/VisionModule::specifyMemoryBlocks
5. Add a pointer to tools/UTNaoTool/VisionWindow.h, add a `nullptr` initializer to the constructor of VisionWindow.cpp and fill in the getBlockByName call in VisionWindow::update in VisionWindow.cpp.

