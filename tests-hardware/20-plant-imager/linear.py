import argparse
import math
from romi.remote_device import OquamXYTheta
        
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--registry', type=str, nargs='?', default="127.0.0.1",
                    help='IP address of the registry')
    args = parser.parse_args()
    
    cnc = OquamXYTheta("cnc", args.registry)
    cnc.power_up()

    dimension = cnc.get_range()
    print(f"dimension={dimension}")

    xc = (dimension[0][0] + dimension[0][1]) / 2.0
    yc = (dimension[1][0] + dimension[1][1]) / 2.0
    d = min(dimension[0][1] - dimension[0][0],
            dimension[1][1] - dimension[1][0])
    r = (d - 0.02) / 2
    print(f"c=({xc}, {yc}), r={r}")
    
    #cnc.moveto(0.1, 0, 0, 0.5)
    #cnc.moveto(0, 0.1, 0, 0.5)
    cnc.moveto(0.2, 0.2, 0, 0.1)
    #cnc.moveto(0.2, 0.2, 0, 0.5)
    #cnc.moveto(0.2, 0.2, math.pi, 0.5)
    #cnc.moveto(0.2, 0.2, math.pi, 0.5)
    #cnc.moveto(0.05, 0.05, 0, 1)
    
    print("*** position=", cnc.get_position())
    
    cnc.moveto(0.7, 0.7, 0, 0.1)
    
    print("*** position=", cnc.get_position())

    cnc.moveto(0.0, 0.0, 0, 0.1)

    print("*** position=", cnc.get_position())
    
    #cnc.moveto(0, 0.7, 0, 1)
    #cnc.moveto(0.7, 0, 0, 1)
    #cnc.moveto(0.05, 0.05, 0, 0.1)
    
    #cnc.moveto(0.7, 0.7, 0.0, 0.5)
    
    #position = cnc.get_position()
    #print(f"position={position}")
    #
    #cnc.moveto(0.0, 0.0, 0.0, 0.5)
    #
    #position = cnc.get_position()
    #print(f"position={position}")
