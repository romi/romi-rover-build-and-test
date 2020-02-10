
# Parameters

## Communication speed & minimum duration of move actions

Oquam's protocal is based on textual commands sent over a serial
connection for convenience. Serial makes it easy to connect a terminal
to the controller and type in a command. It also makes it easy to
upload new versions of the firmware.

Before discussing the parameters that define the communication speed
and the minimum duration of move actions, we will give some background
on the implementation.

The Oquam firmware does two things concurrently. First, it read and
parses incoming serial messages. Incoming actions are put into a
queue. Second, a timer interupt grabs the actions from the queue and
sends the pulses to the stepper drivers. The execution of the interupt
handler has a higher priority than the execution of the main code that
handles the incoming serial data. Also the execution time of the
interupt handler is variable. So, there's no easy way to know how
"fast" the main code will run, and how fast in can process incoming
serial data. 

The implementation of the serial handler on the Arduino uses a rather
small, 64-bytes circular buffer to store the incoming data. When the
buffer is full, incoming data will overwrite old data. The current
protocol cannot handle corrupted messages so this situation should be
avoided. Therefore, we artificially limit the baudrate to avoid
sending commands too quickly.

When, for a given path, the stepper must perform an acceleration, the
path planner will slice the path and transform the acceleration into a
sequence of short move actions with increasing speed. The time
interval that is used to slice the path influences how fast the
firmware must be able to read the messages. If this interval is too
short, the firmware will finish executing a move action before the
next one is scheduled and it will stop instead of accelerate as
desired. So, the slice interval should be larger than the minimum time
to parse a message.

Too summarize:

* If the baudrate is too high: the serial buffer overflow
* If the baudrate is too low: the messages arrive too slowly for smooth accelerations
* If the slice interval is too small: actions are executed faster than can be transmitted
* If the slice interval is too big: accelerations are not very smooth

Suppose we send a move action M[T,dx,dy,dz,id]. The length of a
message, for short moves is 20 bytes long including the newline
character at the end (ex. M[10,10,10,10,1000]).


