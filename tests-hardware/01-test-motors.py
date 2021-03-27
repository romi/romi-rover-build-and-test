import time
import serial
import argparse
import json
import sys
import traceback
import math

from testlib import *


def ask_prerequisites():
    request_confirmation("Ready to start the tests?")
    request_confirmation("Is the security button still off (pushed in)?")
    request_confirmation("Are the 0V, S1, and S2 wires connected from the Arduino to the Sabertooth?")
    request_confirmation("Is the DIP switch of the Sabertooth configured (1=off, 2=on, 3=off, 4=off, 5=on, 6=off)?")
    request_confirmation("Is the power connected to the Sabertooth over the security button, the power relay, and the 40A circuit breaker?")
    request_confirmation("Is the circuit breaker engaged (=current passes; flag set to red)?")
    request_confirmation("Are the motor cables plugged in?")
    request_confirmation("Are the wheel handles engaged?")
    
def test_direction(controller, left, right, message):
    controller.moveat(left, right)
    time.sleep(2.0)
    controller.moveat(0, 0)                 
    request_confirmation(message)

def test_directions(controller):
    print("Testing the directions of motors.")        
    test_direction(controller, 300, 0, "Did the left motor roll forward?")
    test_direction(controller, -300, 0, "Did the left motor roll backward?")
    test_direction(controller, 0, 300, "Did the right motor roll forward?")
    test_direction(controller, 0, -300, "Did the right motor roll backward?")

def test_speeds(controller):
    print("Testing motors at different speeds.")
    ramp_up = [i * 0.1 for i in range(1, 11)]
    ramp_down = [1.0 - i * 0.1 for i in range(1, 11)]
    speed_profile = ramp_up + ramp_down
    print(speed_profile)
    change_speeds(controller, speed_profile)
    change_speeds(controller, [-speed for speed in speed_profile])

    
def change_speeds(controller, speeds):
    for speed in speeds:
        print("Speed %f" % speed)
        controller.moveat(speed*1000, speed*1000)
        time.sleep(2.0)
        
    
if __name__ == "__main__":
    # execute only if run as a script
    parser = argparse.ArgumentParser()
    parser.add_argument('--device', type=str, nargs='?', default="/dev/ttyACM0",
                    help='Set the serial device')
    parser.add_argument('--config', type=str, nargs='?', default="../config.json",
                        help='Set the configuration file')

    args = parser.parse_args()
                 
    try:
        ask_prerequisites()
        config = load_configuration_file(args.config)
        controller = MotorController(args.device, config)
        request_confirmation("Activate the security button (turn and/or pull out).")
        test_directions(controller)
        request_confirmation("Run the speed tests?")
        test_speeds(controller)
            
    except RuntimeError as e:
        traceback.print_exc()
