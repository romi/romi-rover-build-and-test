import time
import serial
import argparse
import json
import sys
import traceback

def send_command(link, s):
    #print("Command: %s" % s)
    link.write(s.encode('ascii'))
    return assert_reply(read_reply(link))
    
def read_reply(link):
    while True:
        s = link.readline().decode("ascii").rstrip()
        if s[0] == "#":
            break;
        if s[0] == "!":
            print("Log: %s" % s)
    return s

def assert_reply(line):
    #print("Reply: %s" % line)
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
    
def test_homing(link):
    send_command(link, "#H\r")
    
def test_move_x(link, millis, steps):
    send_command(link, "#M[%d,%d,0,0,0]\r" % (millis, steps))

def test_move_y(link, millis, steps):
    send_command(link, "#M[%d,0,%d,0,0]\r" % (millis, steps))
    
def test_move_xy(link, millis, dx, dy):
    send_command(link, "#M[%d,%d,%d,0,0]\r" % (millis, dx, dy))
    
def test_move_and_back_x(link, millis, steps):
    test_move_x(link, millis, steps)
    test_move_x(link, millis, -steps)
    
def test_move_and_back_y(link, millis, steps):
    test_move_y(link, millis, steps)
    test_move_y(link, millis, -steps)
    
def wait(link):
    while True:
        status = send_command(link, "#S\r")
        if status[2] == 0 and status[3] == -1: # voodoo
            break
        time.sleep(1);
    
if __name__ == "__main__":
    # execute only if run as a script
    parser = argparse.ArgumentParser()
    parser.add_argument('--device', type=str, nargs='?', default="/dev/ttyACM0",
                    help='Set the serial device')

    args = parser.parse_args()
    print("Opening device %s" % args.device)

    link = serial.Serial(args.device, 115200)
    time.sleep(2.0)

    try:
        
        print("Testing the directions of motors.")
        print("Testing X")
        test_move_x(link, 1000, 1000)
        response = input("Did the CNC move away from the limit switches? [Y/n]")
        if (response == "n"):
            sys.exit("Bad motor direction on x-axis")
            
        print("Testing Y")
        test_move_y(link, 1000, 1000)
        response = input("Did the CNC move away from the limit switches? [Y/n]")
        if (response == "n"):
            sys.exit("Bad motor direction on y-axis")
                
        print("Testing homing")
        test_homing(link)
        response = input("Did the CNC home correctly? [Y/n]")
        if (response == "n"):
            sys.exit("Error with homing")

        print("Doing speed tests")    
        test_move_xy(link, 500, 500, 500)
    
        print("Testing X")    
        test_move_and_back_x(link, 1000, 1000)    
        test_move_and_back_x(link, 1000, 2000)
        test_move_and_back_x(link, 1000, 3000)
        test_move_and_back_x(link, 1000, 4000)
        wait(link)
        response = input("Did the CNC move ok? [Y/n]")
        if (response == "n"):
            sys.exit("Error with speed tests on x-axis")

        print("Testing Y")    
        test_move_and_back_y(link, 1000, 1000)
        test_move_and_back_y(link, 1000, 2000)
        test_move_and_back_y(link, 1000, 3000)
        test_move_and_back_y(link, 1000, 4000)
        wait(link)
        response = input("Did the CNC move ok? [Y/n]")
        if (response == "n"):
            sys.exit("Error with speed tests on y-axis")

        print("All tests OK")    
            
    except RuntimeError as e:
        traceback.print_exc()
