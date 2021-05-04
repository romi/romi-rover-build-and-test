function move_and_grab()
{
    echo $1 $2 $3
    
    x=`echo $1 \* 0.095 | bc -q -l`
    y=`echo $2 \* 0.095 | bc -q -l`
    zreal=`echo $3 \* 0.105 | bc -q -l`    
    zcnc=`echo $zreal / 2.0 | bc -q -l`
    
    x=`printf "%0.3f" $x`
    y=`printf "%0.3f" $y`
    zreal=`printf "%0.3f" $zreal`
    zcnc=`printf "%0.3f" $zcnc`
    echo "$x $y $zreal =$zcnc"
    
    rcom request cnc "{'command': 'moveto', 'x': $x, 'y': $y, 'z': -$zcnc}"
    sleep 1
    wget http://192.168.100.100:8080/service/camera/camera.jpg -O calibration-$x-$y-$zreal.jpg 
}

rcom request cnc "{'command': 'homing'}"

for ((k=0; k<4; k++));
do

    i=0
    for ((j=0; j<6; j++));
    do
        move_and_grab $i $j $k
    done


    j=6
    for ((i=0; i<6; i++));
    do
        move_and_grab $i $j $k
    done

    i=6
    for ((j=6; j>0; j--));
    do
        move_and_grab $i $j $k
    done

    j=0
    for ((i=6; i>0; i--));
    do
        move_and_grab $i $j $k
    done
done

