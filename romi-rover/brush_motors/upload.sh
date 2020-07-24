
PORT="/dev/ttyACM1"
BOARD="arduino:avr:uno"

if [ "x$1" != "x" ]; then
    PORT=$1
fi

arduino --upload BrushMotorController/BrushMotorController.ino \
        --board $BOARD \
        --port $PORT

