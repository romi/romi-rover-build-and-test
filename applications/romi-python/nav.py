import cv2
import numpy as np
import json
from sklearn.decomposition import PCA

svm_pars = None

def get_mask(path):
    global svm_pars
    im = cv2.imread(path)
    h, w, _ = im.shape
    im_vec = im.reshape([h*w,3])

    pred = ((im_vec @ np.asarray(svm_pars["coef"]) + svm_pars["intercept"])>0).reshape([h, w])
    return (pred*255).astype(np.uint8)

def erode_dilate(mask, er_it=5, dil_it=25):
    kernel = np.array([[0, 1, 0],
                       [1, 1, 1],
                       [0, 1, 0]]).astype(np.uint8)

    res = cv2.erode(mask, kernel, iterations=er_it)
    res = cv2.dilate(res, kernel, iterations=dil_it)
    return res

def get_pca(im):
    X = np.squeeze(np.where(im==255)).T #get coordinates of white pixels

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
    return (s
            * np.abs((x2[0] - x1[0]) * (x1[1] - x0[1])
                     - (x1[0] - x0[0]) * (x2[1] - x1[1]))
            / np.sqrt((x2[0] - x1[0])**2 + (x2[1] - x1[1])**2))


def get_rel_orientation(x1, x2):
    pts = np.array([x1,x2])
    pts = pts[pts[:, 0].argsort()]
    return np.arctan2(pts[0][0]-pts[1][0], pts[0][1]-pts[1][1])+np.pi/2


def get_center_mask(mask):
    h, w = mask.shape 
    ncc, cc, bbox,centers = cv2.connectedComponentsWithStats(mask.astype(np.uint8))
    center_mask = np.zeros_like(mask)
    for k in range(1, ncc):
        if (centers[k][0]  > .33 * w) * (centers[k][0] < .66 * w):
            center_mask[cc == k] = 255
    return center_mask

def get_cte_orientation(path):
    mask = get_mask(path)
    h, w = mask.shape
    filtered_mask = erode_dilate(mask, 15, 15)
    center_mask = get_center_mask(filtered_mask)
    center, pc0, pc1 = get_pca(center_mask)
    cte = get_cross_track_error([h/2,w/2], center, pc0)
    orientation = get_rel_orientation(center, pc0)
    return cte, orientation


def nav_init(path):
    global svm_pars
    svm_pars = json.load(open(path, "r"))


def nav_handle_request(params):
    image_path = params["path"]
    return get_cte_orientation(image_path)


if __name__ == "__main__":

    nav_init("nav/camera-000000_2000_noweed.json")
    path = "test-data/im.png"
    cte, orientation = get_cte_orientation(path)

    im = cv2.imread(path)
    mask = get_mask(path)
    h, w = mask.shape
    filtered_mask = erode_dilate(mask, 15, 15)
    center, pc0, pc1 = get_pca(filtered_mask)
    cte = get_cross_track_error([h/2,w/2], center, pc0)
    orientation = get_rel_orientation(center, pc0)
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
    cv2.imwrite("test.png", im)
