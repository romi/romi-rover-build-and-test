import numpy as np
import matplotlib.pyplot as pl

import glob

cities = np.loadtxt("cities.txt")
files = glob.glob("path-*1.txt")

files.sort()

for f in files:
    print(f)
    path = np.loadtxt(f)
    pl.clf()
    pl.plot(cities[:,0],cities[:,1],"s")
    pl.plot(path[:,0],path[:,1])
    pl.savefig("%s.png" % f)
