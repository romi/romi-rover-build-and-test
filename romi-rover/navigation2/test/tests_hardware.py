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

def send_configuration(link, config):
    steps = config["navigation"]["rover"]["encoder_steps"]
    wheel_diameter = config["navigation"]["rover"]["wheel_diameter"]
    max_speed = config["navigation"]["rover"]["maximum_speed"]
    max_signal = config["navigation"]["brush-motor-driver"]["maximum_signal_amplitude"]
    use_pid = config["navigation"]["brush-motor-driver"]["use_pid"]
    kp = config["navigation"]["brush-motor-driver"]["pid"]["kp"]
    ki = config["navigation"]["brush-motor-driver"]["pid"]["ki"]
    kd = config["navigation"]["brush-motor-driver"]["pid"]["kd"]
    left_encoder = config["navigation"]["brush-motor-driver"]["encoder_directions"]["left"]
    right_encoder = config["navigation"]["brush-motor-driver"]["encoder_directions"]["right"]
    circumference = math.pi * wheel_diameter
    max_rps = max_speed / circumference;
    send_command(link, "C[{0},{1},{2},{3},{4},{5},{6},{7},{8}]"
                 .format(int(steps), int(100 * max_rps), int(max_signal),
                         int(use_pid), int(kp * 1000), int(ki * 1000), int(kd * 1000),
                         int(left_encoder), int(right_encoder)))
    
def load_configuration(link, filename):
    with open(filename, "r") as read_file:
        config = json.load(read_file)
        send_configuration(link, config)
        
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
    print("Opening device %s" % args.device)

    link = serial.Serial(args.device, 115200)
    time.sleep(2.0)

    load_configuration(link, args.config)
    send_enable(link)
                 
    try:
        
        response = input("Ready to start the tests? [Y/n]")
        if (response == "n"):
            sys.exit("Quitting")
                 
        print("Testing the directions of motors.")
        
        print("Testing left motor (forward)")
        print("Current position")
        get_encoders(link)
        test_moveat(link, 300, 0)
        time.sleep(2.0)
        test_moveat(link, 0, 0)
        print("New position")
        get_encoders(link)
                 
        response = input("Did the left motor roll forward? [Y/n]")
        if (response == "n"):
            sys.exit("Bad motor direction on the left wheel")
                 
        response = input("Did the left encoder value increase? [Y/n]")
        if (response == "n"):
            sys.exit("Bad encoder direction on left wheel")


        print("Testing left motor (backward)")
        print("Current position")
        get_encoders(link)
        test_moveat(link, -300, 0)
        time.sleep(2.0)
        test_moveat(link, 0, 0)
        print("New position")
        get_encoders(link)
                 
        response = input("Did the left motor roll backward? [Y/n]")
        if (response == "n"):
            sys.exit("Bad motor direction on the left wheel")
                 
        response = input("Did the left encoder value decrease? [Y/n]")
        if (response == "n"):
            sys.exit("Bad encoder direction on left wheel")



        print("Testing right motor (forward)")
        print("Current position")
        get_encoders(link)
        test_moveat(link, 0, 300)
        time.sleep(2.0)
        test_moveat(link, 0, 0)
        print("New position")
        get_encoders(link)
                 
        response = input("Did the right motor roll forward? [Y/n]")
        if (response == "n"):
            sys.exit("Bad motor direction on the right wheel")
                 
        response = input("Did the right encoder value increase? [Y/n]")
        if (response == "n"):
            sys.exit("Bad encoder direction on right wheel")


        print("Testing right motor (backward)")
        print("Current position")
        get_encoders(link)
        test_moveat(link, 0, -300)
        time.sleep(2.0)
        test_moveat(link, 0, 0)
        print("New position")
        get_encoders(link)
                 
        response = input("Did the right motor roll backward? [Y/n]")
        if (response == "n"):
            sys.exit("Bad motor direction on the right wheel")
                 
        response = input("Did the right encoder value decrease? [Y/n]")
        if (response == "n"):
            sys.exit("Bad encoder direction on right wheel")


            
        print("Testing speeds (forward)")
        for i in range(1, 11):
            print("Speed %f" % (i*0.1))
            test_moveat(link, i*100, i*100)
            time.sleep(2.0)
            test_moveat(link, 0, 0)
            
        print("Testing speeds (backward)")
        for i in range(1, 11):
            print("Speed %f" % (-i*0.1))
            test_moveat(link, -i*100, -i*100)
            time.sleep(2.0)
            test_moveat(link, 0, 0)

        print("All tests OK")    
            
    except RuntimeError as e:
        traceback.print_exc()
