
cnc_width = 0.950
cnc_height = 0.900
image_width = 1640
image_height = 1232

ax = 0.0010166906502002402
ay = -0.0010129423469800148
bx = -0.3413004224953589
by = 1.1676487581890997
                                                      
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
