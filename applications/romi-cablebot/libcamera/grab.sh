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

libcamera-jpeg -o $IMAGE -x grab.config -w $WIDTH -H $HEIGHT -q $QUALITY -d $DEVICE -D 0 -z 5 2>&1 > /dev/null 


