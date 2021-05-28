import cv2
import matplotlib.pyplot as pl
import glob
import numpy as np
import os

folder = "images"

files = glob.glob(folder + "/*")
files.sort()

sel_files = files
sel_files = []
for file in files:
    if file[-4:]==".jpg":
        sel_files.append(file)


xy_cnc = []
xy_im = []

for i, file in enumerate(sel_files):
    name = os.path.basename(file)[:-4]	
    x = float(name[12:17])
    y = float(name[18:23])	
        
    im = cv2.imread(file)
    im_hsv = cv2.cvtColor(im, cv2.COLOR_BGR2HSV)

    #lower = np.array([20,96,142])
    #upper = np.array([31,187,255])
    lower = np.array([156 , 79, 0])
    upper = np.array([173 , 202, 255])

    mask = cv2.inRange(im_hsv, lower, upper)
    kernel = np.array([[0, 1, 0],
                       [1, 1, 1],
                       [0, 1, 0]]).astype(np.uint8)
    
    mask = cv2.erode(mask, kernel, iterations=3)
    mask = cv2.dilate(mask, kernel, iterations=3)
    
    cv2.imwrite(file + "-mask.png", mask)
    
    idxs = np.where(mask)
    print(f"{file} {len(idxs[0])}")
    #if len(idxs[0]) > 100:
    if len(idxs[0]) > 400:
        x_im = np.mean(idxs[1])
        y_im = np.mean(idxs[0])
        xy_cnc.append([x, y])	
        xy_im.append([x_im, y_im])
        print("%s: cnc(%0.3f, %0.3f) -> im(%0.3f, %0.3f) (size=%d)"
              % (name, x, y, x_im, y_im, len(idxs[0])))
                                
np.savetxt("%s/pos_cnc.txt" % folder, xy_cnc)
np.savetxt("%s/pos_im.txt" % folder, xy_im)
