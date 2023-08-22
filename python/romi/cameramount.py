import argparse
from rcom.rcom_client import RcomClient

class CameraMount(RcomClient):
   
    def __init__(self, name = 'camera-mount', id = 'mount', registry = '127.0.0.1'):
        super().__init__(name, id, registry)
       
    def homing(self):
        self.execute('camera-mount:homing')
       
    def power_up(self):
        self.execute('power-up')
        
    def power_down(self):
        self.execute('power-down')
       
    def get_range(self):
        return self.execute('camera-mount:get-range')
       
    def moveto(self, x, y, z, ax, ay, az, speed):
        params = {
            "x": x, "y": y, "z": z,
            "ax": ax, "ay": ay, "az": az,
            "speed": speed
        }
        print(params)
        cmd = { "method": "cnc-moveto", "params": params }
        self.execute('camera-mount:moveto', params)
       
    def get_position(self):
        return self.execute('camera-mount:get-position')

    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--registry', type=str, nargs='?', default="127.0.0.1",
                    help='IP address of the registry')
    parser.add_argument('--topic', type=str, nargs='?', default="camera-mount",
                    help='The topic of the camera mount (def.: camera-mount)')
    parser.add_argument('--id', type=str, nargs='?', default="mount",
                    help='The id of the camera mount (def.: mount)')
    args = parser.parse_args()

    mount = CameraMount(args.topic, args.id, args.registry)
    #cnc.homing()
    mount.moveto(1, 0, 0, 0.5)
