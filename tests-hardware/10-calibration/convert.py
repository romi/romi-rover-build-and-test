
cnc_width = 0.810
cnc_height = 0.900
image_width = 1640
image_height = 1232

ax = 0.0010250919430117701
ay = -0.0010320257301022603
bx = -0.43424592896177727
by = 1.159818057590064
 
def camera_to_cnc(x, y):
    return ax * x + bx, ay * y + by

def cnc_to_camera(x, y):
    return (x - bx) / ax, (y - by) / ay

print(camera_to_cnc(0, 0))
print(cnc_to_camera(0, 0))

x0, y0 = cnc_to_camera(0, 0)
x1, y1 = cnc_to_camera(cnc_width, cnc_height)

crop_width = x1 - x0
crop_height = y0 - y1

workspace = [int(x0), int(image_height - y0),
             int(crop_width), int(crop_height)]

print(workspace)
