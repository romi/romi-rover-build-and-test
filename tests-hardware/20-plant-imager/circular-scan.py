import argparse
import math
import time
import json
from romi.remote_device import OquamXYTheta
from romi.camera import Camera
        
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--registry', type=str, nargs='?', default="127.0.0.1",
                    help='IP address of the registry')
    args = parser.parse_args()
    
    cnc = OquamXYTheta("cnc", args.registry)
    cnc.power_up()
    cnc.homing()

    camera1 = Camera("camera-top", args.registry)
    #camera2 = Camera("camera-bottom", args.registry)
    camera1.select_option('exposure-mode', 'off')
    camera1.set_value('analog-gain', 10)
    camera1.set_value('shutter-speed', 60000)
    #camera1.set_value('jpeg-quality', 95)
    
    dimension = cnc.get_range()
    print(f"dimension={dimension}")

    xc = (dimension[0][0] + dimension[0][1]) / 2.0
    yc = (dimension[1][0] + dimension[1][1]) / 2.0
    d = min(dimension[0][1] - dimension[0][0],
            dimension[1][1] - dimension[1][0])
    r = (d - 0.02) / 2
    print(f"c=({xc}, {yc}), r={r}")

    speed = 0.8
    N = 16
    
    cnc.moveto(xc - r, yc, 0.0, speed)

    metadata = {}
    
    for i in range(N):
        angle = -2.0 * math.pi / N
        cnc.helix(xc, yc, angle, angle, speed)
        image1 = camera1.grab()
        name1 = f"camera1-{i:05d}.jpg"
        image1.save(name1)
        position = cnc.get_position()
        metadata[name1] = {'camera': name1,
                           'camera-index': 0,
                           'index': i,
                           'position': {'x': position['x'],
                                        'y': position['y'],
                                        'pan': position['z']}}

    with open('metadata.json', 'w') as outfile:
        json.dump(metadata, outfile, indent=4)
        
    cnc.moveto(0, 0, 0, speed)
