# Using the Nao and the UT Austin Villa Codebase

<a id="introduction"></a>

### Introduction

This tutorial provides instructions for installing the UT Austin Villa SPL codebase on your personal machine, factory resetting and deploying code to a robot, and general notes for robot usage and care. An additional [tutorial](codebase_tutorial.md) is available as well for issues related directly to software development.

<a id="personal"></a>
### Instructions for Installing the Codebase on Your Personal Linux Machine
Before you begin, consider the following checklist to make sure your machine is prepared for the installation.

#### Pre-install Checklist

1. [Ubuntu 16.04 64-bit](http://releases.ubuntu.com/trusty/ubuntu-16.04.5-desktop-amd64.iso). The code may work on other configurations with some adjustments to the setup procedure, but these aren't officially supported. The setup process will configure your machine for building 32-bit software, which can cause conflicts with other applications in Ubuntu. It is **strongly suggested** that you use a fresh Ubuntu install dedicated to RoboCup. This may be easiest to accomplish using a [virtual machine](https://www.virtualbox.org/wiki/Downloads), however a native install will run faster and more smooothly with your network card(s) and/or GPU.
2. Root (sudo) access to your installation machine.
3. A GPU that supports OpenGL 2.0 or higher.
4. An active high-speed internet connection for downloading software. Most software is downloaded from locations on the CS network, so performing the installation on the UT campus is preferable.
5. You will need to install Git to retrieve code from the repository: `sudo apt-get install git`.

#### Installation Procedure

1. Create and enter the install folder in your home directory: 

        mkdir -p ~/nao/trunk && cd ~/nao/trunk
2. Retrieve the codebase from github:

        git clone git@github.com:larg/spl.git .

3. The install script requires a python depenency, pexpect. Install it with:

	bash ~/nao/trunk/install/pre-install.sh
        
4. Run the codebase install script: 

        ~/nao/trunk/install/install
5. The previous step will add environment variable definitions to your `.bashrc` script. To load them, close the current terminal window and open a new one. Alternatively, you can run `source ~/.bashrc`.
6. Follow the instructions for [compiling the code](#compiling) and [setting up the robot](#initial_setup). You may wish to compile everything with `$NAO_HOME/build/compile everything` to ensure that the codebase install has succeeded.


<a id="compiling"></a>
### Compiling the Code

Once the codebase is installed, compiling the code is fairly straight-forward. The simplest approach to compiling is to navigate to `$NAO_HOME/build` and run `./compile everything`. Similarly, you can copy compiled code to the robot with `./copy_robot everything <robot ip address>`. You can even combine these steps into one with `./cpcompile everything <robot ip address>`.

The compiling and copying scripts accept a number of parameters to speed up builds or improve the user experience.

#### Compile

* Use the `--debug` flag to build in debug mode.
* Use the `--fast` flag to avoid reconfiguring. Do not use this flag when adding new source files!
* Use the `--sound` flag to generate a noise when compiling has finished.
* Use the `--clean` flag to remove the build directories of the specified interfaces.

#### Copy

* Use the `--debug` flag to copy debug binaries.
* Use the `--verify` flag to verify copied files using an md5 checksum (no copy).
* Use the `--copy-verify` flag to verify copied files using an md5 checksum (with copy).
* Use the `--sound` flag to generate a noise when copying has finished.

<a id="initial_setup"></a>
### Initial Robot Setup

#### Flashing

When you get a robot from repairs or from the factory, the first thing you'll want to do is flash it. We currently use Naoqi 2.1.4.13, so any robot running another version of Naoqi should be re-flashed.

1. Download the flashing software from the [Aldebaran website](https://community.aldebaran.com/en/resources/software/language/en-gb) or from [here](http://cs.utexas.edu/~AustinVilla/software/flasher-2.1.0.19-linux64.tar.gz).
2. Download the nao system image from the [Aldebaran website](https://community.aldebaran.com/en/resources/software/language/en-gb) or from  [here](http://cs.utexas.edu/~AustinVilla/software/opennao-atom-system-image-2.1.4.13_2015-08-27.opn).
3. Extract the flasher: `tar zxvf flasher-*`
4. Insert a USB stick that can be safely erased and run the extracted `flasher` script with `sudo`.
5. Select the system image you downloaded, check "Factory reset", and click "Write".
6. Wait for the system image to be written to your USB stick, then insert the stick into your nao in the back of the head.
7. Hold the Nao's chest button until it starts to flash blue. Wait up to 30 minutes for the flashing process to complete.

#### Installing the codebase

After the robot has been flashed, you will need to install the codebase onto it. 

1. Enter the `$NAO_HOME/install` directory and ensure that you have the file `vim.tar.gz` in this directory. Normally this is downloaded during your initial setup process, but you can find the file [here](http://cs.utexas.edu/~AustinVilla/software/vim.tar.gz) as well.
2. Shut down your robot, connect it to your laptop with an ethernet cable, and restart the robot.
3. Create a manual ethernet connection: IP: 169.254.1.75, Subnet Mask: 255.255.0.0, Gateway (if required): 0.0.0.0. If you're running linux on a virtual machine, create this connection on the host operating system.
4. When your robot finishes booting (it will usually start talking or stop flashing), press its chest button and it will say its IP address.
5. Run the following script with that address and the robot's 2-digit ID (usually taped on the back of the head): `$NAO_HOME/install/setup_robot --ip <IP_ADDRESS_YOUR_ROBOT_SAID> --id <ROBOT_HEAD_ID>`.
6. Wait for the script to finish, and then the robot's server will reboot. After the reboot the setup process is complete.

<a id="wireless"></a>
### Connecting with Wireless

It is usually simplest to connect to the lab robots over the wireless network. We use the SSIDs `naonet` and `naonet5g` (no password) for communicating with the robots.

1.  Connect to the appropriate wireless network using DHCP.
2.  Use the IP address 192.168.1.XX when accessing the robot, where XX is the robot number taped on the back of the head near the ear.

<a id="ethernet"></a>
### Connecting with Ethernet

It can sometimes be advantageous to connect to the robot with an ethernet cable rather than over wireless, particularly when transferring logs, or when there are latency issues. To do so you'll need to follow these steps:

1.  Create a manual connection: IP: 11.0.1.75, Subnet Mask: 255.255.255.0, Gateway (if required): 0.0.0.0. If you're running linux on a virtual machine, create this connection on the host operating system.
2.  Connect your machine with an ethernet cable to the robot. The robot's ethernet port is on the back of its head.
3.  Use the IP address 11.0.1.XX when accessing the robot, where XX is the robot number taped on the back of the head near the ear.

<a id="logging"></a>
### Logging to the Robot

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

<a id="streaming"></a>
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

An overview of the codebase is available on the [tutorial](codebase_tutorial.md) page. This page points to the areas of the codebase that you'll be using along with code snippets to get you started.  

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


### Other Things You Should Know

* When sshed into the Nao, you can type `nao stop` to stop naoqi, `nao start` to start naoqi and `killall -9 naoqi motion vision` to kill everything immediately. You know the robot is ready to use once it has said "vision".
*   Use the scripts in `$NAO_HOME/build` to compile (`compile`), copy (`copy_robot`), or perform both simultaneously (`cpcompile`).
* In most cases running `./compile robot tool` will take into account any changes made to the codebase for both the core libraries and the tool interface. In some advanced cases you may need to do a clean build for a particular interface. Adding the `--clean` option when running `compile` will remove all previously built objects to allow for a clean build. You can also delete the build directory: `rm -rf ~/nao/trunk/build/build`
* It is safe to leave the robot on as long as it is plugged in and its joints are loose (i.e. stiffness commands all set to 0). If the robot is holding a joint in place, then the joint is stiff and shouldn't be left on for long periods of time. Killing the `naoqi` process is a guaranteed way to turn off stiffness. Since this has the effect of turning off all joints on the robot, be sure it is in a sitting position when you do this.
* When the robot is in Initial or Finished, the right eye indicates ball visibility. If the top right eye is blue, then the ball is visible in the top camera. If the bottom right eye is blue then the ball is visible in the bottom camera. In these states the left eye indicates power and heat. If the top left eye is white, then the joints are all cool. Similarly, if the bottom left eye is white, then the robot's battery level is high. The top and bottom of the left eye will transition toward red as these conditions worsen.
* The robot will not walk unless the walk engine has been calibrated. Calibrations happen periodically while `naoqi` and `motion` are actively running. If the robot is not calibrated and it tries to walk, it will repeatedly say "Not Calibrated" and its ears will flash.
* * *

