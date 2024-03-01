import argparse
import time
from rcom.rcom_client import RcomClient

class CNC(RcomClient):
   
    def __init__(self, name = 'cnc', id = 'cnc'):
        super().__init__(name, id)
       
    def homing(self):
        self.execute('cnc-homing')
       
    def set_relay(self, index, value):
        params = {'index': index, 'value': value }
        self.execute('set-relay', params)
       
    def power_up(self):
        self.execute('power-up')
        
    def power_down(self):
        self.execute('power-down')
       
    def get_range(self):
        return self.execute('cnc-get-range')
       
    def moveto(self, x, y, z, speed):
        params = {}
        if x != None:
            params["x"] = x
        if y != None:
            params["y"] = y
        if z != None:
            params["z"] = z
        if speed != None:
            params["speed"] = speed
        cmd = { "method": "cnc-moveto", "params": params }
        self.execute('cnc-moveto', params)
       
    def get_position(self):
        return self.execute('cnc-get-position')
       
    def synchronize(self, timeout_seconds):
        return self.execute('cnc-synchronize', {'timeout': timeout_seconds})

    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--topic', type=str, nargs='?', default="cnc",
                    help='The regsitry topic')
    args = parser.parse_args()
    
    cnc = CNC(args.topic, args.topic)
    # Turn off battery charger
    cnc.set_relay(0, True)
    cnc.power_up()
    # Move to 1 meter
    cnc.moveto(1.0, 0, 0, 0.75)
    time.sleep(1)
    # Return to 4 cm before the homing
    cnc.moveto(0.04, 0, 0, 0.75)
    # Do homing
    #cnc.homing()
    cnc.power_down()
    # Recharge the battery
    cnc.set_relay(0, False)
