from unet import unet_init, get_pred_unet
import json
import cv2
import numpy as np
import os
import argparse
from SupportVectorMachine import SupportVectorMachine

def triple_init(svm_path_1, svm_path_2):
    global include_module, exclude_module
    include_module = SupportVectorMachine(svm_path_1) # bleu tags
    include_module.set_erode_dilate(3, 25)
    exclude_module = SupportVectorMachine(svm_path_2) # yellow tags
    exclude_module.set_erode_dilate(3, 40)

def store(mask, output_path):
    cv2.imwrite(output_path, mask)

def triple_run(path, output_name):
    folder = os.path.dirname(path)
    mask = get_pred_unet(path, "unet")
    include = include_module.run(path, "blue")
    exclude = exclude_module.run(path, "yellow")
    result = (mask | include) & ~exclude 
    store(result, f"{folder}/{output_name}.png")
    return result
    
def triple_handle_request(params):
    print("Running triple_handle_request")
    start_time = time.time()
    image_path = params["path"]
    output_name = params["output-name"]
    print(f"New request, image: {image_path}, output name: {output_name}")
    triple_run(image_path, output_name)
    now = time.time()
    print(f"handle_unet_request: {now-start_time:0.3f} seconds")
    return True

if __name__ == "__main__":
    unit_init("/home/romi/ACRE/models/unet_model_chatelain_20210605")
    triple_init("svm0/blue_tags.json", "svm0/yellow-hose_0010_1000.json")
    triple_run("svm0/test-triple-yellow.png", "triple")

