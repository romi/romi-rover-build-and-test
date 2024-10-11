#!/usr/bin/env bash

IMAGE=$1

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $DIR

if [ "x$IMAGE" == "x" ]; then
    echo "Missing image file"
    exit 1
fi

if [ ! -e "grab.timestamp" ]; then
    echo "Creating config file"
    python3 convert-config.py
    touch grab.timestamp
elif [ "grab.json" -nt "grab.timestamp" ]; then
     echo "Updating config file"
     python3 convert-config.py
     touch grab.timestamp
fi

libcamera-jpeg -o $IMAGE --width $WIDTH --height $HEIGHT --quality $QUALITY -camera $DEVICE --autofocus-on-capture 2>&1 > /dev/null 


