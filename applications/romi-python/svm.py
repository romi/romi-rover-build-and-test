import cv2
import numpy as np
import os
from itertools import combinations


svm_name = "challenge_rose_017_1000"
svm_coeff = None
svm_intercepts = None


def load_model(path):
    global svm_name
    global svm_coeff
    global svm_intercepts
    svm_coeff = np.loadtxt(f"{path}/%s_coefs.txt" % svm_name)
    svm_intercepts = np.loadtxt(f"{path}/%s_intercepts.txt" % svm_name)


def svm_init(path):
    load_model(path)
    
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
    return True


def svm_handle_request(params):
    image_path = params["path"]
    output_name = params["output-name"]
    print(f"New request, image {image_path}")
    return run_svm(image_path, output_name)
