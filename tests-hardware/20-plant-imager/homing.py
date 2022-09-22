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

    
