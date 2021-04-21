import time
import serial
import argparse
import json
import sys
import traceback

from testlib import *
from oquam_controller import OquamController
    
def test_homing(cnc):
    cnc.homing()
    
def configure_homing(cnc, axes):
    cnc.configure_homing(axes)
    
def test_move_x(cnc, millis, steps):
    cnc.move(millis, steps, 0, 0)

def test_move_y(cnc, millis, steps):
    cnc.move(millis, 0, steps, 0)

def test_move_z(cnc, millis, steps):
    cnc.move(millis, 0, 0, steps)
    
def test_move_xy(cnc, millis, dx, dy):
    cnc.move(millis, dx, dy, 0)
    
def test_move_and_back_x(cnc, millis, steps):
    test_move_x(cnc, millis, steps)
    test_move_x(cnc, millis, -steps)
    
def test_move_and_back_y(cnc, millis, steps):
    test_move_y(cnc, millis, steps)
    test_move_y(cnc, millis, -steps)
    
def wait(cnc):
    cnc.wait()

def ask_prerequisites():
    request_confirmation("Ready to start the tests?")
    request_confirmation("Check the locations of the limit switches")
    request_confirmation("Is the security button still tuned on (pushed in)?")
        
if __name__ == "__main__":
    # execute only if run as a script

    parser = argparse.ArgumentParser()
    parser.add_argument('--device', type=str, nargs='?', default="/dev/ttyACM0",
                    help='Set the serial device')

    args = parser.parse_args()

    try:
        cnc = OquamController(args.device)

        ask_prerequisites()

        cnc.enable()

        print("Testing the directions of motors.")
        print("Testing X")
        
        test_move_x(cnc, 1000, 1000)
        request_confirmation("Did the CNC move away from the limit switches on the X-axis?")
            
        print("Testing Y")
        test_move_y(cnc, 1000, 1000)
        request_confirmation("Did the CNC move away from the limit switches on the Y-axis?")
            
        print("Testing Z")
        test_move_z(cnc, 1000, 1000)
        request_confirmation("Did the CNC move away from the limit switches on the Z-axis?")
                
        if (ask("Test homing?")):
            axes = [-1,-1,-1]
            if (ask("Homing on X?")):
                axes[0] = 0
            if (ask("Homing on Y?")):
                axes[1] = 1
            if (ask("Homing on Z?")):
                axes[2] = 2
            configure_homing(cnc, axes)
            
            print("Testing homing")
            test_homing(cnc)
            request_confirmation("Did the CNC home correctly?")

        print("Doing speed tests")    
        test_move_xy(cnc, 500, 500, 500)
    
        print("Testing X")    
        test_move_and_back_x(cnc, 1000, 1000)    
        test_move_and_back_x(cnc, 1000, 2000)
        test_move_and_back_x(cnc, 1000, 3000)
        test_move_and_back_x(cnc, 1000, 4000)
        cnc.wait()
        request_confirmation("Did the CNC move ok?")

        print("Testing Y")    
        test_move_and_back_y(cnc, 1000, 1000)
        test_move_and_back_y(cnc, 1000, 2000)
        test_move_and_back_y(cnc, 1000, 3000)
        test_move_and_back_y(cnc, 1000, 4000)
        cnc.wait()
        request_confirmation("Did the CNC move ok?")

        if (ask("Test motor relay?")):
            cnc.set_relay(True)
            time.sleep(2.0)
            cnc.set_relay(False)
            request_confirmation("Did the motor work?")
        
        print("All tests OK")    

    except RuntimeError as re:
        traceback.print_exc()

    cnc.disable()
