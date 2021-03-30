import serial
import json
import math

from romi_device import RomiDevice

class MotorController(RomiDevice):

    def __init__(self, device, config): 
        super(MotorController, self).__init__(device)
        self.send_configuration(config)
        self.enable()
        self.config = config
        
    def send_configuration(self, config):
        steps = config["rover"]["encoder_steps"]
        wheel_diameter = config["rover"]["wheel_diameter"]
        max_speed = config["rover"]["maximum_speed"]
        max_signal = config["brush-motor-driver"]["maximum_signal_amplitude"]
        use_pid = config["brush-motor-driver"]["use_pid"]
        print(f"PID={use_pid}")
        kp = config["brush-motor-driver"]["pid"]["kp"]
        ki = config["brush-motor-driver"]["pid"]["ki"]
        kd = config["brush-motor-driver"]["pid"]["kd"]
        left_encoder = config["brush-motor-driver"]["encoder_directions"]["left"]
        right_encoder = config["brush-motor-driver"]["encoder_directions"]["right"]
        circumference = math.pi * wheel_diameter
        max_rps = max_speed / circumference;
        self.send_command("C[{0},{1},{2},{3},{4},{5},{6},{7},{8}]"
                          .format(int(steps), int(100 * max_rps), int(max_signal),
                                  int(use_pid), int(kp * 1000), int(ki * 1000),
                                  int(kd * 1000), int(left_encoder), int(right_encoder)))
    
    def get_config(self):
        return self.config
    
    def enable(self):
        self.send_command("E[1]")
    
    def disable(self):
        self.send_command("E[0]")
    
    def moveat(self, left, right):
        self.send_command("V[%d,%d]" % (left, right))

    def get_encoder_values(self):
        reply = self.send_command("e")
        # The first element is the status code
        reply.pop(0)
        return reply
