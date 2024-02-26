device="/dev/video0"

function all_default() {
    while read this_train
    do
        parameter=$( sed -rn 's/^ *([^ ]*) .*/\1/p;'            <<< $this_train)
        default=$( sed -rn "s/^.* default=([^ ]*) ?.*/\1/p;"    <<< $this_train)
        echo v4l2-ctl -d $device --set-ctrl=$parameter=$default
        v4l2-ctl -d $device --set-ctrl=$parameter=$default
    done <<< $(v4l2-ctl -d $device --list-ctrls | sed -r "/:/!d;")

    echo "Chattanooga choo choo, won't you choo-choo me home..."
    echo "The setting does not seem to be permanent until you play:"
    echo "https://www.bitchute.com/video/fgvFDTeeXzsb/"
    echo "A strange bug... :-)"

}
all_default
