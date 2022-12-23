Import("env")
import os, shutil

def copyFirmware(source, target, env):
    fromC = os.path.join(env["PROJECT_DIR"], str(target[0]))
    toC = os.path.join(env["PROJECT_DIR"], "openocd-upload", str(target[0].name))
    shutil.copyfile(fromC, toC)

env.AddPostAction("$BUILD_DIR/firmware.bin", copyFirmware)


