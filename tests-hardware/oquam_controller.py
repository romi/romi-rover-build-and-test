import time
import serial
import json
import math

from romi_device import RomiDevice

class OquamController(RomiDevice):

    def __init__(self, device): 
        super(OquamController, self).__init__(device)
    
    def enable(self):
        self.send_command("E[1]")
    
    def disable(self):
        self.send_command("E[0]")
    
    def set_relay(self, value):
        if value == True:
            self.send_command("S[1]")
        elif value == False:
            self.send_command("S[0]")
        else:
            raise TypeError("Expected boolean")
            
    def homing(self):
        self.send_command("H")
            
    def configure_homing(self, axes):
        self.send_command("h[%d,%d,%d]" % (axes[0], axes[1], axes[2]))
    
    def wait(self):
        while True:
            status = self.send_command("I")
            if status[1] == 1:
                break
            time.sleep(1);
    
    def moveat(self, x, y, z):
        self.send_command("V[%d,%d,%d]" % (x, y, z))
    
    def move(self, dt, steps_x, steps_y, steps_z):
        self.send_command("M[%d,%d,%d,%d]"
                          % (dt, steps_x, steps_y, steps_z))
