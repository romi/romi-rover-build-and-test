import sys
import asyncio
import json
import argparse
from romi.rpc import Server
from keras_segmentation.models.all_models import model_from_name
import cv2
import time
import numpy as np
import os
import pickle as pk
from itertools import combinations

model = None
model_config = None
svm_name = "challenge_rose_017_1000"
svm_coeff = None
svm_intercepts = None

def load_unet_model(model_path):
    global model
    global model_config
    config_path = f"{model_path}/submission1/Weedelec_Haricot/_config.json"
    model_config = json.loads(open(config_path, "r").read())
    model = model_from_name[model_config['model_class']](model_config['n_classes'],
                                                     input_height=model_config['input_height'],
                                                     input_width=model_config['input_width'])
    weights = f"{model_path}/submission1/Weedelec_Haricot/.14"
    model.load_weights(weights)


def load_svm_model(path):
    global svm_name
    global svm_coeff
    global svm_intercepts
    svm_coeff = np.loadtxt(f"{path}/%s_coefs.txt" % svm_name)
    svm_intercepts = np.loadtxt("{path}/%s_intercepts.txt" % svm_name)


def erode_dilate(mask, er_it=5, dil_it=25):
    kernel = np.array([[0, 1, 0],
	               [1, 1, 1],
	               [0, 1, 0]]).astype(np.uint8)
    res = cv2.erode(mask, kernel, iterations=er_it)
    res = cv2.dilate(res, kernel, iterations=dil_it)
    return res


def get_pred_svm(path):
    global svm_coeff
    global svm_intercepts
    image = cv2.imread(path)
    h, w, _ = image.shape
    Nc = 4
    xs = image.reshape([h*w, 3])
    classes = np.arange(Nc)
    combos = list(combinations(classes, 2))
    votes = np.zeros([h*w, Nc])
    for i in range(len(svm_intercepts)):
        dec = (xs @ svm_coeff[i] + svm_intercepts[i])
        votes[:, combos[i][0]] += (dec > 0)
        votes[:, combos[i][1]] += (dec < 0)
        pred = np.argmax(votes, 1).reshape([h,w])
    return pred


def store_svm_mask(pred, output_path):
    blue_tags = (pred == 2).astype(np.uint8) * 255
    dil_blue_tags = erode_dilate(blue_tags, er_it=10, dil_it=50)
    cv2.imwrite(output_path, dil_blue_tags)

    
def run_svm(path, output_name):
    folder = os.path.dirname(path)
    pred = get_pred_svm(path)
    store_svm_mask(pred, f"{folder}/{output_name}.png")


def handle_svm_request(params):
    image_path = params["path"]
    output_name = params["output-name"]
    print(f"New request, image {image_path}")
    return run_svm(image_path, output_name)


def get_pred_unet(path, output_name):
    folder = os.path.dirname(path)
    filename = os.path.basename(path)
    img = cv2.imread(path)
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
        
        cv2.imwrite(folder + "/" + output_name + "_rgb.png", seg_img)
        cv2.imwrite(folder + "/" + output_name + ".png", seg_img[:,:,0])
    


    
def handle_unet_request(params):
    image_path = params["path"]
    output_name = params["output-name"]
    print(f"New request, image {image_path}")
    return get_pred_unet(image_path, output_name)


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
    parser.add_argument('--svm-path', type=str, nargs='?', default=".",
                        help='Set the path to the SVM data files')
    parser.add_argument('--registry', type=str, nargs='?', default="10.10.10.1",
                    help='Set the IP address of the registry')
    parser.add_argument('--ip', type=str, nargs='?', default="10.10.10.1",
                    help='The local IP address to use')
    args = parser.parse_args()

    load_unet_model(args.model_path)
    load_svm_model(args.svm_path)
    
    server = Server("python",
                    {
                        "unet": handle_unet_request
                        "svm": handle_svm_request
                    },
                    args.registry, args.ip)
    
    loop = asyncio.get_event_loop()
    loop.run_until_complete(init())
    asyncio.get_event_loop().run_forever()
