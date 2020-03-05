
# Overview

Oquam is both a small C++ library and an rcom application to control a
CNC and to convert a polygone path given by the user into a smooth,
continuous path.

The main functionnality is contained in liboquam. This library can be
used in other projects. It does not depend on rcom but does require
[libr](https://github.com/romi/libr).


# Background

We developed oquam to control the CNC of the Romi Rover, a weeding
robot for organic, market farms (https://romi-project.eu). Initially,
we were using Grbl running on an Arduino Uno to control the CNC. One
of the biggest problems we had was that the weeding arm would get
stuck in the soil and the CNC would loose its position. We wanted to
add encoders on the motors but this was not so easy to add to Grbl. In
addition, it seemed more natural to us to perform some of the more
complex path smoothening on the host instead of on the Arduino. This
opens up the possibility to use different types of motor drivers as a
back-end as well.


***The implementation of the position tracking and error handling
   mechanism is currently still ongoing.***

# Installation

Oquam is currently part of the
[romi-rover](https://github.com/romi/romi-rover) suite but it can be
installed separately using the standard cmake approach:

```bash
$ cd oquam
$ mkdir build
$ cd build
$ cmake ..
$ make
```

This creates both the *liboquam* library and the *oquam*
[rcom](https://github.com/romi/rcom) node. If you only need the
libary, you can run "make liboquam" instead of make.

# Arduino firmware

The directory Oquam contains the Arduino code. It can be uploaded
using the Arduino IDE.

As of writing, the current implementation is not complete, yet. Test
have been done with the Arduino Uno and the gShield. Check
[config.h](https://github.com/romi/romi-rover/blob/master/oquam/Oquam/config.h)
for more information about support hardware.


# Using the library

The header to include is <oquam/oquam.hpp>. The library's API is
defined in the abstract class Controller (see
[Controller.hpp](https://github.com/romi/romi-rover/blob/master/oquam/src/Controller.hpp))
and in the
[script.h](https://github.com/romi/romi-rover/blob/master/oquam/src/script.h)
header.

To use the library your application should instatiate a subclass of
Controller during the initialization of your program. The Controller
subclasses implement a specific hardware configuration (see
below). Following that, whenever you want the CNC to travel a certain
path, you should take the following steps:

* create a new script object,
* register all the points of the polygone path using *script_moveto()* - all points have absolute positions,
* call *controller->run()* with the script as argument, and
* delete the script when done using *delete_script()*. 

The following code shows a simple example to get started:  

```c++
#include <oquam/oquam.hpp>

const char *device_name = "/dev/ttyUSB0";
double xmax[3] = { 0.7, 0.7, 0.5 };   // in meter
double vmax[3] = { 0.1, 0.1, 0.005 }; // max speed in m/s
double amax[3] = { 0.3, 0.3, 0.03 };  // max acceleration in m/s²
double scale[3] = { 40000, 40000, -100000 }; // steps/m
double period = 0.014; // in seconds

// The points of the polygone
double p[][3] = {{0, 0, 0}, ... };

// The travel speed of each segment in m/s
double v[] = { 0.1, ... };

// The number of points
int n = sizeof(p) / (3 * sizeof(double));

int main(int argc, char **argv)
{
        Controller *controller = new OquamStepperController(device_name,
                                                    xmax, vmax, amax, deviation,
                                                    scale, period);
        script_t *script = new_script();

        for (int i = 0; i < n; i++)
                script_moveto(script,  p[i].x, p[i].y, p[i].y, v[i], i);

        controller->run(script);                
        delete_script(script);
        delete controller;
}
```

The path planner needs a few parameters in order to compute the smooth
curve:

* xmax: the dimensions of the CNC in meters,
* vmax: the maximum allowed speeds in the x, y, and z directions in m/s,
* amax: the maximum allowed accelerations in the x, y, and z directions in m/s²,
* deviation: the maximum allowed deviation from the theoretical path in meter.

If the deviation is zero, the CNC will reduce the speed to zero in all
the junction points of the path. For larger than zero deviations, the
CNC will curve in the junctions and perform a continuous travel.

The stepper controller also needs to know the following:
* scale: the number of motor steps per meter in each direction,
* period: the time interval with which the path will be sliced (see below).


## Triggers and delays

It is also possible to insert triggers into the script. Triggers allow
you to synchronise other actions with the traveling of the CNC, for
example, grabbing an image from a camera.

```c++
void do_trigger(void *userdata, int16_t arg)
{
        printf("Trigger %d\n", arg);
}

int main(int argc, char **argv)
{
        double delay = 1.0; // seconds
        // ...

        for (int i = 0; i < n; i++) {
                script_moveto(script,  p[i].x, p[i].y, p[i].y, v[i], i);
                script_trigger(script, do_trigger, NULL, i, delay);
        }
        
        // ...
}
```

During the execution of the script, the trigger's callback function
will be called with two arguments, a pointer to a user-defined data
structure and the trigger ID. Both arguments are passed to *script_trigger()*.

The last argument, "delay", tells the CNC to pause the executions of
the script for a given amount of time. If the delay is less than zero,
the CNC will wait indefinitely, or until
*controller->continue_script()* is called.

You can also add a delay into the script without a trigger using
*script_delay()*. This may be useful for example if you want to be
sure the CNC is completely at rest before doing an operation such as
grabbing the image.


## Direct commands

The controller also provides two methods that you can call directly
without creating a script:

* moveat: Move at a given speed in the x, y, and z direction
* moveto: Move to an absolute position


## Controller implementations

Currently, there are two implementations of the controller interface:

* OquamStepperController: A controller for stepper motors. This
  controller connects to an Arduino over a serial connection. The
  Arduino must run the Oquam firmware that is part of [this
  repository](https://github.com/romi/romi-rover/tree/master/oquam/Oquam).

* VirtualStepperController: A virtual stepper controller for testing
  purposes.

Both implementations are a subclass of StepperController, which itself
is a sublass of Controller.


# Using the rcom node

The oquam node makes the CNC functions available to other rcom
nodes. It exports the following commands:

* moveto: parameters: "x", "y", "z", "v".
  Example: { "command": "moveto", "x": "0.7", "v": 0.05 }

* travel: parameters: "path".
  Example: { "command": "travel", "path": [[0,0,0,0.1], [0.5,0,0,0.1]], [0.5,0.5,0,0.1], [0,0.5,0,0.1], [0,0,0,0.1]] }

* spindle: not implemented, yet

* homing: not implemented, yet



# Implementation

Consider the code below. We have a list of points that we want the
path to go through in the array p[]. For each segment going from
p[i-1] to p[i] the travel speed should be v[i].

```c
double p[][3] = {{0, 0, 0}, ... };
double v[] = { 0.1, ... };
int n = sizeof(p) / (3 * sizeof(double));
Controller *controller = new OquamStepperController("...",
                                                    xmax, vmax, amax, deviation,
                                                    scale, period);
script_t *script = new_script();

for (int i = 0; i < n; i++)
        script_moveto(script,  p[i].x, p[i].y, p[i].y, v[i], i);

controller->run(script);                
delete_script(script);
delete controller;
```

Two segments of the path are shown in the following figure: 

![](path1.svg)


In most cases, there will be a discontinuity at the speed at the
junction point that requires a very high (theoretically infinite)
acceleration. The change in speed corresponds to a change in value, a
change in direction, or both.

![](path2.svg)

In order to remove the discontinuity and create a smooth path with
acceptable accelerations, we replace the junction with a round
curve. Before the curve, the CNC may have to slow down so as to enter
the curve with a lower speed. During the curve, the speed has a
constant absolute value but changes direction. After the curve, the
CNC may have to speed up again to reach the desired speed.

The curve introduces an error, as the path no longer goes through the
junction point. When the path is calculated, the application must
therefore specify what the maximum allowed deviation is, that is, what
the maximum allowed distance is between the junction point and the
curve.

![](path3.svg)

For starters, the speed at the entry of the curve is the minimum speed
of the speed values in segment s[i] and s[i+1]. As we will see below,
the error made by the curve depends on the entry speed and of the
maximum force (read: acceleration) that the motors can apply. 

To define the curve, we have to compute the entry and exit points (the
points q0 and q1 in the figure below), the value of the speed
throughout the curve, and the acceleration that we have to apply. This
calculation is simplified by putting the origin in the junction point
and the xy-plane in the plane defined by the two segments. The x and
y-axes are placed as shown in the figure below.

![](path4.svg)

We will be using the indices 0 and 1, instead of i and i+1, to refer
to the properties before and after the curve.

In this new reference frame, the speed along the x-axis is constant,
*v_x0 = v_x1*. The speed along the y axis reverses, *v_y0 = -v_y1*.

          ∆vy = vy1 - vy0
           
The equation for the speed is (*vy0* and *a* have opposite
signs):

        vy1 = vy0 + a·∆t
        ⇒ ∆t = -2vy0/a                      (1)
          
The equation for the y position is:

        y = y0 + vy0·t + a.t²/2

When t = ∆t/2, y reaches its minimim ym  

        ⇒ ym = y0 + vy0·∆t/2 + a.(∆t/2)²/2, using (1) and develop
        ⇒ ym = y0 - vy0²/2a                                      (2)
          
The time it takes to follow the speed curve is the same as the time it
takes to follow the two segments of the original path that go through
the junction point. This follows from the fact the the speed along the
x-axis remains the same in both cases. Following the orginal straight
path and starting from y0 with a speed of vy0, the junction y=0 is
reached after a time ∆t/2:
          
         y0 + vy0·∆t/2 = 0 ⇒ y0 = vy0²/a                      (3)
          
(2) and (3) combined gives us that:

         ym = vy0²/2a
          
The error ym should be smaller than the maximim deviation d:

          ym < d
          ⇒ vy0²/2a < d
          ⇒ vy0 < √2ad                                          (4)
          
If the requested speed at the entry of the curve is larger than the
√2ad, the speed components in the xyz directions have to scaled
linearly to satisfy the constraint (4).
          
We already calculated the y coordinate of the entry and exit points in
(3). The x coordinates of the entry and exit points are:
          
          ∆x = vx0·∆t
          ⇒ ∆x = -2vy0.vx0/a,   using (1)           
          ⇒ x0 = -∆x/2 = vy0.vx0/a
             and x1 = ∆x/2 = -vy0·vx0/a

To obtain the acceleration to apply on the stepper motors, we have to
rotate the acceleration back from the reference frame above into the
coordinate space of the CNC.



## Acceleration - travel - deceleration - curve

Each segment of the polygone will be replaced by four sections: an
acceleration, a travel, a deceleration, and a curve section or, as we
named it in the code, an ATDC.

![](path5.svg)

In the travel section, the arm moves at the constant speed that was
requested for that segment. The acceleration brings the arm up to
speed from a stand-still or after a curve. The deceleration slow the
arm down to a stand-still or to the entry speed of the curve. During
the curve, a constant acceleration is applied to chenge the direction
of the arm movement. Any of the sections may be absent.

An exception that is handled separately is when the segment is too
short to reach the requested speed. In that case, the speed is
interpolated linearly between the entry speed and the exit speed.

It is possible also that the speed at the entry of a segment is too
high and the segment is not long enough to slow down to the maximum
entry speed of the curve at the end of the segment. In this case, We
must scale back the speed at the start of this segment. This change
will have to be propagated back to the previous segments. This will be
done during the backward traversal of the segments.

The conversion of a polygone consisting of segments with constant
speed produces a list of ATDC elements that describes the position and
speed as a continuous function. The acceleration is a step-wise
function with a capped maximum value.


# Stepper controller

The stepper controller implements an acceleration as a sequence of
small segments with constant speed. The speed from one segment to the
next increases or decreases as needed. To obtain the sequence of small
segments, the ATDC list is sliced and converted into a long list of
move commands.

![](path6.svg)

All this preparative work is done on the host PC. To execute the path,
the stepper driver will send the move commands to the Arduino over a
serial connection. The Arduino uses a circular buffer to store the
move commands and informs the host if the buffer is full and to try
sending data again a bit later.

The Arduino executes the move commands. Similar to the Grbl
implementation, it uses a timer interupt to generate the standard step
signals used by most stepper drivers. Again, like Grbl, it uses
[Bresenham's
algorithm](https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm)
to determine when a step pin should be raised.


## Parameters

### Communication speed & minimum duration of move actions

Oquam's protocol is based on textual commands sent over a serial
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


(Suppose we send a move action M[T,dx,dy,dz,id]. The length of a
message, for short moves is 20 bytes long including the newline
character at the end (ex. M[10,10,10,10,1000]). With a baudrate of
38400, the time to transfer the message is 20*8/38400 = 4.2 ms.)




