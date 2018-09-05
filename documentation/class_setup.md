# CS393R: Autonomous Robots
## Using the Nao and the UT Austin Villa Codebase

All assignments for this course will revolve around programming humanoid Nao robots using a stripped version of the UT Austin Villa codebase. The codebase includes infrastructure for cross-compiling, reading and processing sensor data, sending actuator commands, and debugging low- to high-level robotics code.

### Initial Setup on Lab Machines

You should use these steps to obtain, setup, and run code on the lab machines and robots for the first time. Note that these steps do **not** work on the departmental machines. See the bottom of this page for instructions if you want to try and install the codebase on your personal Linux machine.

1. Read the [Robot Care](#care) section below.
2. Copy and run the `lab_setup` script from my home directory to yours with the following command. **This script will delete your ~/nao directory**.

        cp /home/jphanna/nao/trunk/install/lab_setup ~ && ~/lab_setup
3. This will add environment variable definitions to your `.bashrc` script. To load them, close the current terminal window and open a new one.
4. Go to the nao build directory at `~/nao/trunk/build` and compile the core libraries and the tool.

        cd ~/nao/trunk/build
        ./compile all 

<a id="starting_up"></a>
#### Starting up the Robot

1. Get your robot out of its locker, and place it on the ground in a sitting position. Plug it in. Turn it on by pushing its chest button. Lights should come on in the eyes.
2. Connect to the lab wireless network as described [here](#wireless). Your robot's wireless address will be 10.202.16.XX, and its ethernet address will be 11.0.1.XX, where XX is the robot number. This is taped on the back of the robot's head near the ear. In the following instructions replace &lt;robot ip&gt; with the applicable IP address.
3. In a new terminal, ping your robot by typing `ping <robot ip>`.
4. Set up passwordless ssh for your robot:

        ssh-keygen
        ssh-copy-id nao@<robot ip>
        nao # Enter this password when prompted

5. Now you should ssh into your robot by typing `ssh nao@<robot ip>`. Once here, type `nao stop` to stop naoqi.
6. Using the terminal you used to run the build scripts, copy your code to the robot with `./copy_robot <robot ip> everything`
7. Now you can go back to the terminal you used to ssh into your robot. Type `nao start`. The robot will eventually say interface and then vision. Once it has said vision, you know it has completely booted.
8. Run `$NAO_HOME/bin/tool` and open the Files window. Select your robot ip address from the drop-down menu in the upper right corner of the skinny files window. Press the 'Playing' button to have the robot follow a simple routine in which it randomly walks and turns for a short period of time.
9. When you're done with the robot, turn it off by holding its chest button down for ~10 seconds.

<a id="wireless"></a>
### Connecting with Wireless

The robots can be accessed over wireless through either the lab machines or your personal machines. Your robot's IP address is determined by the number on the back of its ear; for example, robot `45` has IP address `10.202.16.45`. 

When connecting with your personal machines, you'll need to connect to the robolab wireless network. The SSID is `robolab-cs393r-3.710a` and the psk can be found in `~/nao/trunk/data/scripts/wpa_supplicant.conf`. The wireless network does not broadcast its SSID so you will need to configure it as a hidden network.

If you need to update your robot's wireless configuration, connect to it over ethernet and run the following commands, replacing XX with the robot id:
```bash
cd ~/nao/trunk/install
./setup_robot --ip 11.0.1.XX --id XX --wireless-only
```

<a id="ethernet"></a>
### Connecting with ethernet from your personal machine

It can sometimes be advantageous to connect to the robot with an ethernet cable rather than over wireless, particularly when transferring logs, or when there are latency issues. To do so you'll need to follow these steps:

1.  Create a manual connection: IP: 11.0.1.75, Subnet Mask: 255.255.0.0, Gateway (if required): 0.0.0.0. If you're running linux on a virtual machine, create this connection on the host operating system.
2.  Connect your machine with an ethernet cable to the robot. The robot's ethernet port is on the back of its head.
3.  Use the IP address 11.0.1.XX when accessing the robot, where XX is the robot number taped on the back of the head near the ear.

### Connecting with ethernet from the lab machines

1. Connect an ethernet cable from the back of the robot's head to the unused secondary ethernet port on the lab machine. This is the port that is attached via a PCI card on the bottom of the back of the machine. **Do not unplug the ethernet cable from the primary ethernet port** which is attached through the motherboard.
2. Select the ethernet icon from the top right of the Ubuntu desktop and select the Nao ethernet connection.
3.  Use the IP address 11.0.1.XX when accessing the robot, where XX is the robot number taped on the back of the head near the ear.

### Logging to the robot

Following are instructions to capture logs and download them from the robot to your `$NAO_HOME/logs` folder. Note: your robot should be on, naoqi should be running, and the desired code should already be uploaded before following these instructions.

1.  If you've already captured logs, you may find it helpful to remove old logs from the nao. To remove all logs on the robot, open the Files window and click the "Remove Logs" button.
2.  Go to `$NAO_HOME` and open the tool by typing `bin/tool`. 
3.  Click on the Files window and select your robot from the dropdown. Verify that the robot shows up as ALIVE.
4.  Click on the Log Select button.
5.  Select what you want to log in the Log Select window.
6.  Click the Send To button to send the modules to log to the robot.
7.  Select a number of frames to log.
8.  Click the "Logging is OFF" button to begin logging an indefinite number of frames. To log a specific number, set the integer box on the right to the number of frames you wish to log. To set a minimum delay between log frames, set the Log Frequency value to the number of seconds you want the delay to be. This can be helpful when logging raw images, as they can quickly overflow the Nao's DMA buffer and cause the robot to process frames slowly. 
9.  If you've selected a frame count, logging will stop automatically when that number of frames has been reached. Otherwise, you must manually stop the logging process by clicking the "Logging is ON" button.
10.  In the Files window, click the "Get Logs" button to retrieve logs from the robot.

### Streaming

Streaming is similar to logging, except that the memory is streamed to your computer rather than directly to a file. Additionally, the stream can be logged locally for later viewing. Most laptops can write larger logs for longer periods of time than the robots are capable of, and so streaming your logs is usually easier and less error-prone than logging to the robot.

1.  Go to `$NAO_HOME` and open the tool by typing `bin/tool`. Click on the Log Select button.
2.  Select what you want to log in the Log Select window.
3.  Click on the Files window and select your robot from the dropdown. Verify that the robot shows up as ALIVE.
4.  Click "Stream" in the main tool. Verify that the stream is active and that the TCP connection has been established by opening the State window and verifying that the frame numbers are incrementing.
5.  If you want to log the stream, check/uncheck the "Log Stream" checkbox in the main tool window. Stream logs will show up in your `$NAO_HOME/logs` folder with the prefix `stream_`.
6.  If you want to just view the robot's memory, open any of the various tool windows as normal. They will automatically populate with data streamed from the robot.

<a id="color-table"></a>
### How to Make a Color Table

1. Go to `~/nao/trunk` and open the tool by typing `bin/tool`.
2. Open a log by going File->Open Log in the UTNaoTool and selecting the appropriate log.
3. Click the Vision button in the UTNaoTool main window.
4. Go to the Layers tab and click on one of the segmented images on the top right. Each pair of images is a different layer for the top and bottom cameras. Select the image for the camera that you wish to make a color table for.
5. Go to the "C Tables" tab and check "Enable Classification".
6. Select a color and left-click on the large image. All YUV values that are similar to the pixel you've selected will be assigned to the color you selected. You can adjust the size of the ball with the dials in the C Tables tab. If you're happy with the pixels that have been reclassified, right-click the image to commit the change to the color table. If you skip the right-click step and left-click again this will undo the changes from the previous left-click.
7. Save your table with the Top/Bottom Table menu.
8. Transfer your tables to the robot using the "Send ctable" button in the Files window, or by using the `copy_robot` script with the argument `color_table`.

### Tutorial

An overview of the codebase is available on the [tutorial](tutorial.md) page. This page points to the areas of the codebase that you'll be using along with code snippets to get you started.  

<a id="care"></a>
### Robot Care

The Nao robots are expensive machines that can break fairly easily from falling or overheating. Each joint needs to work precisely for the robot to move smoothly and receive input from its sensors, so even subtle damage can significantly affect the robot. Please follow these guidelines when working with your robot:

1. Keep the robot in a sitting position with stiffness off when not in use. You can tell that stiffness is off because the joints are easily flexible. If the robot's joints are stuck with stiffness enabled due to a crash, you can kill the `naoqi` process to disable stiffness completely. **Killing `naoqi` will cause the robot to collapse**, so be sure you are holding the robot when you do this.
2. Always have the robot in a sitting position with its arms to its side when you are starting it up.
3. Monitor the robot's battery and heat sensors and be sure to let it cool down by disabling joint stiffness when not in use, and keep it charged by plugging it in when it isn't moving.
4. Avoid commands that move the joints excessively fast.
5. Carry the robot by holding its abdomen. Holding it by the shoulders or extremities can wear down the joints.
6. Whenever the robot is moving, you or your partner should be next to it, ready to catch it if it falls.
7. If you need to stop the robot while it's in PLAYING mode, press its center chess button to put it into PENALISED. This will cause the robot to sit.
8. Only turn the robot on when it is on the **carpeted area** of the lab.

<a id="personal"></a>
### Instructions for Installing the Codebase on Your Personal Linux Machine

To install the codebase on your personal machine, you should be running</a> [Ubuntu 16.04 64-bit](http://releases.ubuntu.com/trusty/ubuntu-16.04.5-desktop-amd64.iso). The code may work on other configurations with some adjustments to the setup procedure, but these aren't officially supported. The setup process will configure your machine for building and running 32-bit software, which can cause conflicts with other applications in Ubuntu. It is **strongly suggested** that you use a fresh Ubuntu install for this. This may be easiest to accomplish using a [virtual machine](https://www.virtualbox.org/wiki/Downloads).

1. Read the [Robot Care](nao.html#care) section above.
2. Create and enter the install folder in your home directory: 

        mkdir -p ~/nao/trunk && cd ~/nao/trunk
3. Retrieve the codebase from github:

        git clone https://github.com/LARG/robotics-2018.git .

4. Run the nao pre-install script:

	bash ~/nao/trunk/install/pre-install.sh

5. Run the nao install script: 

        ~/nao/trunk/install/install
6. This will add environment variable definitions to your `.bashrc` script. To load them, close the current terminal window and open a new one.
7. Compile the code base:

	cd ~/nao/trunk/build
	./compile all

8. Try running the tool to ensure that the build has completed and your libraries are configured: `~/nao/trunk/bin/tool`
9. Connect to the robolab wireless network using the SSID and password you received.
10. Follow the instructions for starting up the robot [above](#starting_up).

### Using Git

Git is a distributed version control system used for managing code repositories. I will be using Git in this class for distributing updates to the codebase. The easiest way to incorporate these updates, as well as manage your own code, will be to maintain your own private git repository and periodically pull from the class repository for updates.

1. First, you should create an account and a private repository at a git hosting site. [Bitbucket](bitbucket.org) and [GitLab](gitlab.com) are two examples of sites that provide these for free.
2. Let's assume your username is `student` and your repository is `roborepo`. You should now update your remote URLs so that you can easily push updates to your private repo and pull updates from the class repo. You can do this using these commands:

        cd ~/nao/trunk
        git remote set-url origin git@bitbucket.org:student/roborepo.git
        git remote set-url class https://github.com/utaustinvilla/robotics-2018.git
3. When you have changes to the codebase that you want to push to the git server, use these commands:

        cd ~/nao/trunk
        git commmit -am "Adding all of my latest changes: Updated main behaviors for Assignment 1, added comments."
        git push origin master # Push all updates into the master branch at the origin
4. When updates are posted to the class repository for new assignments, you can pull them into your repository like this:

        cd ~/nao/trunk
        git pull class master # Pull all updates from the master branch at the class repository
5. If you have a merge conflict run `git mergetool`. This will open a window with three versions of your file:
  a. LOCAL (left): This is the file you've been working with.
  b. BASE (middle): This is the most recent common ancestor of the two versions of the file.
  c. REMOTE (right): This is the version of the file that the server has.
The point of merging is to modify the BASE (middle) so that it reflects the changes you've made locally and the changes that have been made remotely. Once the middle file is correct, you can save your changes and close. You can repeat this process for each file that needs to be merged. Once the merge is complete, you can commit your merge updates.

Git is a complex program that is capable of quite a bit more. [GitHub](https://help.github.com/articles/good-resources-for-learning-git-and-github/) has a helpful page that covers more of what is possible.

### Other Things You Should Know

* When sshed into the Nao, you can type `nao stop` to stop naoqi, `nao start` to start naoqi and `killall -9 naoqi motion vision` to kill everything immediately. You know the robot is ready to use once it has said "vision".
*   Use the scripts in `$NAO_HOME/build` to compile (`compile`), copy (`copy_robot`), or perform both simultaneously (`cpcompile`).
* In most cases running `./compile robot tool` will take into account any changes made to the codebase for both the core libraries and the tool interface. In some advanced cases you may need to do a clean build for a particular interface. Adding the `--clean` option when running `compile` will remove all previously built objects to allow for a clean build. You can also delete the build directory: `rm -rf ~/nao/trunk/build/build`
* It is safe to leave the robot on as long as it is plugged in and its joints are loose (i.e. stiffness commands all set to 0). If the robot is holding a joint in place, then the joint is stiff and shouldn't be left on for long periods of time. Killing the `naoqi` process is a guaranteed way to turn off stiffness. Since this has the effect of turning off all joints on the robot, be sure it is in a sitting position when you do this.
* When the robot is in Initial or Finished, the right eye indicates ball visibility. If the top right eye is blue, then the ball is visible in the top camera. If the bottom right eye is blue then the ball is visible in the bottom camera. In these states the left eye indicates power and heat. If the top left eye is white, then the joints are all cool. Similarly, if the bottom left eye is white, then the robot's battery level is high. The top and bottom of the left eye will transition toward red as these conditions worsen.
* The robot will not walk unless the walk engine has been calibrated. Calibrations happen periodically while `naoqi`, 'vision', and `motion` are actively running. If the robot is not calibrated and it tries to walk, it will repeatedly say "Not Calibrated" and its ears will flash.
* * *

