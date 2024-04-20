import websocket
from PIL import Image
from io import BytesIO
from rcom.rcom_client import RcomClient
import argparse
import json

class Config(RcomClient):
   
    def __init__(self, topic = 'config', id = 'config'):
        super().__init__(topic, id)
        
    def has_section(self, name):
        params = {'name': name}
        result = self.execute('has-section', params)
        return result['answer']
        
    def get_section(self, name):
        params = {'name': name}
        return self.execute('get-section', params)
        
    def set_section(self, name, value):
        params = {'name': name, 'value': value}
        return self.execute('set-section', params)
       
    def get(self):
        return self.execute('get')

        
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--topic', type=str, nargs='?', default="config",
                    help='The regsitry topic')
    args = parser.parse_args()
    
    config = Config(args.topic, args.topic)

    print(json.dumps(config.get(), indent=4))
    print("------------------------------------")
    print(config.has_section('camera'))
    print("------------------------------------")
    print(json.dumps(config.get_section('camera'), indent=4))
    print("------------------------------------")
    try:
        config.set_section('non-existant', 4)
    except Exception as e:
        print(f"Error: {e}")
    print("------------------------------------")
    try:
        config.set_section('test', 4)
    except Exception as e:
        print(f"Error: {e}")
