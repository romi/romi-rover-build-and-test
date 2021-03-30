import time
import argparse
import json
import traceback

from testlib import *
from motor_controller import MotorController

def ask_prerequisites():
    request_confirmation("Did you run the motor tests?")
    request_confirmation("Are the encoders wired?")
    request_confirmation("Are the encoders values in the config file correct (encoder steps/rotation x gears ratio?")

def test_encoder(controller, index):
    encoders_before = controller.get_encoder_values()
    if index == 0:
        left = 200
        right = 0
    elif index == 1:
        left = 0
        right = 200
    controller.moveat(left, right)
    time.sleep(1)
    controller.moveat(0, 0)
    encoders_after = controller.get_encoder_values()
    print("Encoder: before: {0}, after {1}".format(encoders_before[index],
                                                        encoders_after[index]))
    assert(encoders_before[index] + 100 < encoders_after[index])

    
def test_left_encoder(controller):
    test_encoder(controller, 0)

def test_right_encoder(controller):
    test_encoder(controller, 1)

    
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
        navigation_config = config["navigation"]
        controller = MotorController(args.device, navigation_config)
        request_confirmation("Activate the security button (turn and/or pull out).")
        test_left_encoder(controller)
        test_right_encoder(controller)
            
    except RuntimeError as e:
        traceback.print_exc()
