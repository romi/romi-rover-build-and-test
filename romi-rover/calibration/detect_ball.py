import cv2
import matplotlib.pyplot as pl
import glob
import numpy as np
import os

#res_folder="/home/kodda/Dropbox/p2pflab/LettuceCalibrate/robot_cnc_yellowball/data/detect_ball"
res_folder = "/home/hanappe/projects/ROMI/DATA/2020/Chatelain/calibration-20200910"


#folder="/home/kodda/Data/robot_chatelain_2020/calibration-20200903"
folder = "/home/hanappe/projects/ROMI/DATA/2020/Chatelain/calibration-20200910"

files=glob.glob(folder+"/*")
files.sort()

sel_files = []
for file in files:
        #if file[-7:]=="315.jpg": sel_files.append(file)
        if file[-7:]=="000.jpg": sel_files.append(file)


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
        lower = np.array([130, 197, 109])
        upper = np.array([154, 255, 255])

        mask = cv2.inRange(im_hsv, lower, upper)
        idxs = np.where(mask)
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
