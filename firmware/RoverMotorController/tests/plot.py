import argparse
import numpy as np
from sklearn.linear_model import LinearRegression
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties


def show_results(file, handles):
    values = np.loadtxt(file)
    time = values[:,0] / 1000.0
    target = values[:,1]
    delta_target = values[:,2]
    delta = values[:,3]
    error = values[:,4]
    sum = values[:,5]
    out = values[:,6]
     
    p1, = plt.plot(time, delta_target, label=f'Left target ({file})')
    p2, = plt.plot(time, delta, label=f'Left measured ({file})')
    handles.append(p1)
    handles.append(p2)
    
def show_all_results(files):
    plt.clf()
    plt.xlabel('Signal')
    plt.ylabel('Speed')
    handles = []
    for f in files:
        show_results(f, handles)
    fontP = FontProperties()
    fontP.set_size('small')
#    plt.legend(handles = handles,
#               title = 'legend',
#               bbox_to_anchor=(1.05, 1),
#               loc = 'upper left', prop = fontP)
    plt.legend(handles = handles,
               title = 'legend',
               prop = fontP)
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file", nargs='+')
    args = parser.parse_args()
    show_all_results(args.file)
