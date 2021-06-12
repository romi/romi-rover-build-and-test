import os
import glob
import cv2
import numpy as np
import matplotlib.pyplot as pl
from sklearn.linear_model import LinearRegression
import argparse
import json
import shutil

def main(folder, config_file):
    files = list_files(folder)
    if len(files) < 8:
        raise RuntimeError("Not enough images")
    print(f"Found {len(files)} files");
    image_height = get_image_height(files[0])
    lower, upper = select_lower_and_upper_values(files[0])
    xy_cnc, xy_image = detect_ball(files, lower, upper)
    ax, ay, bx, by = get_transformation(xy_cnc, xy_image)
    cnc_width, cnc_height = get_cnc_dimensions(config_file)
    workspace = compute_workspace(cnc_width, cnc_height,
                                  image_height, ax, bx, ay, by)
    update_config(config_file, workspace)


def list_files(folder):
    files = glob.glob(folder + "/*")
    files.sort()
    sel_files = files
    sel_files = []
    for file in files:
        if file[-4:]==".jpg":
            sel_files.append(file)
    return sel_files


def get_image_height(file):
    image = cv2.imread(file)
    h, w, _ = image.shape
    return h
    
def select_lower_and_upper_values(file):
    # Load image
    image = cv2.imread(file)

    image = cv2.pyrDown(image)

    # Create a window
    cv2.namedWindow('image')

    # Create trackbars for color change
    # Hue is from 0-179 for Opencv
    cv2.createTrackbar('HMin', 'image', 0, 179, nothing)
    cv2.createTrackbar('SMin', 'image', 0, 255, nothing)
    cv2.createTrackbar('VMin', 'image', 0, 255, nothing)
    cv2.createTrackbar('HMax', 'image', 0, 179, nothing)
    cv2.createTrackbar('SMax', 'image', 0, 255, nothing)
    cv2.createTrackbar('VMax', 'image', 0, 255, nothing)

    # Set default value for Max HSV trackbars
    cv2.setTrackbarPos('HMax', 'image', 179)
    cv2.setTrackbarPos('SMax', 'image', 255)
    cv2.setTrackbarPos('VMax', 'image', 255)

    # Initialize HSV min/max values
    hMin = sMin = vMin = hMax = sMax = vMax = 0
    phMin = psMin = pvMin = phMax = psMax = pvMax = 0
    lower = np.array([hMin, sMin, vMin])
    upper = np.array([hMax, sMax, vMax])

    while (True):
        # Get current positions of all trackbars
        hMin = cv2.getTrackbarPos('HMin', 'image')
        sMin = cv2.getTrackbarPos('SMin', 'image')
        vMin = cv2.getTrackbarPos('VMin', 'image')
        hMax = cv2.getTrackbarPos('HMax', 'image')
        sMax = cv2.getTrackbarPos('SMax', 'image')
        vMax = cv2.getTrackbarPos('VMax', 'image')

        # Set minimum and maximum HSV values to display
        lower = np.array([hMin, sMin, vMin])
        upper = np.array([hMax, sMax, vMax])

        # Convert to HSV format and color threshold
        hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        mask = cv2.inRange(hsv, lower, upper)
        result = cv2.bitwise_and(image, image, mask=mask)

        # Print if there is a change in HSV value
        if ((phMin != hMin) | (psMin != sMin) | (pvMin != vMin)
            | (phMax != hMax) | (psMax != sMax) | (pvMax != vMax) ):
            print("(hMin = %d , sMin = %d, vMin = %d), (hMax = %d , sMax = %d, vMax = %d)" % (hMin , sMin , vMin, hMax, sMax , vMax))
            phMin = hMin
            psMin = sMin
            pvMin = vMin
            phMax = hMax
            psMax = sMax
            pvMax = vMax

        # Display result image
        cv2.imshow('image', result)
        if cv2.waitKey(10) & 0xFF == ord('q'):
            break

    cv2.destroyAllWindows()
    return lower, upper 
            

def nothing(x):
    pass


def detect_ball(files, lower, upper):
    xy_cnc = []
    xy_im = []

    for i, file in enumerate(files):
        name = os.path.basename(file)[:-4]	
        x = float(name[12:17])
        y = float(name[18:23])	
        
        im = cv2.imread(file)
        im_hsv = cv2.cvtColor(im, cv2.COLOR_BGR2HSV)
    
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
                                
        np.savetxt("pos_cnc.txt", xy_cnc)
        np.savetxt("pos_im.txt" , xy_im)
        
    return np.array(xy_cnc), np.array(xy_im)


def get_transformation(p_cnc, p_im):
    ax, ay, bx, by = reg_direct(p_im, p_cnc, True)
    print({'a': [ax, ay], 'b': [bx, by]})
    return ax, ay, bx, by


def reg_direct(x, y, plotit=False):
    N = len(x)
    
    ax = ((N*np.sum(x[:,0] * y[:,0]) - np.sum(x[:,0]) * np.sum(y[:,0]))
          / (N * np.sum(x[:,0]**2) - np.sum(x[:,0])**2))
    
    bx = (np.sum(y[:,0]) - ax * np.sum(x[:,0])) / N

    ay = ((N * np.sum(x[:,1] * y[:,1]) - np.sum(x[:,1]) * np.sum(y[:,1]))
          / (N * np.sum(x[:,1]**2) - np.sum(x[:,1])**2))
    
    by = (np.sum(y[:,1]) - ay * np.sum(x[:,1])) / N

    if plotit:
       xtest = np.linspace(min(x[:,0]),max(x[:,0]),100)
       ytest = np.linspace(min(x[:,1]),max(x[:,1]),100)
       pl.subplot(121)
       pl.plot(x[:,0], y[:,0], "o")
       pl.plot(xtest, ax*xtest+bx)

       pl.subplot(122)
       pl.plot(x[:,1], y[:,1], "o")
       pl.plot(ytest, ay*ytest+by)

       pl.show()
    return ax, ay, bx, by


def get_cnc_dimensions(config_file):
    with open(config_file, 'r') as f:
        config = json.load(f)
    width = config["oquam"]["cnc-range"][0][1]
    height = config["oquam"]["cnc-range"][1][1]
    return width, height


def compute_workspace(cnc_width, cnc_height, image_height, ax, bx, ay, by):
    x0, y0 = cnc_to_camera(0, 0, ax, bx, ay, by)
    x1, y1 = cnc_to_camera(cnc_width, cnc_height, ax, bx, ay, by)
    crop_width = x1 - x0
    crop_height = y0 - y1
    workspace = [int(x0), int(image_height - y0),
                 int(crop_width), int(crop_height)]
    print(f"Workspace {workspace}")
    return workspace


def cnc_to_camera(x, y, ax, bx, ay, by):
    return (x - bx) / ax, (y - by) / ay

def update_config(config_file, workspace):
    shutil.copyfile(config_file, config_file + ".backup")
    with open(config_file, 'r') as f:
        config = json.load(f)
    config["weeder"]["imagecropper"]["workspace"] = workspace
    with open(config_file, 'w') as f:
        json.dump(config, f, sort_keys=True, indent=4)
    
    
    
if __name__ == "__main__":
    # execute only if run as a script
    parser = argparse.ArgumentParser()
    parser.add_argument('--folder', type=str, nargs='?', default="images",
                    help='The path to the folder with the calibration images')
    parser.add_argument('--config', type=str, nargs='?', default="config.json",
                        help='Set the configuration file')

    args = parser.parse_args()

    main(args.folder, args.config)
