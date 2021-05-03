import numpy as np
import matplotlib.pyplot as pl

cities = np.loadtxt("cities.txt")
path = np.loadtxt("path.txt")

pl.plot(cities[:,0],cities[:,1],"s")
pl.plot(path[:,0],path[:,1])

pl.savefig("path.png")
