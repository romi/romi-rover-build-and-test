import argparse
import numpy as np
import math
import sys
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties

class ConversionSettings():
    def __init__(self):
        self.reset()

    def reset(self):
        self.radian_to_degrees = False
        self.offset_first = False

    def parse(self, names):
        self.reset()
        for name in names:
            if name == "radian-to-degrees":
                self.radian_to_degrees = True
            elif name == "offset-first":
                self.offset_first = True
            else:
                print(f"Unknown conversion: {name}")
                
class DataLog():
    def __init__(self, path, start_time, end_time):
        self.path = path
        self.entries = []
        self.__load(start_time, end_time)
        
    def __load(self, start_time, end_time):
        print(f"Loading from {start_time} to {end_time}")
        with open(self.path, 'r') as f:
            for line in f:
                values = line.split(',')
                time = float(values[0])
                name = values[1]
                value = float(values[2])
                insert = ((start_time <= 0
                           or (start_time > 0 and time >= start_time))
                          and (end_time <= 0
                               or (end_time > 0 and time <= end_time)))                
                if insert:
                    self.entries.append([time-start_time, name, value])

    def select(self, name):
        values = []
        print(len(self.entries))
        for i in range(len(self.entries)):
            if (self.entries[i][1] == name):
                time = self.entries[i][0]
                value = self.entries[i][2]
                values.append([time, value])
        return np.array(values)

    
class TimeData():
    def __init__(self, datalog, name, conversion):
        self.name = name
        self.values = datalog.select(name)
        print(self.values.shape)
        self.convert(conversion)
        
    def convert(self, conversion):
        if conversion.radian_to_degrees:
            self.radian_to_degrees()
            
        if conversion.offset_first:
            self.offset_first()
        
    def radian_to_degrees(self):
        for i in range(len(self.values)):
            self.values[i,1] = 180 * self.values[i,1] / math.pi
        
    def offset_first(self):
        offset = self.values[0,1]
        for i in range(len(self.values)):
            self.values[i,1] = self.values[i,1] - offset
        

class DataViewer():
    def __init__(self, nplots):
        self.handles = []
        plt.clf()
        self.axis = []
        figures, axis = plt.subplots(nplots, 1)
        if nplots == 1:
            self.axis.append(axis)
        else:
            self.axis = axis
            
        for i in range(nplots):
            self.axis[i].set_xlabel('time')
        
    def add(self, index, data):
        time = data.values[:,0]
        values = data.values[:,1]
        p, = self.axis[index].plot(time, values, label=data.name)  
        self.handles.append(p)
            
    def show(self):
        fontP = FontProperties()
        fontP.set_size('small')
        plt.legend(handles=self.handles,
                   title='legend',
                   bbox_to_anchor=(1.01, 1),
                   loc='upper left',
                   prop=fontP)
        plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--file', type=str, nargs='?', default="datalog.txt",
                    help='Path to the datalog file')
    parser.add_argument('--start-time', type=float, nargs='?', default=0.0,
                    help='Start time of the plot')
    parser.add_argument('--end-time', type=float, nargs='?', default=0.0,
                    help='End time of the plot')
    parser.add_argument('--plot', type=str, nargs='?', default="",
                        help='Data to be plotted (comma-separated list of names)')
    parser.add_argument('--plot2', type=str, nargs='?', default="",
                        help='Data to be plotted in second graph')
    parser.add_argument('--plot3', type=str, nargs='?', default="",
                        help='Data to be plotted in third graph')
    parser.add_argument('--plot4', type=str, nargs='?', default="",
                        help='Data to be plotted in third graph')
    parser.add_argument('--conv1', type=str, nargs='?', default="",
                        help='Conversion to be applied')
    parser.add_argument('--conv2', type=str, nargs='?', default="",
                        help='Conversion to be applied')
    parser.add_argument('--conv3', type=str, nargs='?', default="",
                        help='Conversion to be applied')
    parser.add_argument('--conv4', type=str, nargs='?', default="",
                        help='Conversion to be applied')

    args = parser.parse_args()

    if not args.plot:
        print("Nothing to plot")
        sys.exit(1)

    num_plots = 1
    plot_names_1 = args.plot.split(",")
    plot_names_2 = []
    plot_names_3 = []
    plot_names_4 = []
    conv_names_1 = []
    conv_names_2 = []
    conv_names_3 = []
    conv_names_4 = []

    print(f"Plot[1]={plot_names_1}")

    conversion = ConversionSettings();
    
    if args.plot2:
        num_plots = 2
        plot_names_2 = args.plot2.split(",")
        print(f"Plot[2]={plot_names_2}")
        if args.plot3:
            num_plots = 3
            plot_names_3 = args.plot3.split(",")
            print(f"Plot[3]={plot_names_3}")
            if args.plot4:
                num_plots = 4
                plot_names_4 = args.plot4.split(",")
                print(f"Plot[4]={plot_names_4}")

    if args.conv1:
        conv_names_1 = args.conv1.split(",")
    if args.conv2:
        conv_names_2 = args.conv2.split(",")
    if args.conv3:
        conv_names_3 = args.conv3.split(",")
    if args.conv4:
        conv_names_4 = args.conv4.split(",")
            
    datalog = DataLog(args.file, args.start_time, args.end_time)
    dataviewer = DataViewer(num_plots)

    conversion.parse(conv_names_1)
    for name in plot_names_1:
        series = TimeData(datalog, name, conversion)
        dataviewer.add(0, series)
        
    conversion.parse(conv_names_2)
    for name in plot_names_2:
        series = TimeData(datalog, name, conversion)
        dataviewer.add(1, series)
        
    conversion.parse(conv_names_3)
    for name in plot_names_3:
        series = TimeData(datalog, name, conversion)
        dataviewer.add(2, series)
        
    conversion.parse(conv_names_4)
    for name in plot_names_4:
        series = TimeData(datalog, name, conversion)
        dataviewer.add(3, series)
    
    dataviewer.show()

    
