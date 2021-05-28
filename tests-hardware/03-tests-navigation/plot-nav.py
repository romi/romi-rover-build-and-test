import numpy as np
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties

def show_distance(nav):
    time = nav[:,0]
    x = nav[:,3]
    plt.clf()
    plt.plot(time, x)
    plt.xlabel('time')
    plt.ylabel('distance')
    plt.show()

def show_pid(nav):
    time = nav[:,0]
    target_speed = nav[:,7]
    measured_speed = nav[:,8]
    pid_output = nav[:,9]
    error_p = nav[:,10]
    error_i = nav[:,11]
    error_d = nav[:,12]
    control_in = nav[:,13]
    plt.clf()
    p1, = plt.plot(time, target_speed, label='Target speed')  
    p2, = plt.plot(time, measured_speed, label='Measured speed')  
    p3, = plt.plot(time, pid_output, label='PID output')  
    p4, = plt.plot(time, error_p, label='PID proportional error')  
    p5, = plt.plot(time, error_i, label='PID integrated error')  
    p6, = plt.plot(time, error_d, label='PID differential error')  
    p7, = plt.plot(time, control_in, label='Controller input')  
    plt.xlabel('time')
    plt.ylabel('pid')
    fontP = FontProperties()
    fontP.set_size('xx-small')
    #plt.legend(handles=[p1, p2, p3, p4, p5, p6, p7],
    plt.legend(handles=[p1, p2, p3, p4, p5, p6, p7],
               title='legend',
               bbox_to_anchor=(1.05, 1),
               loc='upper left', prop=fontP)
    plt.show()

values = np.loadtxt("nav.csv")
show_pid(values)
