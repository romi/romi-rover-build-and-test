import time
import serial
import argparse
import json
import sys
import traceback
import math

def send_command(link, s):
    print("Command: %s" % s)
    command = "#" + s + ":xxxx\r"
    link.write(command.encode('ascii'))
    return assert_reply(read_reply(link))

def read_reply(link):
    while True:
        s = link.readline().decode("ascii").rstrip()
        if s[0] == "#":
            if s[1] == "!":
                print("Log: %s" % s)
            else:
                print("Reply: %s" % s)
                break;
    return s
        
def assert_reply(line):
    s = str(line)
    start = s.find("[")
    end = 1 + s.find("]")
    array_str = s[start:end]
    return_values = json.loads(array_str)
    status_code = return_values[0]
    success = (status_code == 0)
    if not success:
        raise RuntimeError(return_values[1]) 
    return return_values

def send_configuration(link, config, kp, ki, kd):
    steps = config["navigation"]["rover"]["encoder_steps"]
    wheel_diameter = config["navigation"]["rover"]["wheel_diameter"]
    max_speed = config["navigation"]["rover"]["maximum_speed"]
    max_signal = config["navigation"]["brush-motor-driver"]["maximum_signal_amplitude"]
    use_pid = True
    left_encoder = config["navigation"]["brush-motor-driver"]["encoder_directions"]["left"]
    right_encoder = config["navigation"]["brush-motor-driver"]["encoder_directions"]["right"]
    circumference = math.pi * wheel_diameter
    max_rps = max_speed / circumference;
    send_command(link, "C[{0},{1},{2},{3},{4},{5},{6},{7},{8}]"
                 .format(int(steps), int(100 * max_rps), int(max_signal),
                         1, int(kp * 1000), int(ki * 1000), int(kd * 1000),
                         int(left_encoder), int(right_encoder)))
    
def load_configuration(link, filename, kp, ki, kd):
    with open(filename, "r") as read_file:
        config = json.load(read_file)
        send_configuration(link, config, kp, ki, kd)
        
def send_enable(link):
    send_command(link, "E[1]")

def get_encoders(link):
    send_command(link, "e")
    
def test_moveat(link, left, right):
    send_command(link, "V[%d,%d]" % (left, right))

    
if __name__ == "__main__":
    # execute only if run as a script
    parser = argparse.ArgumentParser()
    parser.add_argument('--device', type=str, nargs='?', default="/dev/ttyACM0",
                    help='Set the serial device')
    parser.add_argument('--config', type=str, nargs='?', default="../config.json",
                        help='Set the configuration file')

    args = parser.parse_args()


    try:
        
#        for p in range(1, 11):
#        
#            print("================================================")
#            print("Opening device %s" % args.device)
#
#            link = serial.Serial(args.device, 115200)
#            time.sleep(2.0)
#
#            print("Kp %f" % (p * 0.1))
#
#            load_configuration(link, args.config, p * 0.1, 0, 0)
#            send_enable(link)
#            test_moveat(link, 500, 500)
#            time.sleep(4.0)
#            test_moveat(link, 0, 0)
#            time.sleep(1.0)
#
#            link.close()

        for i in range(1, 20):
        
            print("================================================")
            print("Opening device %s" % args.device)

            link = serial.Serial(args.device, 115200)
            time.sleep(2.0)

            kp = 0.7
            ki = i * 0.01
            print("Kp %f, Ki %f" % (kp, ki))

            load_configuration(link, args.config, kp, ki, 0)
            send_enable(link)
            test_moveat(link, 300, 300)
            time.sleep(10.0)
            test_moveat(link, 0, 0)
            time.sleep(2.0)

            link.close()

            
    except RuntimeError as e:
        traceback.print_exc()
