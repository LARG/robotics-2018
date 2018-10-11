# Running simulations

* * *

The UTNaoTool provides a number of simulation options for debugging various aspects of the codebase. In particular, the simulator is a good choice for testing and debugging high level behaviors or localization.

* * *

### Localization Simulator

This section describes the purpose and basic features of the localization simulation. To run a simulation, follow these steps:

1. Run the UTNaoTool and open the World window.
2. Select `LocalizationSim` from the dropdown.
3. (Optional) select a seed - this will be used to generate a pseudo-random path for your robot to follow.
4. Click the play button.

The robot will now move from point to point through the environment. It will pass simulated odometry readings and vision observations with random noise added through the memory blocks to the core localization module. The text area will show RMSE error in terms of distance from your robot's belief to the ground truth, as well as in terms of rotational error.

In the OpenGL window you will see a white robot moving around - this is the ground truth robot. Similarly, the orange ball represents the ground truth ball. All solid, static field objects are ground truth as well.

Your robot's belief is represented with a blue robot, a blue ball, and translucent field objects representing your robot's projected observations.

If you notice your robot getting lost, you may want to follow these steps to debug the issue:

1. Restart the simulator with the same seed, and skip to a frame just before your robot starts getting lost. You can do this by entering the frame number into the "Skip To" box in the World window, and then clicking the "Skip To" button. Note that if you restart the simulator it will automatically advance to the "Skip To" frame.
2. Pause the simulator and open the Text Log window. 
3. Select "Localization" from the dropdown in the Text Log window, advance the simulation a single frame, and then make sure you can see the log output from your robot for the current frame. Be sure you use lots of logging statements to help diagnose issues. You can narrow these down by using the log level ranges at the bottom left of the Log window. For example, if your logging statements look like this:

    ```cpp
    log(43, "Started processing at frame %i", frame_id);
    /* ... */
    log(45, "Position estimated at %2.2f,%2.2f", x, y);
    ```

  Then you can show only the first message by selecting start/end levels of 43. You can show both by setting the start/end to 43 and 45, respectively. Generally it is advisable to use lower numbers for coarser information and increase the log level along with the level of detail in your log messages.
4. With the simulator still paused, continue advancing the simulation one frame at a time until you identify the frame that caused the localization error to occur. You can now make changes, recompile, and restart the simulation with this seed at this frame repeatedly until you solve the issue.
5. To speed up the process of recompiling and restarting, you can start the tool with the `-w` option to load the World window immediately.

### Simulating without UI

You may want to run simulations without the UI loaded so that you can get a better sense of your algorithm's performance on a large number of random paths. You can do this by running the tool with the `-x` option and redirecting standard output to `/dev/null`:

```bash
    $NAO_HOME/build/tool/UTNaoTool --loc-sim 1>/dev/null
```
    
This will produce output similar to the following:

    ----------------------------------------------------------
    Running simulation with seed 1714636915
    Sim time: 0.13 seconds
    Default RMSE dist error: 8265.13, rot error: 104.18, steps: 2648
    Avg dist: 9861.02, Avg rot: 105.28, Avg steps: 3350.75
    ----------------------------------------------------------
    Running simulation with seed 1957747793
    Sim time: 0.16 seconds
    Default RMSE dist error: 9818.65, rot error: 104.53, steps: 3394
    Avg dist: 9852.54, Avg rot: 105.13, Avg steps: 3359.40
    ----------------------------------------------------------
    Running simulation with seed 424238335
    Sim time: 0.14 seconds
    Default RMSE dist error: 8930.99, rot error: 115.88, steps: 3098
    Avg dist: 9698.95, Avg rot: 106.92, Avg steps: 3315.83
    ----------------------------------------------------------
    Running simulation with seed 719885386
    Sim time: 0.22 seconds
    Default RMSE dist error: 14554.33, rot error: 112.75, steps: 4946
    Avg dist: 10392.58, Avg rot: 107.76, Avg steps: 3548.71
    ----------------------------------------------------------
    Running simulation with seed 1649760492
    Sim time: 0.12 seconds
    Default RMSE dist error: 6175.59, rot error: 92.92, steps: 2390
    Avg dist: 9865.45, Avg rot: 105.90, Avg steps: 3403.88
    ----------------------------------------------------------
    Running simulation with seed 596516649
    Sim time: 0.13 seconds
    Default RMSE dist error: 7694.68, rot error: 115.33, steps: 2671
    Avg dist: 9624.26, Avg rot: 106.95, Avg steps: 3322.44
    ----------------------------------------------------------

Each simulation yields four lines: 

1. The seed value used to randomly generate the robot's path.
2. The RMSE distance and rotational error for this particular simulation.
3. The running time for this simulation.
4. The average RMSE distance and rotational error over all simulations so far.

If a particular path yields a large amount of error, you can inspect it by copying the seed into the World window and running the localization simulator manually on the generated path.

### Simulation vs Reality

Keep in mind that the simulator is not a perfect representation of reality. It generates observations without any knowledge of the effectiveness of your vision, and cannot accurately represent all the kinds of noise that you'll observe in the real environment. While simulation can be a great tool for initial debugging and sanity checking, there is no substitute for testing with real data.

If you want to tweak the observations generated by the simulator to more accurately represent your robot's observation noise, see the [`ObservationGenerator`](https://github.com/LARG/robotics-2018/blob/master/tools/UTNaoTool/simulation/ObservationGenerator.cpp#L221-L253) class.
