## romi-rover-build-and-test
| |master| |ci_dev| |
|:---|:---|:---|:---|:---|
|**build & test**|![Build Status](https://github.com/romi/romi-rover-build-and-test/workflows/CI/badge.svg?branch=master)|[![codecov](https://codecov.io/gh/romi/romi-rover-build-and-test/branch/master/graph/badge.svg)](https://codecov.io/gh/romi/romi-rover-build-and-test)|![Build Status](https://github.com/romi/romi-rover-build-and-test/workflows/CI/badge.svg?branch=ci_dev)|[![codecov](https://codecov.io/gh/romi/romi-rover-build-and-test/branch/ci_dev/graph/badge.svg)](https://codecov.io/gh/romi/romi-rover-build-and-test)| 
|**librcom**|![Build Status](https://github.com/romi/librcom/workflows/CI/badge.svg?branch=master)|[![codecov](https://codecov.io/gh/romi/librcom/branch/master/graph/badge.svg)](https://codecov.io/gh/romi/librcom) |![Build Status](https://github.com/romi/librcom/workflows/CI/badge.svg?branch=ci_dev)|[![codecov](https://codecov.io/gh/romi/librcom/branch/ci_dev/graph/badge.svg)](https://codecov.io/gh/romi/librcom)  
|**libromi**|![Build Status](https://github.com/romi/libromi/workflows/CI/badge.svg?branch=master)|[![codecov](https://codecov.io/gh/romi/libromi/branch/master/graph/badge.svg)](https://codecov.io/gh/romi/libromi)|![Build Status](https://github.com/romi/libromi/workflows/CI/badge.svg?branch=ci_dev)|[![codecov](https://codecov.io/gh/romi/libromi/branch/ci_dev/graph/badge.svg)](https://codecov.io/gh/romi/libromi) 

The purpose of this repo is to provide a single point of interaction to download, build, test, and optionally generate a code coverage test report for the romi-rover and all its dependencies.
Since the romi-rover uses CMake as it's build system, CMake is also used to download the project and dependencies in addition to building the rover.

This ReadMe file gives a detailed description of the download / build / test functionality as well as configuration / build information.

# Dependencies
Before continuing this project assumes you have CMake, GCC and Build essentials installed.
You'll also need to install the fluidsynth-dev package for development: **sudo apt install libfluidsynth-dev**

## Getting Started
As you have probably already noticed, this repo contains a number of submodules. The libraries used in this project all have a a dependency on the google test suite
as well as LCOV.
This project uses CMake to download these external dependencies required to build the romi-rover with minimal user intervention required.
Note: Everything is currently a debug build. This will change as the project progresses and is more thoroughly tested.

1) Clone this repo. Since it contains submodules use the command:   
**git clone --branch ci_dev --recurse-submodules https://github.com/romi/romi-rover-build-and-test.git**  
This will clone all the submodules, but it will leave the submodules in a detached state. This is normal. Don't panic! Leave that to me.
2) From a terminal in your favourite flavour of linux, change into the romi-rover-build-and-test directory.  
**cd romi-rover-build-and-test** 
3) Now we want to setup the out of source build:  
**mkdir my-build-directory**  
**cd my-build-directory**  
4) Configure the build. Everything is forced to Debug at present. For a basic build that will build the rover and testsuite use:  
**cmake ..**  
5) Build the project:  
**make**
6) Run all the tests in the project:  
**ctest -V**
7) To create a coverage report simply take the name of the unit tests package and add "_coverage" to the name, and build using make.
The coverage report is then created in the subdirectory of the library for which you've just build the report and can be viewed with your favourite browser
**make r_unit_tests_coverage**  
**firefox libr/r_unit_tests_coverage/index.html&**  
**make rcom_unit_tests_coverage**  
**firefox rcom/rcom_unit_tests_coverage/index.html&**  

and thats it! Simple huh?

Now you will notice that during the configure step (4) it took a little longer to configure than normal. This is because this is where the magic is happening! 
The supporting test software (google test, LCOV etc) are downloaded, and the required config files depending on your build type are created. **phew!**

## Now my repo looks busy. Whats going on?
The first thing you'll notice is that there is a lot more in your romi-rover-test-and-build directory. Good!
Let's explain the structure a bit.

### romi-rover-test-and-build/thirdparty/
When the cmake is run to download and configure the system, you don't necessarily want to re-download all the supporting 
libs every time you configure a build locally. To save your bandwidth, and a small portion of your sanity, 
supporting libs are downloaded to named folders at the root level under the <thirdparty>/ directory. 

The reason that they are downloaded to their own downloads area rather than keeping it all and neat in the <build-directory> 
is that often when building we'll re-configure cmake to create a different build type (debug, release, coverage etc), and often we'll completely clean out the <build-directory> 
Keeping the downloads in a separate download area saves re-downloading every time you do a new cmake.

### Submodules
The romi rover project is dependent on a number of libries.  

romi-rover-test-and-build/libr/  
romi-rover-test-and-build/librcom/

Each of these libraries is designed such that the individual library can be cloned and used independently of this super project,
however we recommend you use this project for any development. If you poke around in the submodule CMakeLists.txt files
you will notice that each will also download dependencies googletest, LCOV, and any other library on which submodule depends if necessary.
To remove any duplication, the dependencies of each submodule are only downloaded if the ***_SOURCE_DIRECTORY variables are unset.

# Making changes to submodules.
Working with submodules is a pretty simple practice, and requires no additional git command knowlege. **However** it does require extra through in the development process.  
The superproject (the project you are looking at now), is just a snapshot of the project at any moment linked to the commit hashes of it's subprojects.  
So lets look at a working practice for making a change to **libr** contained in this project.  
You've just cloned and built the project using the fantastically clear(!) instructions above. Now we want to make a change to libr.
1) The first thing we need to do is tell git what branch we're working on as submodules are cloned in a detached head state.  
**cd romi-rover-build-and-test/libr**  
**git checkout ci_dev**
2) Now we make our changes in libr. We build and test them (using the commands above), get the code reviewed (more on that later). 
Were happy that everything is good and want to push back to the repo. We do that using the normal git flow.
**git commit (whatever)**
**git push**
3) Now this is where things get interesting. If anyone else were to pull the super project at this point, and clone everything they would still get the old version of libr.
This is because, as we said above, the superproject is linked to specific commits of the submodule. So lets update this superproject to point to the 
new libr we just changed.  
**cd romi-rover-build-and-test**
4) If you check the git status at this point you'll see that the super-project knows there are changes in the submodule.  
**git status** returns info similar to  
	*modified:   libr (modified content)*  
5) Now lets update the super-project to point to the new libr we just changed. It's just a normal git workflow but with an extra little spice. 
See the explanation in the gotcha section below.  
**git add libr**  
**git commit (whatever)**    
**git push --recurse-submodules=on-demand**  

That's it!

## Gotcha!
Now there are a few potential gotchas to keep in mind. Remember that the super project points to a specific commit right?  
Well there are two main issues that can arise from not committing pushing **both** the submodule **and** the super-project.

1) You commit and push the submodule, but you run out tea, and after sorting out that emergency you forget to push the super-project.  
If this happens anyone who clones the super project will not get the latest and greatest submodule. 
This will not cause any significant issues since you will just get the latest superproject which was pushed after building and testing.  

2) You commit the submodule, and commit and push the super project, but don't push the submodule.  
Well. Now you've done it. Anyone who tries to clone this repo and get the changes will be in trouble since your changes are still
local and they have no way to get them. Please don't do this!

The solution to option 2 above is to append the "--recurse-submodules=check" or "--recurse-submodules=on-demand" flags to the git push statement.  
This will cause git to either check if all submodules have neen pushed or not and fail if any have not. 
If you use the on-demand option git will try to push any un-pushed submodules before the super-project is pushed. It will abort the push on failure.  
  
If you want to make this the default behaviour for your system you can simply add it to git config:  
    **git config push.recurseSubmodules on-demand**


## Setting up a Raspberry Pi

1. Install Raspberry Pi OS Lite 64-bits on an SD-card using the [RPi Imager software](https://www.raspberrypi.com/software/). In the configuration dialog of the installer, it's handy to immediately configure the WiFi connection and enable SSH so the Pi is ready to connect to the local network upon the first boot.  
2. Start the Raspberry Pi and in a shell (either you hook up a screen and keayboard, or you connect over SSH if it was enabled) and install the required software packages:

```
sudo apt install git cmake build-essential libi2c-dev libpng-dev libjpeg-dev python3-pip
```
The Pi comes with the `nano` text editor but you may want to install your preferred shell-based text editor. As an example, I use emacs (the one without the X-based GUI - noX): 
```
sudo apt install emacs-nox
```

4. Clone the git repository

```
git clone  --branch ci_dev --recurse-submodules https://github.com/romi/romi-rover-build-and-test.git
```

6. Depending on what you need, compile the code:

```sh
cd romi-rover-build-and-test
mkdir build
cd build
cmake ..
```

For a camera set-up:

```sh
make romi-camera
```

For a main node that contraols a CNC:

```sh
make rcom-registry romi-cnc
```
7. When using the CNC module, copy the config file:

For the 1000 mm XCarve
```sh
cp ~/romi-rover-build-and-test/config/default.json ~/config.json
```

For the Cablebot
```sh
cp ~/romi-rover-build-and-test/config/cablebot-v3.json ~/config.json
```

8. Startup Rcom Registry on boot:

```sh
sudo emacs /etc/rc.local
```

Add, before the `exit 0` line:

```sh
sudo -u romi /home/romi/romi-rover-build-and-test/build/bin/rcom-registry >> /home/romi/rcom-registry.log 2>&1 &
sleep 3
sudo -u romi /home/romi/romi-rover-build-and-test/build/bin/romi-config --config /home/romi/config.json  >> /home/romi/romi-config.log 2>&1 &
sleep 3
sudo -u romi /home/romi/romi-rover-build-and-test/build/bin/romi-cnc --session-directory /home/romi/cnc >> /home/romi/cnc/temp.log 2>&1 &
```

9. Install Python interface
```sh
cd ~/romi-rover-build-and-test/librcom/python/
python3 setup.py install --user

cd ~/romi-rover-build-and-test/python/
python3 setup.py install --user
```

