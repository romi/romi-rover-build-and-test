from rcom.rcom_client import RcomClient
from romi.camera import Camera
from romi.cameramount import CameraMount
import argparse

class ImagingDevice(RcomClient):
   
    def __init__(self,
                 topic, 
                 camera_id = 'camera',
                 mount_id = 'mount'):
        super().__init__(topic, topic)
        self.camera = {}
        self.camera[camera_id] = Camera(topic, camera_id, registry)
        self.mount = CameraMount(topic, mount_id, registry)
       
    def power_up(self):
        for id, camera in self.camera.items():
            camera.power_up()
        self.mount.power_up()
        
    def power_down(self):
        self.mount.power_down()
        for id, camera in self.camera.items():
            camera.power_down()

        
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--topic', type=str,
                        help='The registry topic of the imaging device')
    args = parser.parse_args()
    
    device = ImagingDevice(args.topic)
    device.power_up()
    
    for i in range(0, 17):
        device.mount.moveto(i/2.0, 0, 0, 0, 0, 0, 0.5)
        image = device.camera['camera'].grab()
        if image != None:
            print(f"Saving image-{i:05d}.jpg")
            image.save(f"image-{i:05d}.jpg")

    device.mount.moveto(0.1, 0, 0, 0, 0, 0, 0.5)

    
