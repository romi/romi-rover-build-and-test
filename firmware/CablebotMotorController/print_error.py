
import odrive
import odrive.enums as od
import time


def encoder_error_string(errno):
    error = "Unknown"
    if errno == 0:
        error = "None"
    elif errno == 1:
        error = "UNSTABLE_GAIN"
    elif errno == 2:
        error = "CPR_POLEPAIRS_MISMATCH"
    elif errno == 4:
        error = "NO_RESPONSE"
    elif errno == 8:
        error = "UNSUPPORTED_ENCODER_MODE"
    elif errno == 16:
        error = "ILLEGAL_HALL_STATE"
    elif errno == 32:
        error = "INDEX_NOT_FOUND_YET"
    elif errno == 64:
        error = "ABS_SPI_TIMEOUT"
    elif errno == 128:
        error = "ABS_SPI_COM_FAIL"
    elif errno == 256:
        error = "ABS_SPI_NOT_READY"
    elif errno == 512:
        error = "HALL_NOT_CALIBRATED_YET"
    return error

def print_encoder_error(index, axis):
    errno = axis.encoder.error
    print(f'Encoder{index}: {errno}: {encoder_error_string(errno)}')

def print_axis_error(index, axis):
    print(f'Axis{index}:    {odrv.axis0.error}')

    
odrv = odrive.find_any(serial_number="316133753232")

print(f'Errors:')
print_axis_error(0, odrv.axis0)
print_encoder_error(0, odrv.axis0)




