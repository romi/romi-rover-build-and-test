#!/usr/bin/env python3

import json

def read_json():
    with open('grab.json', 'r') as json_file:
        json_config = json.load(json_file)
    return json_config


def write_config(values):
    with open('grab.config', 'w') as config_file:
        config_file.write(f"width={values['width']}\n")
        config_file.write(f"height={values['height']}\n")
        config_file.write(f"quality={values['quality']}\n")
        config_file.write(f"device={values['device']}\n")


def check_value(values, key):
    if key not in values:
        raise ValueError("Missing value: {key}")
    
def check_values(values):
    check_value(values, 'width')
    check_value(values, 'height')
    check_value(values, 'quality')
    check_value(values, 'device')
    


values = read_json()
check_values(values)
write_config(values)
