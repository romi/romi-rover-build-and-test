import argparse
from rcom.rcom_client import RcomClient

class Oquam(RcomClient):
   
    def __init__(self, name = 'cnc', registry = '127.0.0.1'):
        super().__init__(name, registry)
       
    def homing(self):
        self.execute('cnc-homing')
       
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
    parser.add_argument('--registry', type=str, nargs='?', default="127.0.0.1",
                    help='IP address of the registry')
    args = parser.parse_args()
    
    cnc = Oquam("cnc", args.registry)
    #cnc.homing()
    cnc.moveto(1, 0, 0, 0.5)
