
cnc_width =  0.570
cnc_height =  0.570
image_width =  1920
image_height =  1080

if False:
    ax = 0.0007989
    ay = -0.0008240
    bx = -0.44957
    by = 0.82430
else:
    ax = 0.0006428817307041013
    ay = -0.0006748170112293223
    bx = -0.29410270541484007
    by = 0.749470993954305

def camera_to_cnc(x, y):
    return ax * x + bx, ay * y + by

def cnc_to_camera(x, y):
    return (x - bx) / ax, (y - by) / ay

#print(camera_to_cnc(0, 0))
#print(cnc_to_camera(0, 0))

x0, y0 = cnc_to_camera(0, 0)
x1, y1 = cnc_to_camera(cnc_width, cnc_height)

crop_width = x1 - x0
crop_height = y0 - y1

workspace = [0.0,
             int(x0), int(image_height - y0),
             int(crop_width), int(crop_height),
             cnc_width, cnc_height]

print(workspace)
