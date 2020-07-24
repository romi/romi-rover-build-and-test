
PORT="/dev/ttyACM0"
BOARD="arduino:avr:uno"

if [ "x$1" != "x" ]; then
    PORT=$1
fi

arduino --upload ControlPanelLcdKeypad/ControlPanelLcdKeypad.ino \
        --board $BOARD \
        --port $PORT

