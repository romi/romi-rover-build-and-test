
* Install PlatformIO
* `export PATH=$PATH:$HOME/.platformio/penv/bin`
* Compile: `pio run`
* Upload: `pio run -t upload --upload-port /dev/ttyACM0 -e m0`
* Show debug messages: `picocom -b 115200 /dev/ttyACM0`

