import smbus, struct

# enum I2cCommands {
I2C_FOLLOW = 0
I2C_ANGLE = 1
I2C_ZERO_OFFSET = 2
I2C_ZERO = 3
I2C_MAX = 4
I2C_MOTOR_SLEEP = 5
I2C_MOTOR_POWER = 6
I2C_MOTOR_POSITION = 7
I2C_KP = 8
I2C_MAX_ACCEL = 9
I2C_RESET = 10
# 	I2C_COMMAND_COUNT
# }; 

# import gimbal
# g = gimbal.Gimbal(11)

# Normal startup rouine should be:
# 1. Manually put camera pointing perpendicular to ground
# 2. g.set_zero()     -> This will set offsets for the motor encoder and the accelerometer
# 2a. g.reset()       -> Make sure motor is not in a fal state
# 3. g.motor_wake()
# 4. g.set_motor_position(0)
# 5. g.set_follow(1)


class Gimbal():

    def __init__(self, addr):
        self.bus = smbus.SMBus(1)
        self.address = addr


    def set_follow(self, value):
        # Turns on(1) and off(0) the holding of angle
        self.i2c_set(I2C_FOLLOW, int(value))

    def get_angle(self):
        d,e = self.i2c_get(I2C_ANGLE)
        if not e: return d / 10000

    def set_angle(self, newAngle):
        # Sets and holds a roll angle based on IMU data. range?
        self.i2c_set(I2C_ANGLE, int(newAngle * 10000))

    def get_zero_offset(self):
        d,e = self.i2c_get(I2C_ZERO_OFFSET)
        if not e: return d / 10000

    def set_zero_offset(self, newOffset):
        # Sets the zero offset for the roll calculation
        self.i2c_set(I2C_ZERO_OFFSET, int(newOffset * 10000));

    def set_zero(self):
        # Sets the current position as zero
        # The idea is send this command when you know that the camera is pointing perpendicular to ground,
        self.i2c_set(I2C_ZERO, 0);

    def get_max(self):
        d,e = self.i2c_get(I2C_MAX)
        if not e: return d / 10000

    def set_max(self, newMax):
        self.i2c_set(I2C_MAX, int(newMax * 10000))

    def motor_sleep(self):
        self.i2c_set(I2C_MOTOR_SLEEP, 1)

    def motor_wake(self):
        self.i2c_set(I2C_MOTOR_SLEEP, 0)
        self.set_motor_position(0) # Without this, motor doesn't start properly.

    def get_motor_power(self):
        d,e = self.i2c_get(I2C_MOTOR_POWER)
        if not e: return d

    def set_motor_power(self, newPower):
        # Power should be 0-100 value
        self.i2c_set(I2C_MOTOR_POWER, int(newPower))

    def get_motor_position(self):
        d,e = self.i2c_get(I2C_MOTOR_POSITION)
        if not e: return d / 10000

    def set_motor_position(self, newPos):
        self.i2c_set(I2C_MOTOR_POSITION, int(newPos * 10000))

    def get_kp(self):
        d,e = self.i2c_get(I2C_KP)
        if not e: return d / 10000

    def set_kp(self, newKp):
        self.i2c_set(I2C_KP, int(newKp * 10000))

    def get_max_accel(self):
        d,e = self.i2c_get(I2C_MAX_ACCEL)
        if not e: return d / 10000

    def set_max_accel(self, maxAccel):
        self.i2c_set(I2C_MAX_ACCEL, int(maxAccel * 10000))

    def reset(self):
        self.i2c_set(I2C_RESET, 1);

    # Utility functions

    def from_bytes(self, data):
        return struct.unpack('i', bytearray(data))[0]

    def to_bytes(self, data):
        return list(bytearray(struct.pack('i', data)))

    def i2c_set(self, command, data):
        listData = self.to_bytes(data)
        check = self.to_bytes(sum(listData))[0]
        listData.append(check)
        self.bus.write_i2c_block_data(self.address, command, listData)

    def i2c_get(self, command):
        data = self.bus.read_i2c_block_data(self.address, command, 5)
        check = self.to_bytes(sum(data[0:4]))[0]
        if check != data[4]:
            error = True
        else:
            error = False
        return self.from_bytes(data[0:4]),error
