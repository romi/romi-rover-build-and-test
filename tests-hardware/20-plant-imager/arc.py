import argparse
import math
import time
from romi.remote_device import OquamXYTheta
        
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--registry', type=str, nargs='?', default="127.0.0.1",
                    help='IP address of the registry')
    args = parser.parse_args()
    
    cnc = OquamXYTheta("cnc", args.registry)
    cnc.power_up()
    cnc.homing()

    dimension = cnc.get_range()
    print(f"dimension={dimension}")

    xc = 0
    yc = 0.30
    r = 0.30
    print(f"c=({xc}, {yc}), r={r}")

    speed = 0.4
    
    cnc.moveto(0.0, 0.0, 0.0, speed)
    
    #cnc.helix(xc, yc, -2.0 * math.pi, -2.0 * math.pi, speed)
    cnc.helix(xc, yc, 0.5 * math.pi, 0.0, speed)

    position = cnc.get_position()
    print(f"position={position}")
    
    #n = 32
    #delta_alpha = -2.0 * math.pi / n
    #start_time = time.time()
    #for i in range(n):
    #cnc.helix(xc, yc, delta_alpha, delta_alpha, speed)
    #
    #print(f"duration: {time.time()-start_time}")
