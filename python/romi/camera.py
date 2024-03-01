import websocket
from PIL import Image
from io import BytesIO
from rcom.rcom_client import RcomClient
import argparse

class Camera(RcomClient):
   
    def __init__(self, topic = 'camera', id = 'camera'):
        super().__init__(topic, id)
        
    def print_error(self, data):
       print(f"Failed to grab the image: {r['error']['message']}")
       
    def grab(self):
        cmd = f'{{"method": "camera:grab-jpeg-binary", "id": "{self.id}"}}'
        self.connection.send(cmd, websocket.ABNF.OPCODE_BINARY)
        data = self.connection.recv()
        if type(data) is str:
            if data == "null":
                print("Failed to grab the image")
                return None
            else:
                self.print_error(data)
                return None
        else:
            return Image.open(BytesIO(data))

    def set_value(self, name, value):
        params = {'name': name, 'value': value}
        self.execute('camera:set-value', params)

    def select_option(self, name, value):
        params = {'name': name, 'value': value}
        self.execute('camera:select-option', params)
       
    def power_up(self):
        self.execute('power-up')
        
    def power_down(self):
        self.execute('power-down')

        

class FakeCamera(RcomClient):

    
    def __init__(self, topic = 'camera', id = 'camera', file = 'test.jpg'):
        super().__init__(topic, id)
        self.image = Image.open(file)
        
    def print_error(self, data):
       print(f"Failed to grab the image: {r['error']['message']}")
       
    def grab(self):
        return self.image

    def set_value(self, name, value):
        params = {'name': name, 'value': value}
        print(f'camera:set-value: {name}={value}')

    def select_option(self, name, value):
        params = {'name': name, 'value': value}
        print(f'camera:select-option: {name}={value}')
       
    def power_up(self):
        #print(f'power-up')
        pass
        
    def power_down(self):
        #print(f'power-down')
        pass

        
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--topic', type=str, nargs='?', default="camera",
                    help='The registry topic')
    parser.add_argument('--file', type=str, nargs='?', default="test.jpg",
                    help='The file for the fake camera')
    args = parser.parse_args()
    
    #camera = Camera(args.topic, args.topic)
    camera = FakeCamera(args.topic, args.topic, args.file)
    for i in range(10):
        image = camera.grab()
        if image != None:
            print(f"Saving {args.topic}-{i:05d}.jpg")
            image.save(f"{args.topic}-{i:05d}.jpg")
