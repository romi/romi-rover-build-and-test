import sys
import asyncio
import json
import argparse
from romi.rpc import Server
from keras_segmentation.models.all_models import model_from_name
import cv2
import time
import numpy as np

model = None
model_config = None

def load_model(model_path):
    global model
    global model_config
    config_path = f"{model_path}/submission1/Weedelec_Haricot/_config.json"
    model_config = json.loads(open(config_path, "r").read())
    model = model_from_name[model_config['model_class']](model_config['n_classes'],
                                                     input_height=model_config['input_height'],
                                                     input_width=model_config['input_width'])
    weights = f"{model_path}/submission1/Weedelec_Haricot/.14"
    model.load_weights(weights)


def get_pred(folder):
   img = cv2.imread(folder + "/unet.jpg")
   h, w, _ = img.shape
   img = cv2.resize(img, (model_config['input_width'],
                          model_config['input_height']))
   img = img.astype(np.float32)
   img[:, :, 0] -= 103.939
   img[:, :, 1] -= 116.779
   img[:, :, 2] -= 123.68
   img = img[:, :, ::-1]

   t0 = time.time()
   pr = model.predict(np.array([img]))[0]
   
   pr = pr.reshape((model.output_height,
                    model.output_width,
                    model.n_classes)).argmax(axis=2)

   colors=[[0,0,0], [255,255,255], [0,0,255]]

   seg_img = np.zeros((model.output_height, model.output_width, 3))

   for c in range(model.n_classes):
      seg_arr_c = pr[:, :] == c
      seg_img[:, :, 0] += ((seg_arr_c)*(colors[c][0])).astype('uint8')
      seg_img[:, :, 1] += ((seg_arr_c)*(colors[c][1])).astype('uint8')
      seg_img[:, :, 2] += ((seg_arr_c)*(colors[c][2])).astype('uint8')

   seg_img = cv2.resize(seg_img, (w, h))
   
   cv2.imwrite(folder+"/mask_rgb.png", seg_img)
   cv2.imwrite(folder+"/mask.png", seg_img[:,:,0])
    


    
def handle_unet_request(params):
    folder = params["path"]
    print(f"New request, path {folder}")
    return get_pred(folder)

async def server_callback(websocket, path):
    global server
    await server.handle_client(websocket)

async def init():    
    global server
    loop = asyncio.get_event_loop()
    registration = loop.create_task(server.register())
    await registration
    await server.start(server_callback)

if __name__ == "__main__":
    global server

    parser = argparse.ArgumentParser()
    parser.add_argument('--model-path', type=str, nargs='?', default="models",
                        help='Set the path to the model data')
    parser.add_argument('--registry', type=str, nargs='?', default="10.10.10.1",
                    help='Set the IP address of the registry')
    parser.add_argument('--ip', type=str, nargs='?', default="10.10.10.1",
                    help='The local IP address to use')
    args = parser.parse_args()

    load_model(args.model_path)
    
    server = Server("python", { "unet": handle_unet_request }, args.registry, args.ip)
    
    loop = asyncio.get_event_loop()
    loop.run_until_complete(init())
    asyncio.get_event_loop().run_forever()
