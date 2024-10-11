import argparse
import time
from romi.cnc import CNC
from romi.camera import Camera


def startup_cnc(cnc):
    # Turn off the battery charger
    cnc.set_relay(0, True)
    # Power-up the motor
    cnc.power_up()

    
def scan(cnc, camera, x0, dx, count):
    for i in range(count + 1):
        x = x0 + i * dx
        cnc.moveto(x, 0, 0, 0.75)
        time.sleep(1)
        image = camera.grab()
        if image != None:
            filename = f"cablebot-{i:05d}-{int(1000*x)}.jpg"
            print(f"Saving {filename}")
            image.save(filename)

            
def shutdown_cnc(cnc, no_homing):
    if no_homing:
        cnc.moveto(0.0, 0, 0, 0.75)
    else:
        # Return to 4 cm before the homing
        cnc.moveto(0.04, 0, 0, 0.75)
        cnc.homing()
        
    cnc.power_down()
    # Recharge the battery
    cnc.set_relay(0, False)
    

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--start', type=float, nargs='?', default=0.0,
                    help='The start position')
    parser.add_argument('--interval', type=float, nargs='?', default=0.5,
                    help='The distance between camera positions')
    parser.add_argument('--count', type=int, nargs='?', default="1",
                    help='The number of images')
    parser.add_argument('--no-homing', action=argparse.BooleanOptionalAction,
                        help='Go back to zero without the homing procedure')
    
    cnc = CNC("cnc", "cnc")
    camera = Camera("camera", "camera")

    startup_cnc(cnc)
    scan(cnc, camera, args.start, args.interval, args.count)
    shutdown_cnc(cnc, args.no_homing)
    # shutdown_cnc(cnc, True)

    
