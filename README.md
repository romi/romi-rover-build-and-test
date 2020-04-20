# romi-rover-build-and-test
The purpose of this repo is to provide a single point of interaction to download, build, test, and optionaly generate a code coverage test report for the romi-rover and all it's dependencies.
Since the romi-rover uses CMake as it's build system, CMake is also used to download the project and dependencies in addition to building the rover.

This ReadMe file gives a detailed description of the download / build / test functionality as well as configuration / build information.

## Getting Started
As you have probably already noticed, this repo is small.... very small. It contains a master CMakefile.txt that will download all the git repos required to build the romi-rover with minimal user intervention required.

1) Clone this repo. 
2) From a terminal in your favourite flavour of linux, change into the romi-rover-build-and-test directory.
3) mkdir <build-directory>
4) cd <build-directory>
5) cmake ..  ( for a basic build that will build the rover and testsuite)
6) make
7) ctest -V

and thats it! Simple huh?

Now you will notice that during the configure step (5) it took a little longer to configure that normal. This is because this is where the magic is happening! The required repos are cloned, the supporting test software (google test, LCOV etc) are downloaded, and the required config files depending on your build type are created. *phew*

## Now my repo looks busy. Whats going on?
The first thing you'll notice is that there is a lot more in your romi-rover-test-and-build directory. Good!
Let's explain the structure a bit.

###_romi-rover-test-and-build/download/_

When the cmake is run to download and configure the system, you don't necessarily want to re-download all the supporting libs every time you configure a build locally. 
To save your bandwidth, and a small portion of your sanity, supporting libs are downloaded to named folders under the "download" folder. 

The reason that they are downloaded to their own downloads area, rather than keeping it all neat and tidy in the <build-directory> is this:
often when building we'll re-configure cmake to create a different build type (debug, release, coverage etc), and often we'll completely clean out the <build-directory> so keeping the downloads in a separate download area saves re-downloading every time you do a new cmake.
It also removes the duplicate libraries that would be created per build configuration.







