## Raspberry Pi 3 Buster/Bullseye Dockerfiles
This folder contains a dockerfile with built in ARM emulator for building and testing the romi rover project.
Tested on Ubuntu 20.04

### Pre-requisites: qemu-user-static. 
Install in the host system.
sudo apt-get update -y
sudo apt-get install -y qemu-user-static

### What you should know

1) user romi / pw romi
2) user root / pw root
3) build and run tests using user romi and sudo as necessary. File tests run as root can fail due to root permissions allowing writing anywhere.
4) Also included .devcontainer/devcontainer.json for use with vscode and the fantastic remote containers docker plugin (suggested)
5) Not all tests work at present due to Qemu limitations.
6) Mainly for development should be used as an additional check with x86 dev platform to run all tests.
7) Dockerfile / tests are a work in progress, tests are constantly being fixed for this dockerfile, but generally work on real hardware.

### Run container
The following command will run the docker container and will mount the current directory as /hostworkspace inside the container.
docker run -it -v ${PWD}:/hostworkspace --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --privileged pi3arm7debianbullseye /bin/bash
docker run -it -v ${PWD}:/hostworkspace --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --privileged pi3arm7debianbuster /bin/bash
docker run -it -v ${PWD}:/hostworkspace --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --privileged pi3arm64debianbuster /bin/bash

${PWD} can be replaced with a path to the romi_build_and_test project root, but it is suggested to keep all the code within the container.
32bit versions of debian have issues with a 64bit filesystem and a *lot* of tests will fail and you will get write failures.
Make sure ${PWD} or whatever directory you use has write permissions for all users.
Prefer the 64bit verion of buster.

## Running the tests
1) Run all tests except tests requiring socat as user romi
   ctest -V -E rcom_rcdiscover_unit_tests
   
2) Run rcdiscover tests as sudo
   sudo ctest -V -R rcom_rcdiscover_unit_tests