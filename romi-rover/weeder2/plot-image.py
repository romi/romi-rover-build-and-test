import numpy as np
import matplotlib.pyplot as pl

def norm2map(y, xm, xM):
   return xm+(xM-xm)*y

image = cv2.imread("image.jpg")
idx = np.where(ds>0)[0][-1]
p_SOM = SOM.norm2map(p_SOMs[idx],cs_m,cs_M)[:,::-1]
stp = np.round(p_SOM, 0).reshape((-1,1,2)).astype(np.int32)

cv2.polylines(image,[stp],False,[145,235,229],8)
cv2.imwrite("SOM.png", cropped_image)





