import json
import cv2
import numpy as np
import os
import argparse

class SupportVectorMachine:

    def __init__(self, path):
        self.params = json.load(open(path, "r"))
        self.clip_region = False
        self.filter_params = [5, 25]

    def predict(self, path):
        im = cv2.imread(path)
        h, w, _ = im.shape
        im_vec = im.reshape([h*w,3])
        
        pred = ((im_vec @ np.asarray(self.params["coef"])
                 + self.params["intercept"])>0).reshape([h, w])
        return (pred * 255).astype(np.uint8)

    def set_erode_dilate(self, er_it, dil_it):
        self.filter_params = [er_it, dil_it]

    def erode_dilate(self, mask):
        if self.filter_params:
            return self.do_erode_dilate(mask)
        else:
            return mask

    def do_erode_dilate(self, mask):
        kernel = np.array([[0, 1, 0],
	                   [1, 1, 1],
	                   [0, 1, 0]]).astype(np.uint8)
        res = cv2.erode(mask, kernel, iterations=self.filter_params[0])
        res = cv2.dilate(res, kernel, iterations=self.filter_params[1])
        return res
        
    def store(self, pred, output_path):
        cv2.imwrite(output_path, pred)

    def set_clip(self, x1, x2, y1, y2):
        self.clip_region = [x1, x2, y1, y2]
        
    def clip(self, mask):
        if self.clip_region:
            return self.do_clip(mask)
        else:
            return mask
            
    def do_clip(self, mask):
        h, w = mask.shape
        x1 = int(w * self.clip_region[0])
        x2 = int(w * self.clip_region[1])
        y1 = int(h * self.clip_region[2])
        y2 = int(h * self.clip_region[3])
        center_mask = mask
        center_mask[0:y1, :] = 255
        center_mask[y2:h, :] = 255
        center_mask[:, 0:x1] = 255
        center_mask[:, x2:w] = 255
        return center_mask

    def run(self, path, output_name):
        folder = os.path.dirname(path)
        pred = self.predict(path)
        pred = self.erode_dilate(pred)
        pred = self.clip(pred)
        self.store(pred, f"{folder}/{output_name}.png")
        return pred

    
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file")
    parser.add_argument('--weights', type=str, nargs='?',
                        default="svm0/blue_tags.json",
                        help='Path to SVM weights file')
    args = parser.parse_args()

    svm = SupportVectorMachine(args.weights)
    svm.set_clip(0.3, 0.7, 0.0, 1.0)
    svm.set_erode_dilate(10, 50)
    svm.run(args.file, "test")
