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
        self.rtts = {}
        self.totals = {}
    
    def parse(self,filename,dest,rtt):
        sum = {}
        f = open(filename)
        for line in f.readlines():
            p = Packet(line)
            self.max_time = p.time
            if p.event == "r" and p.ip.dest == dest and not p.tcp.syn:
                # find port
                port = p.tcp.dest_port
                # record packet
                if port not in sum:
                    sum[port] = 0
                sum[port] += p.ip.length
        for port in sum.keys():
            if port not in self.rtts:
                self.rtts[port] = []
                self.totals[port] = []
            self.rtts[port].append(rtt)
            self.totals[port].append(8*sum[port]/self.max_time/1000000)

    def plot(self,name):
        clf()
        for port in self.rtts.keys():
            plot(self.rtts[port],self.totals[port],linewidth=0.5)
        xlabel("RTT (ms)")
        ylabel("Rate (Mbps)")
        savefig(name)
    
rtts = range(10,110,10)
t = TcpPlot()
for rtt in rtts:
    t.parse("tcp-rtt-%d.tr" % (rtt),"10.1.1.2",rtt)
t.plot("rtt-rate.png")
