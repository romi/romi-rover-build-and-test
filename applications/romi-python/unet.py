import json
from keras_segmentation.models.all_models import model_from_name
import cv2
import time
import numpy as np
import os

model = None
model_config = None


def load_model(model_path):
    global model
    global model_config
    config_path = f"{model_path}/_config.json"
    model_config = json.loads(open(config_path, "r").read())
    model = model_from_name[model_config['model_class']](model_config['n_classes'],
                                                     input_height=model_config['input_height'],
                                                     input_width=model_config['input_width'])
    weights = f"{model_path}/.14"
    model.load_weights(weights)

    
def pre_load_libs():
    image_path = "./dummy_image.png"
    output_name = "dummy_image_out"
    image = np.zeros([10,10,3]).astype(np.uint8)
    cv2.imwrite(image_path, image)
    get_pred_unet(image_path, output_name)


def get_pred_unet(path, output_name):
    start_time = time.time()
    folder = os.path.dirname(path)
    filename = os.path.basename(path)
    img = cv2.imread(path)
    h, w, _ = img.shape

    now = time.time()
    print(f"get_pred_unet: imread {now-start_time:0.3f} seconds")
    
    img = cv2.resize(img, (model_config['input_width'],
                           model_config['input_height']))
    
    now = time.time()
    print(f"get_pred_unet: resize {now-start_time:0.3f} seconds")
    
    img = img.astype(np.float32)
    img[:, :, 0] -= 103.939
    img[:, :, 1] -= 116.779
    img[:, :, 2] -= 123.68
    img = img[:, :, ::-1]
    
    pr = model.predict(np.array([img]))[0]

    now = time.time()
    print(f"get_pred_unet: predict {now-start_time:0.3f} seconds")
    
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

    now = time.time()
    print(f"get_pred_unet: segment {now-start_time:0.3f} seconds")
    
    cv2.imwrite(folder + "/" + output_name + "_rgb.png", seg_img)

    mask = seg_img[:,:,0]
    cv2.imwrite(folder + "/" + output_name + ".png", mask)

    now = time.time()
    print(f"get_pred_unet: imwrite {now-start_time:0.3f} seconds")

    return mask


def unet_init(model_path):
    print("Running unet_init")
    load_model(model_path)
    pre_load_libs()

    
def unet_handle_request(params):
    print("Running unet_handle_request")
    start_time = time.time()
    image_path = params["path"]
    output_name = params["output-name"]
    print(f"New request, image: {image_path}, output name: {output_name}")
    get_pred_unet(image_path, output_name)
    now = time.time()
    print(f"handle_unet_request: {now-start_time:0.3f} seconds")
    return True

if __name__ == "__main__":
    unet_init("/home/romi/ACRE/models/unet_ACRE_20210922")

    for i in range(10):
        print(f"========================= {i+1}")
        unet_handle_request({'path': '/home/romi/acre/sessions/20210921-184538/Rover_ROMI1_20210921-184748.397/camera-000568.jpg',
                         'output-name': 'mask-000568-test'})
