import numpy as np
from sklearn.linear_model import LinearRegression
import matplotlib.pyplot as pl
import cv2

folder = "/home/hanappe/projects/ROMI/DATA/2020/Chatelain/calibration-20200910"


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

def reg_transform(x,ax,ay,bx,by):
   res_x = ax * x[:,0] + bx
   res_y = ay * x[:,1] + by
   return np.array([res_x,res_y]).T

def rmse(x1,x2):
    s_err = np.mean((x1[:,0]-x2[:,0])**2+(x1[:,1]-x2[:,1])**2)
    return np.sqrt(s_err)     

p_cnc = np.loadtxt("%s/pos_cnc.txt" % folder)
p_im = np.loadtxt("%s/pos_im.txt" % folder)

ax, ay, bx, by = reg_direct(p_im, p_cnc, True)

print({'a': [ax, ay], 'b': [bx, by]})

H, _ = cv2.findHomography(p_im,p_cnc)

pts = p_im.reshape(-1,1,2).astype(np.float32)
h_tr = cv2.perspectiveTransform(pts, H).reshape((len(pts),2)) 

reg_tr = reg_transform(p_im, ax, ay, bx, by)

err_h = rmse(h_tr, p_cnc)
err_reg = rmse(reg_tr, p_cnc)
