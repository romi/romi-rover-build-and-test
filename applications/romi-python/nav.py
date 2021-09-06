import cv2
import numpy as np
import json
from sklearn.decomposition import PCA
import argparse

svm_pars = None
store_masks_flag = True

image_width = 1640
image_height = 1232
#center_x = int(image_width / 2)
center_x = 420
center_y = int(image_height / 2)
center_x1 = center_x - 200
center_x2 = center_x + 200
center_y1 = int(0.0 * image_height)
center_y2 = image_height - int(0.2 * image_height)

def nav_init(path):
    global svm_pars
    svm_pars = json.load(open(path, "r"))

    
def nav_handle_request(params):
    image_path = params["path"]
    cte, orientation = get_cte_orientation(image_path)
    return { 'cross-track-error': cte, 'orientation-error': orientation }


def get_cte_orientation(path):
    global center_y
    global center_x
    mask = get_mask(path)
    store_mask(path, "segmentation", mask)
    h, w = mask.shape
    #filtered_mask = erode_dilate(mask, 15, 15)
    filtered_mask = erode_dilate(mask, 5, 5)
    store_mask(path, "filter", filtered_mask)
    center_mask = get_center_mask(filtered_mask)
    store_mask(path, "center", center_mask)
    center, pc0, pc1 = get_pca(center_mask)
    #cte = get_cross_track_error([h/2, w/2], center, pc0)
    cte = get_cross_track_error([center_y, center_x], center, pc0)
    orientation = get_rel_orientation(center, pc0)
    store_pca(path, center, pc0, pc1, cte, orientation)
    return cte, orientation


def get_mask(path):
    global svm_pars
    im = cv2.imread(path)
    h, w, _ = im.shape
    im_vec = im.reshape([h*w,3])

    pred = ((im_vec @ np.asarray(svm_pars["coef"])
             + svm_pars["intercept"])>0).reshape([h, w])
    return (pred * 255).astype(np.uint8)


def store_mask(path, postfix, mask):
    global store_masks_flag
    if store_masks_flag:
        cv2.imwrite(f"{path}-{postfix}.png", mask)

    
def erode_dilate(mask, er_it=5, dil_it=25):
    kernel = np.array([[0, 1, 0],
                       [1, 1, 1],
                       [0, 1, 0]]).astype(np.uint8)

    res = cv2.erode(mask, kernel, iterations=er_it)
    res = cv2.dilate(res, kernel, iterations=dil_it)
    return res


def get_center_mask_TODO(mask):
    h, w = mask.shape 
    ncc, cc, bbox,centers = cv2.connectedComponentsWithStats(mask.astype(np.uint8))
    center_mask = np.zeros_like(mask)
    for k in range(1, ncc):
        if (centers[k][0]  > .33 * w) * (centers[k][0] < .66 * w):
            center_mask[cc == k] = 255
    return center_mask


def get_center_mask(mask):
    global center_x1
    global center_x2
    global center_y1
    global center_y2
    # Crop central window of (W, H) = (1000, 800)
    h, w = mask.shape
    center_mask = mask
    center_mask[0:center_y1, :] = 0
    center_mask[center_y2:h, :] = 0
    center_mask[:, 0:center_x1] = 0
    center_mask[:, center_x2:w] = 0
    return center_mask


def get_pca(im):
    X = np.squeeze(np.where(im==255)).T #get coordinates of white pixels

    # TODO: handle case with zero white pixels
    number_white_pixels, _ = X.shape
    print(f"***  #pixels {number_white_pixels} ***")
    
    pca = PCA(n_components=2)
    pca.fit(X)

    center = pca.mean_

    v = pca.explained_variance_
    pcs = pca.components_

    pc0 = center + pcs[0] * np.sqrt(v[0])
    pc1 = center + pcs[1] * np.sqrt(v[1])
    return center, pc0, pc1


def get_sign_cte(x0, x1, x2):
    a = -(x2[1] - x1[1]) / (x2[0] - x1[0])
    c = -(a * x1[0] + x1[1])
    return np.sign(a * x0[0] + x0[1] + c)


def get_cross_track_error(x0, x1, x2):
    s = get_sign_cte(x0, x1, x2)
    return (-s
            * np.abs((x2[0] - x1[0]) * (x1[1] - x0[1])
                     - (x1[0] - x0[0]) * (x2[1] - x1[1]))
            / np.sqrt((x2[0] - x1[0])**2 + (x2[1] - x1[1])**2))


def get_rel_orientation(x1, x2):
    pts = np.array([x1,x2])
    pts = pts[pts[:, 0].argsort()]
    return np.arctan2(pts[0][0]-pts[1][0], pts[0][1]-pts[1][1])+np.pi/2


def store_pca(path, center, pc0, pc1, cte, orientation):
    im = cv2.imread(path)
    cv2.line(im,
             tuple(center[::-1].astype(np.int32)),
             tuple(pc0[::-1].astype(np.int32)),
             (0,0,255),5)
    cv2.line(im,
             tuple(center[::-1].astype(np.int32)),
             tuple(pc1[::-1].astype(np.int32)),
             (255,0,0),5)
    cv2.putText(im, f"CTE:{cte:0.3f}", (100, 200),
                cv2.FONT_HERSHEY_SIMPLEX, 4, color=(0, 0,255), thickness=10)
    cv2.putText(im, f"OR.:{orientation*180/np.pi:0.3f}", (100, 1000),
                cv2.FONT_HERSHEY_SIMPLEX, 4, color=(0, 0,255), thickness=10)   
    cv2.imwrite(path + "-components.png", im)

    
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file")
    parser.add_argument('--weights', type=str, nargs='?',
                        default="nav/default.json",
                        help='Path to SVM weights file')
    args = parser.parse_args()
    
    nav_init(args.weights)
    cte, orientation = get_cte_orientation(args.file)
    print(f"cte={cte}")
    print(f"orientation={orientation}")
