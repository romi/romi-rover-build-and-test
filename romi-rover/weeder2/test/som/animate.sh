#!/bin/bash

export LC_NUMERIC="en_US.UTF-8"

ALPHA=$1
BETA=$2
OUTPUT=$3

if [ "x$ALPHA" == "x" ]; then
    ALPHA=0.2
fi

if [ "x$BETA" == "x" ]; then
    BETA=2.0
fi

DATE=`date +"%Y%m%d-%H%M%S"`
TMPDIR="tmp-$DATE"
SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"



mkdir $TMPDIR
pushd $TMPDIR

$SCRIPTPATH/elastic --print --alpha $ALPHA --beta $BETA.1

COUNT=`ls path-*.txt | wc -l`

cp $SCRIPTPATH/cities.txt .
python3 $SCRIPTPATH/plot-animation.py


if [ "x$OUTPUT" == "x" ]; then
    OUTPUT="path-$DATE-$COUNT-$ALPHA-$BETA.mp4"
fi

ffmpeg -r 10 -i "path-%04d1.txt.png" ../$OUTPUT

popd

rm -rf $TMPDIR
