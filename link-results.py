import matplotlib
from pylab import *

import sys

class DelayPlot:
    def __init__(self):
        self.times = []
        self.delays = []
        self.droptimes = []
        self.drops = []

    def parse(self,filename):
        prior = -1
        f = open(filename)
        for line in f.readlines():
            packet = line.split()
            if packet[0] == "TraceDelay:":
                sequence = int(packet[8])
                delay = float(packet[15][1:-2])
                self.times.append(sequence)
                self.delays.append(delay)
            else:
                print packet
                continue
            for s in range(prior+1,sequence):
                self.droptimes.append(s)
                self.drops.append(0)
            prior = sequence

    def plot(self,name,queue):
        clf()
        scatter(self.times,self.delays,s=0.2)
        if self.droptimes != []:
            scatter(self.droptimes,self.drops,s=0.2)
        xlabel("Sequence Number")
        ylabel("Delay (ns)")
        savefig(name)

class QueuePlot:
    def __init__(self):
        self.times = []
        self.queues = []
        self.droptimes = []
        self.drops = []

    def parse(self,filename):
        queue = 0 
        f = open(filename)
        for line in f.readlines():
            packet = line.split()
            if packet[0] == "+":
                queue += 1
                self.times.append(float(packet[1]))
                self.queues.append(int(queue))
            if packet[0] == "-":
                queue -= 1
                self.times.append(float(packet[1]))
                self.queues.append(int(queue))
            if packet[0] == "d":
                self.droptimes.append(float(packet[1]))
                self.drops.append(-1)

    def plot(self,name,queue):
        clf()
        scatter(self.times,self.queues,s=0.2)
        if self.droptimes != []:
            scatter(self.droptimes,self.drops,s=0.2)
        xlabel("Time (seconds)")
        ylabel("Queue Size")
        ylim([-2,queue*1.1])
        savefig(name)

queues = [1,10,100,1000]
queues = [1,10]
rates = [0.25,0.5,0.75,0.85,0.9,0.95,1.0,1.05,1.10,1.25,1.5,1.75]
for queue in queues:
    for rate in rates:
        d = DelayPlot()
        d.parse("link-results/link-%d-%f.tr" % (queue,rate))
        d.plot("link-results/delay-%d-%f.png" % (queue,rate),queue)


sys.exit(0)

queues = [1,10,100,1000]
rates = [0.25,0.5,0.75,0.85,0.9,0.95,1.0,1.05,1.10,1.25,1.5,1.75]
for queue in queues:
    for rate in rates:
        q = QueuePlot()
        q.parse("link-results/link-%d-%f.tr" % (queue,rate))
        q.plot("link-results/queue-%d-%f.png" % (queue,rate),queue)
