import time
import argparse
import traceback

from testlib import *
from motor_controller import MotorController

def test_direct_control(config, controller):
    config["brush-motor-driver"]["use_pid"] = False
    config["brush-motor-driver"]["pid"]["kp"] = 0.0
    config["brush-motor-driver"]["pid"]["ki"] = 0.0
    config["brush-motor-driver"]["pid"]["kd"] = 0.0

    controller.disable()
    controller.send_configuration(config)
    controller.enable()
    
    controller.moveat(150, 150)
    time.sleep(2.0)
    controller.moveat(0, 0)
    time.sleep(0.2)
    controller.moveat(-150, -150)
    time.sleep(2.0)
    controller.moveat(0, 0)

    
def test_pid(config, controller, speed, kp, ki, kd):
    print("==============================")
    print(f"Kp {kp}, Ki {ki}, Kd {kd}")
    
    config["brush-motor-driver"]["use_pid"] = True
    config["brush-motor-driver"]["pid"]["kp"] = kp
    config["brush-motor-driver"]["pid"]["ki"] = ki
    config["brush-motor-driver"]["pid"]["kd"] = kd

    controller.disable()
    controller.send_configuration(config)
    controller.enable()
    
    controller.moveat(speed, speed)
    time.sleep(5.0)
    controller.moveat(0, 0)
    time.sleep(0.2)
    controller.moveat(-speed, -speed)
    time.sleep(5.0)
    controller.moveat(0, 0)

    
def test_range_proportional(config, controller, speed, start_value, end_value, step):
    retval = None
    kp = start_value
    while kp < end_value:
        test_pid(config, controller, speed, kp, 0, 0)
        if ask("Accept current value?"):
            retval = kp
            break
        kp += step
    return retval
        
def test_range_integration(config, controller, speed, kp, start_value, end_value, step):
    retval = None
    ki = start_value
    while ki < end_value:
        test_pid(config, controller, speed, kp, ki, 0)
        if ask("Accept current value?"):
            retval = ki
            break
        ki += step
    return retval
        
if __name__ == "__main__":
    # execute only if run as a script
    parser = argparse.ArgumentParser()
    parser.add_argument('--device', type=str, nargs='?', default="/dev/ttyACM0",
                    help='Set the serial device')
    parser.add_argument('--config', type=str, nargs='?', default="../config.json",
                        help='Set the configuration file')

    args = parser.parse_args()

    speed = 600

    try:
        config = load_configuration_file(args.config)
        navigation_config = config["navigation"]
        controller = MotorController(args.device, navigation_config)
        request_confirmation("Activate the security button (turn and/or pull out).")
        test_direct_control(navigation_config, controller)
        request_confirmation("Did the wheels turn?")
        kp = test_range_proportional(navigation_config, controller, speed, 0.28, 0.36, 0.02)
        ki = test_range_integration(navigation_config, controller, speed, kp, 0.02, 0.2, 0.01)
            
    except RuntimeError as e:
        traceback.print_exc()

# kp=0.30
# ki=
