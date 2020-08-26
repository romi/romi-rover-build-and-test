#!/bin/bash
# Copy all necessary files from the rom-rover binary directory specified on the command line
# Usage: copy-romi-rover-bin.sh <path-to-build-bin-dir>

rsync -av --progress $1/ ~/romi-rover/ --exclude={'*test*','*py*','*json*'}
