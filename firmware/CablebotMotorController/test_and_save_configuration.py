
import odrive
import odrive.enums as od
import time

#odrv = odrive.find_any(serial_number="316133753232")
odrv = odrive.find_any()
axis = odrv.axis0

def pause(t):
    time.sleep(t)

def wait(axis):
    while axis.current_state != od.AXIS_STATE_IDLE:
        time.sleep(0.1)


#odrv.config.enable_brake_resistor = False
axis.motor.config.motor_type = 0
axis.motor.config.pole_pairs = 7     
axis.motor.config.torque_constant = 8.27/280  # Motor K_v = 280 rpm/V
axis.motor.config.current_lim = 20
axis.motor.config.calibration_current = 20
axis.controller.config.vel_limit = 20
axis.controller.config.vel_ramp_rate = 5

# Counts per revolution. cf. AMT102-V datahsheet, DIP Switch
axis.encoder.config.cpr = 8192  
axis.encoder.config.use_index = True
axis.encoder.config.mode = od.ENCODER_MODE_INCREMENTAL

#
#axis.motor.config.pre_calibrated = True
#axis.encoder.config.pre_calibrated = True

#axis.config.startup_motor_calibration = True
#axis.config.startup_encoder_index_search = True
#axis.config.startup_encoder_offset_calibration = True
#axis.config.startup_closed_loop_control = True
#axis.config.startup_encoder_offset_calibration = True
odrv.save_configuration()

#pause(2)

print("Step 1: Calibration")
axis.requested_state = od.AXIS_STATE_FULL_CALIBRATION_SEQUENCE
wait(axis)
#print(axis.error)

print("Step 2: Index Search")
# We're using the encoder's Z output to calibrate the latter through stored index
axis.requested_state = od.AXIS_STATE_ENCODER_INDEX_SEARCH 
wait(axis)
print(axis.error)


print("Step 3: Offset Calibration")
axis.requested_state = od.AXIS_STATE_ENCODER_OFFSET_CALIBRATION
wait(axis)
print(axis.error)


#axis.requested_state = od.AXIS_STATE_CLOSED_LOOP_CONTROL
#axis.controller.config.control_mode = od.CONTROL_MODE_VELOCITY_CONTROL
#
#while True:
#    try:
#        speed = float(input("Entrer la vitesse en tour/s (0 pour quitter): "))
#        if speed == 0:
#            break
#        axis.controller.input_vel = speed
#    except ValueError:
#        print("Entrée non valide, veuillez entrer un nombre.")
#
#axis.controller.input_vel = 0                                             #
#axis.controller.input_pos = axis.encoder.pos_estimate

axis.requested_state = od.AXIS_STATE_IDLE
#axis.requested_state = od.AXIS_STATE_CLOSED_LOOP_CONTROL

#axis.motor.config.pre_calibrated = True
#axis.encoder.config.pre_calibrated = True

axis.config.startup_motor_calibration = True
axis.config.startup_encoder_index_search = True
axis.config.startup_encoder_offset_calibration = True
axis.config.startup_closed_loop_control = True

print("Saving")
odrv.save_configuration()
odrv.reboot()

