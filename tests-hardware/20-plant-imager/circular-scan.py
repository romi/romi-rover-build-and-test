import argparse
import math
import time
import json
import os
from datetime import datetime
from romi.remote_device import OquamXYTheta
from romi.camera import Camera

def new_session_id():
    return datetime.now().strftime("%Y%m%d-%H%M%S")

def create_session(db, session_id):
    path = os.path.join(db, session_id)
    os.makedirs(path)
    path = os.path.join(db, session_id, "metadata")
    os.makedirs(path)
    
def create_fileset(db, session_id, fileset_id):
    path = os.path.join(db, session_id, fileset_id)
    os.makedirs(path)
        
def store_image(db, session_id, fileset_id, name, image):
    path = os.path.join(db, session_id, fileset_id, name)
    image.save(path)

def store_metadata(db, session_id, fileset_id, metadata):
    path = os.path.join(db, session_id, "metadata", f"{fileset_id}.json")
    with open(path, 'w') as outfile:
        json.dump(metadata, outfile, indent=4)

    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--registry', type=str, default="127.0.0.1",
                    help='IP address of the registry')
    parser.add_argument('--count', type=int, default=36,
                    help='The number of images')
    parser.add_argument('--database', type=str, default=".",
                        help='The path to the database (WORK IN PROGRESS)')
    args = parser.parse_args()
    
    cnc = OquamXYTheta("cnc", args.registry)
    cnc.power_up()
    cnc.homing()

    camera_names = ["camera-top"]
    cameras = []
    
    for name in camera_names:
        camera = Camera(name, args.registry)
        cameras.append(Camera(name, args.registry))
        # TODO: read from settings?
        camera.select_option('exposure-mode', 'off')
        camera.set_value('analog-gain', 10)
        camera.set_value('shutter-speed', 60000)
        #camera.set_value('jpeg-quality', 95)



    db = args.database
    session_id = new_session_id()
    fileset_id = "images"
    create_session(db, session_id)
    create_fileset(db, session_id, fileset_id)

    
    dimension = cnc.get_range()
    print(f"dimension={dimension}")

    xc = (dimension[0][0] + dimension[0][1]) / 2.0
    yc = (dimension[1][0] + dimension[1][1]) / 2.0
    d = min(dimension[0][1] - dimension[0][0],
            dimension[1][1] - dimension[1][0])
    r = (d - 0.02) / 2
    print(f"c=({xc}, {yc}), r={r}")

    speed = 0.8
    N = args.count
    angle = -2.0 * math.pi / N
    
    cnc.moveto(xc - r, yc, 0.0, speed)

    metadata = { 'cameras': camera_names }

    for i in range(N):
        for j in range(len(cameras)):
            camera_name = camera_names[j]
            camera = cameras[j]
            image = camera.grab()
            name = f"{camera_name}-{i:05d}.jpg"
            store_image(db, session_id, fileset_id, name, image)
            position = cnc.get_position()
            metadata[name] = {'camera': camera_name,
                              'index': i,
                              'position': {'x': position['x'],
                                           'y': position['y'],
                                           'pan': position['z']}}
            cnc.helix(xc, yc, angle, angle, speed)

    store_metadata(db, session_id, fileset_id, metadata)
        
    cnc.moveto(0, 0, 0, speed)
