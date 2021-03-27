import time
import serial
import json
import math

class RomiDevice():

    def __init__(self, device): 
        print(f"Opening a serial link to {device}")
        self.driver = serial.Serial(device, 115200)
        time.sleep(2.0)

    def send_command(self, s):
        print("Command: %s" % s)
        command = "#" + s + ":xxxx\r"
        self.driver.write(command.encode('ascii'))
        reply = self.read_reply()
        return self.parse_reply(reply)

    def read_reply(self):
        while True:
            b = self.driver.readline()
            s = b.decode("ascii").rstrip()
            if s[0] == "#":
                if s[1] == "!":
                    print("Log: %s" % s)
                else:
                    print("Reply: %s" % s)
                    break;
        return s
        
    def parse_reply(self, line):
        s = str(line)
        start = s.find("[")
        end = 1 + s.find("]")
        array_str = s[start:end]
        return_values = json.loads(array_str)
        status_code = return_values[0]
        success = (status_code == 0)
        if not success:
            raise RuntimeError(return_values[0]) 
        return return_values

###

class MotorController(RomiDevice):

    def __init__(self, device, config): 
        super(MotorController, self).__init__(device)
        self.send_configuration(config)
        self.enable()

    def send_configuration(self, config):
        print("Configuration:")
        print(json.dumps(config, indent=4, sort_keys=True))
        
        steps = config["navigation"]["rover"]["encoder_steps"]
        wheel_diameter = config["navigation"]["rover"]["wheel_diameter"]
        max_speed = config["navigation"]["rover"]["maximum_speed"]
        max_signal = config["navigation"]["brush-motor-driver"]["maximum_signal_amplitude"]
        use_pid = config["navigation"]["brush-motor-driver"]["use_pid"]
        print(f"PID={use_pid}")
        kp = config["navigation"]["brush-motor-driver"]["pid"]["kp"]
        ki = config["navigation"]["brush-motor-driver"]["pid"]["ki"]
        kd = config["navigation"]["brush-motor-driver"]["pid"]["kd"]
        left_encoder = config["navigation"]["brush-motor-driver"]["encoder_directions"]["left"]
        right_encoder = config["navigation"]["brush-motor-driver"]["encoder_directions"]["right"]
        circumference = math.pi * wheel_diameter
        max_rps = max_speed / circumference;
        self.send_command("C[{0},{1},{2},{3},{4},{5},{6},{7},{8}]"
                          .format(int(steps), int(100 * max_rps), int(max_signal),
                                  int(use_pid), int(kp * 1000), int(ki * 1000),
                                  int(kd * 1000), int(left_encoder), int(right_encoder)))
    
    def enable(self):
        self.send_command("E[1]")
    
    def moveat(self, left, right):
        self.send_command("V[%d,%d]" % (left, right))

    def get_encoder_values(self):
        reply = self.send_command("e")
        # The first element is the status code
        reply.pop(0)
        return reply
    
###

def load_configuration_file(filename):
    print("Loading configuration")
    with open(filename, "r") as read_file:
        return json.load(read_file)

def request_confirmation(message):
    response = input(message + " [Y/n] ")
    if (response == "n"):
        raise AssertionError("User responded 'no'")
