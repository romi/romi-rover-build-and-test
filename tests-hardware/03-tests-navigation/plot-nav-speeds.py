import numpy as np
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties

def show_nav_speed_graph(nav):
    time = nav[:,0]
    left_target = nav[:,1]
    right_target = nav[:,2]
    left_speed = nav[:,3]
    right_speed = nav[:,4]
    plt.clf()
    p1, = plt.plot(time, left_target, label='Left target speed')  
    p2, = plt.plot(time, right_target, label='Right target')  
    p3, = plt.plot(time, left_speed, label='Left output')  
    p4, = plt.plot(time, right_speed, label='Right output')  
    plt.xlabel('time')
    plt.ylabel('navigation speeds')
    fontP = FontProperties()
    fontP.set_size('xx-small')
    #plt.legend(handles=[p1, p2, p3, p4, p5, p6, p7],
    plt.legend(handles=[p1, p2, p3, p4],
               title='legend',
               bbox_to_anchor=(1.05, 1),
               loc='upper left', prop=fontP)
    plt.show()

values = np.loadtxt("nav-speeds.csv")
show_nav_speed_graph(values)
