import cv2
import numpy as np
import json
import os
import argparse

svm_pars = None
store_masks_flag = True

def svm0_init(path):
    print("Running svm0_init")
    global svm_pars
    print(f"svm0_init: loading {path}")
    svm_pars = json.load(open(path, "r"))
    print(svm_pars)
    
def get_pred_svm(path):
    global svm_pars
    im = cv2.imread(path)
    h, w, _ = im.shape
    im_vec = im.reshape([h*w,3])

    pred = ((im_vec @ np.asarray(svm_pars["coef"])
             + svm_pars["intercept"])>0).reshape([h, w])
    return (pred * 255).astype(np.uint8)


def erode_dilate(mask, er_it=5, dil_it=25):
    kernel = np.array([[0, 1, 0],
	               [1, 1, 1],
	               [0, 1, 0]]).astype(np.uint8)
    res = cv2.erode(mask, kernel, iterations=er_it)
    res = cv2.dilate(res, kernel, iterations=dil_it)
    return res

def store_svm_mask(pred, output_path):
    cv2.imwrite(output_path, pred)

def clip_mask(mask):
    h, w = mask.shape
    x1 = int(0.0 * w)
    x2 = w - x1
    y1 = int(0.0 * h)
    y2 = h - int(0.0 * h)
    center_mask = mask
    center_mask[0:y1, :] = 255
    center_mask[y2:h, :] = 255
    center_mask[:, 0:x1] = 255
    center_mask[:, x2:w] = 255
    return center_mask
    
def run_svm(path, output_name):
    folder = os.path.dirname(path)
    pred = get_pred_svm(path)
    pred = erode_dilate(pred, er_it=10, dil_it=10)
    pred = clip_mask(pred)
    store_svm_mask(pred, f"{folder}/{output_name}.png")
    return True

def svm0_handle_request(params):
    print("Running svm0_handle_request")
    image_path = params["path"]
    output_name = params["output-name"]
    print(f"New request, image {image_path}")
    return run_svm(image_path, output_name)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file")
    parser.add_argument('--weights', type=str, nargs='?',
                        default="svm0/blue_tags.json",
                        help='Path to SVM weights file')
    args = parser.parse_args()
    
    svm0_init(args.weights)
    run_svm(args.file, "test")
