
# Overview   

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


In most cases, there will be a discontinuity in the speed at the
junction point. This discontinuity requires a very high (theoretically
infinite) acceleration. The change in speed corresponds to a change in
value, a change in direction, or both.

![](path2.svg)

In order to remove the dicontinuity and create a smooth path, we
replace the junction with a round curve. During the curve, the speed
has a constant absolute value but changes direction. Before the curve,
the CNC may have to slow down so as to enter the curve with a lower
speed.  After the curve, the CNC may have to speed up again to reach
the desired speed.

The curve introduces an error, as the path no longer goes through the
junction point. When the path is calculated, the application must
therefore specify what the maximum allowed deviation is, that is, the
maxumum allowed distance between the junction point and the curve.

![](path3.svg)

For starters, the speed at the entry of the curve is the minimum speed
of the speed values in segment s[i] and s[i+1]. As we will see below,
the error made by the curve depends on the entry speed and of the
maximum force (read: acceleration) that the motors can apply. 

To define the curve, we have to compute the entry and exit points, the
value of the speed throughout the curve, and the acceleration that we
have to apply. This calculation is simplified by putting the origin in
the junction point and the xy-plane in the plane defined by the two
segments. The x and y-axes are placed as shown in the figure below. 

![](path4.svg)

We will be using the indices 0 and 1, instead of i and i+1, to refer
to the properties before and after the curve.

Using this reference frame, the speed along the x-axis is constant,
*v_x0 = v_x1*. The speed along the y axis reverses, *v_y0 = -v_y1*.

          ∆wy = wy1 - wy0 = -|_w0 - _w1| 
           
The equation for the speed is (wy0 and a have opposite
signs):

        wy1 = wy0 + a·∆t => ∆t = -2wy0/a                      (1)
          
The equation for the y position is:

    y = y0 + wy0·t + a.t²/2

When t = ∆t/2, y reaches its minimim ym  

     => ym = y0 + wy0·∆t/2 + a.(∆t/2)²/2, using (1) and develop
     => ym = y0 - wy0²/2a                                      (2)
          
The time it takes to follow the speed curve is the same as
the time it takes to follow the two segments of the original
path that go through the junction point. This follows from
the fact the the speed along the x-axis remains constant in
both cases. Following the orginal straight path for ∆t/2,
the junction at 0 is reached:
          
          y0 + wy0·∆t/2 = 0 => y0 = wy0²/a                      (3)
          
(2) and (3) combined gives: ym = wy0²/2a
          
The error ym should be smaller than the maximim deviation d.

    ym < d => wy0²/2a < d => wy0 < sqrt(2ad)               
          
If the requested speed is larger than the maximum, all
speed components will be scaled linearly.
          
We already calculated the y coordinate of the entry and exit
points in (3). The x coordinates of the entry and exit
points are:
          
          ∆x = wx0·∆t
          => ∆x = -2wy0.wx0/a,   using (1)           
          => x0 = -∆x/2 = wy0.wx0/a
             and x1 = ∆x/2 = -wy0·wx0/a

To obtain the acceleration to apply on the stepper motors,
we have to rotate the acceleration (0,ay,0) back into the
coordinate space of the CNC. 





