
# MotorController

The MotorController firmware manages the speed of two brushed DC
motors. It was designed for a rover with two motors and [differential
steering](https://en.wikipedia.org/wiki/Differential_steering) but may
be useful for any application that requires the speed control of
brushed DC motors.

It outputs two PWM signals on pins 9 and 10. These pins should be
connected to a driver board, basically a H-bridge type driver. By
default, the controller outputs a RC control compatible PWM signal but
this can be modified (currently requires changes the PWM period and
amplitudes in the code).

The motors should have incremental encoders for closed-loop control.

For the control it uses a PI controller.

The controller uses a maximum accelerator setting to assure smooth
speed transitions and avoid sudden jumps when wetting the target
speeds.

The controller uses the RomiSerial library to send commands.

The current version outputs RC control PWM signal and an update period
for the speeds of 20 ms (50 Hz). 

The controller uses integer math. Some care should be taken when
setting the values of the PI controller (see configuration below).

It's been used only on the Arduino Uno but should be compatible with
all ATmega328 based Arduino boards.

## Wiring & pins

| Pin | Type   | Motor | Function |
| --- | ------ | ----- | -------- |
| 2   | Input  | Left  | Encoder line A |
| 3   | Input  | Right | Encoder line A |
| 4   | Input  | Left  | Encoder line B |
| 5   | Input  | Right | Encoder line B |
| 9   | Output | Left  | PWM signal |
| 10  | Output | Right | PWM signal |

## Commands

| Opcode | Args | Description |
| ------ | ---- | ----------- |
| ?      | -    | Returns the firmware name and version |
| e      | -    | Returns the left and right encoder values and the current time |
| C      | 10   | Configures the controller (see details below). The controller can be configured only when it's just started or when it's disabled |
| E      | 1    | Enable (arg=1) or disable (arg=0) the motors. The controller must be configured before it can be enabled. |
| V      | 2    | Sets the target speeds for the left and right motors in 0.001 revolutions per second. For a target speed of 0.5 revolutions/second, send a value of 500 |
| X      | 0    | Immediatley stop the motors |
| v      | 0    | Returns the motor speeds (more info below) |
| T      | 1    | Runs the test suite (arg=0: show menu, arg=N>0: run test N |

### Configuration

The configuration requires ten arguments.

| Arg | Range     | Description |
| --- | --------- | ----------- |
| 0   | [1,32767] | The number of encoder steps for a full wheel turn |
| 1   | {1,-1}   | The direction of the left encoder: 1 means the encoder increments when the wheel turns forward, -1 the encoder decrements when turning forward |
| 2   | {1,-1}   | The direction of the right encoder |
| 3   | [1,32767] | The maximum allowed speed in 0.001 revolutions per second. Example, for 1 revolution per second, the value is 1000.  |
| 4   | [1,32767] | The maximum allowed acceleration in 0.001 rev./s². Example, for a maximum acceleration from zero to 1 rev./s in the span of one second (a=1 rev/s²), pass a value of 1000.  |
| 5 and 6 | [1,32767] | The value of the proportional coefficient (Kp) of the PI controller, specified as a numerator (arg 5) and a denominator (arg 6). |
| 7 and 8 | [1,32767] | The value of the integrator coefficient (Ki) of the PI controller, specified as a numerator (arg 7) and a denominator (arg 8). |
| 9   | [1,32767] | The maximum amplitude of the radio control PWM signal. (If unsure, use 500). |


### Obtains the speeds

The "v" command returns information about the current speed status of
the controller. It returns the following siw values:

| Index | Description |
| ----- | ----------- |
| 1,2   | Left and right target speeds, as requested by the user |
| 3,4   | Left and right current speeds. These speeds are computed from the user defined target speeds taking into account the maximum acceleration |
| 5,6   | Left and right measured speeds. These speeds are computed from the changes in the encoder values |



## Integer math

The integer math used by the controller limits the range of the PI
numerator and denominator. The values should be reduced as much as
possible. So, instead of passing the values 1700 and 1000 for the
numerator and denominator respectively, you should pass the value 17
and 10.

TODO: More info required.
