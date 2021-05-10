import time
import argparse
import traceback
import matplotlib.pyplot as plt
import sys
import math

sys.path.append('..')

from testlib import *
from motor_controller import MotorController



def run_pid_test(config, controller, speed, duration, kp, ki, kd):
    print("==============================")
    print(f"Kp {kp}, Ki {ki}, Kd {kd}")
    
    config["brush-motor-driver"]["use_pid"] = True
    config["brush-motor-driver"]["pid"]["kp"] = kp
    config["brush-motor-driver"]["pid"]["ki"] = ki
    config["brush-motor-driver"]["pid"]["kd"] = kd

    controller.set_configuration(config)

    start_time = time.time()
    encoder_values = []
    debug_value = controller.get_debug()
    controller.set_debug(False)
    
    encoders = controller.send_command("e")
    encoder_values.append(encoders)
    
    controller.moveat(speed, speed)

    while time.time() - start_time < duration:
        try:
            encoders = controller.send_command("e")
            encoder_values.append(encoders)
        except:
            print("controller.send_command('e') raised an error")
        
    print(controller.get_status())
    controller.moveat(0, 0)
    time.sleep(2)
    controller.set_debug(debug_value)
    return encoder_values

def convert_values(values, max_rps, encoder_steps):
    x = []
    left = []
    right = []
    window_left = [0, 0, 0]
    window_right = [0, 0, 0]
    measurement = values[0]
    last_left_value = measurement[1]
    last_right_value = measurement[2]
    last_time = measurement[3]
    start_time = measurement[3]
    
    for i in range(1, len(values)):
        measurement = values[i]
        left_value = measurement[1]
        right_value = measurement[2]
        time = measurement[3]
        dt = (time - last_time) / 1000.0
        speed_left = 1000.0 * (left_value - last_left_value) / dt / encoder_steps / max_rps
        speed_right = 1000.0 * (right_value - last_right_value) / dt / encoder_steps / max_rps

        window_left[i % 3] = speed_left
        window_right[i % 3] = speed_right

        avg_left = sum(window_left) / 3.0
        avg_right = sum(window_right) / 3.0
        
        x.append((time - start_time) / 1000.0)
        left.append(avg_left)
        right.append(avg_right)
        last_left_value = left_value
        last_right_value = right_value
        last_time = time
    return x, left, right
        
def show_graph(values, title, max_rps, encoder_steps):
    x, y1, y2 = convert_values(values, max_rps, encoder_steps)
    plt.clf()
    plt.plot(x, y1)
    plt.xlabel('time')
    plt.ylabel('angular speed')
    plt.title(title)
    plt.show()
        
def store_graph(values, title, max_rps, encoder_steps):
    x, y1, y2 = convert_values(values, max_rps, encoder_steps)
    plt.clf()
    plt.plot(x, y1)
    plt.xlabel('time')
    plt.ylabel('angular speed')
    plt.title(title)
    plt.savefig(f"{title}.svg")

def store_measurements(values, title):
    filename = f"{title}.json"
    with open(filename, 'w') as outfile:
        json.dump(values, outfile, indent=4)
        
def load_measurements(title):
    filename = f"{title}.json"
    with open(filename, 'r') as infile:
        return json.load(infile)

def test_and_store(config, controller, speed, duration, kp, ki, kd):
    encoder_steps = controller.get_encoder_steps()
    max_rps = controller.get_max_rps()
    values = run_pid_test(config, controller, speed, duration, kp, ki, kd)
    title = "data-%04d-%04d-%04d-%04d" % (speed, int(1000*kp), int(1000*ki), int(1000*kd))
    store_measurements(values, title)
    show_graph(values, title, max_rps, encoder_steps)
    store_graph(values, title, max_rps, encoder_steps)

def test_range_kp(config, controller, speed, start_value, end_value, step):
    kp = start_value
    ki = 0
    kd = 0
    while kp < end_value:
        test_and_store(config, controller, speed, 2, kp, ki, kd)
        test_and_store(config, controller, -speed, 2, kp, ki, kd)
        kp += step
        
def test_range_ki(config, controller, speed, kp, start_value, end_value, step):
    ki = start_value
    kd = 0
    while ki < end_value:
        test_and_store(config, controller, speed, 2, kp, ki, kd)
        test_and_store(config, controller, -speed, 2, kp, ki, kd)
        ki += step
        
def test_range_kd(config, controller, speed, kp, ki, start_value, end_value, step):
    kd = start_value
    while kd < end_value:
        test_and_store(config, controller, speed, 2, kp, ki, kd)
        test_and_store(config, controller, -speed, 2, kp, ki, kd)
        kd += step
        
def run_tuning_tests(device, navigation_config):
    try:
        controller = MotorController(device, navigation_config)

        speed = 500
        max_rps = controller.get_max_rps()
        encoder_steps = controller.get_encoder_steps()
        encoder_steps_per_s = max_rps * encoder_steps
        target_angular_speed = encoder_steps_per_s * speed / 1000.0

        print("Target: %f" % target_angular_speed)
        print("encoder_steps_per_s: %f" % encoder_steps_per_s)

        request_confirmation("Activate the security button (turn and/or pull out).")
        
        #test_range_kp(navigation_config, controller, speed, 1.8, 3.0, 0.1)
        
        #kp = 0.7
        #test_range_ki(navigation_config, controller, speed, kp, 0.0, 1.0, 0.05)

        kp = 0.6
        ki = 0.3
        test_range_kd(navigation_config, controller, speed, kp, ki, 0.0, 1.0, 0.05)

        #kp = 0.6
        #ki = 0.3
        #kd = 0.1
        
    except RuntimeError as e:
        traceback.print_exc()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--device', type=str, nargs='?', default="/dev/ttyACM0",
                    help='Set the serial device')
    parser.add_argument('--config', type=str, nargs='?', default="../config.json",
                        help='Set the configuration file')

    args = parser.parse_args()
    
    try:
        config = load_configuration_file(args.config)
        navigation_config = config["navigation"]
    
        run_tuning_tests(args.device, navigation_config)

        #title = 'data-0600-0400-0000-0000'
        #values = load_measurements(title)
        #show_graph(values, title)

    except RuntimeError as e:
        traceback.print_exc()
    
# kp=0.30
# ki=
