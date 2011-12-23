import matplotlib
from pylab import *

import sys

def get_header(line):
    if line == "":
        return (None,None,None)
    fields = line.split()
    htype =  fields[0]
    data = " ".join(fields[1:])
    num = 0
    for c in range(0,len(data)):
        if data[c] == "(":
            num += 1
        if data[c] == ")":
            num -= 1
        if num == 0:
            return (htype,data[1:c],data[c+1:].lstrip())
    return (None,None,None)
        
class TCP:
    def __init__(self,line):
        fields = line.split()
        self.source_port = int(fields[0])
        self.dest_port = int(fields[2])
        self.syn = False
        self.ack = False
        for i in range(len(fields)):
            if fields[i] == "SYN":
                self.syn = True
            if fields[i] == "ACK":
                self.ack = True
            if fields[i].count("=") > 0:
                a,b = fields[i].split("=")
                if a == "Seq":
                    self.seq_number = int(b)
                if a == "Ack":
                    self.ack_number = int(b)
                if a == "Window":
                    self.window = int(b)

class IP:
    def __init__(self,line):
        fields = line.split()
        self.tos = fields[1]
        self.ttl = int(fields[3])
        self.id = fields[5]
        self.protocol = int(fields[7])
        self.offset = int(fields[10])
        self.length = int(fields[14])
        self.source = fields[15]
        self.dest = fields[17]
               
class Packet:
    def __init__(self,line):
        fields = line.split()
        self.event = fields[0]
        self.time = float(fields[1])
        self.device = fields[2]
        self.ip = None
        self.tcp = None
        rest = " ".join(fields[3:])
        while True:
            htype,header,rest = get_header(rest)
            if htype == None:
                break
            if htype == "ns3::Ipv4Header":
                self.ip = IP(header)
            if htype == "ns3::TcpHeader":
                self.tcp = TCP(header)
                         
class TcpPlot:
    def __init__(self):
        self.max_time = 0
        self.packets = {}
        self.times = {}
        self.rates = {}
        self.seq_times = {}
        self.seq_values = {}
        self.ack_times = {}
        self.ack_values = {}
        self.queue_times = []
        self.queue_values = []
        self.drop_times = []
        self.drop_values = []
    
    def parse(self,filename,dest):
        windows = {}
        queue = 0
        f = open(filename)
        for line in f.readlines():
            p = Packet(line)
            self.max_time = p.time
            if p.event == "r" and p.ip.dest == dest and not p.tcp.syn:
                # find port
                port = p.tcp.dest_port
                # initialize packets
                if port not in self.packets:
                    self.packets[port] = []
                # record packet
                self.packets[port].append(p)

            if p.event == "r" and p.ip.dest != dest and not p.tcp.syn and p.time < 2:
                port = p.tcp.source_port
                if port not in self.ack_times:
                    self.ack_times[port] = []
                    self.ack_values[port] = []
                self.ack_times[port].append(p.time)
                ack = p.tcp.ack_number % 146000
                self.ack_values[port].append(ack)


            if p.event == "+":
                queue += 1
                self.queue_times.append(p.time)
                self.queue_values.append(queue)
                if p.ip.dest == dest and not p.tcp.syn and p.time < 2:
                    port = p.tcp.dest_port
                    if port not in self.seq_times:
                        self.seq_times[port] = []
                        self.seq_values[port] = []
                    self.seq_times[port].append(p.time)
                    seq = p.tcp.seq_number % 146000
                    self.seq_values[port].append(seq)
            if p.event == "-":
                queue -= 1
                self.queue_times.append(p.time)
                self.queue_values.append(queue)
            if p.event == "d":
                self.drop_times.append(p.time)
                self.drop_values.append(21)

    def average_rate(self,port,start,end):
        data = 0
        if start < 0:
            start = 0
        if end == 0:
            return 0.0
        for packet in self.packets[port]:
            if packet.time < start:
                continue
            if packet.time > end:
                break
            data += packet.ip.length
        return 8*data/(end - start)/1000000

    def calculate(self):
        for port in self.packets.keys():
            self.times[port] = []
            self.rates[port] = []
        current = 0
        width = 1
        factor = 0.1
        while current < self.max_time:
            start = current - width
            end = current
            for port in self.packets.keys():
                self.times[port].append(end)
                self.rates[port].append(self.average_rate(port,start,end))
            current += factor
            
    def plot(self,name):
        clf()
        for port in self.times.keys():
            plot(self.times[port],self.rates[port],linewidth=0.5)
        xlabel("Time (seconds)")
        ylabel("Rate (Mbps)")
        savefig(name)
    
    def queue_plot(self,name):
        clf()
        plot(self.queue_times,self.queue_values,linewidth=0.5)
        plot(self.drop_times,self.drop_values,'bx')
        xlabel("Time (seconds)")
        ylabel("Queue Size (packets)")
        savefig(name)
    
files = ["newreno-1000","newreno-100"]
for file in files:
    t = TcpPlot()
    t.parse(file + ".tr","10.1.1.2")
    t.calculate()
    t.plot(file + "-rate.png")
    t.queue_plot(file + "-queue.png")
