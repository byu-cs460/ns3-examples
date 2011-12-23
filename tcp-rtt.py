import subprocess
import sys

subprocess.call(["mkdir","-p","tcp-rtt-results"])

null = open('/dev/null','w')

for delay in range(10,110,10):
    print "Simulating TCP flows with delay %d..." % (delay)
    p = subprocess.Popen(["../waf","--run",
                          "tcp-rtt --delay=%d" % (delay)],
                         shell=False,stdout=null,stderr=null)
    p.wait()
    subprocess.call("mv ../tcp-rtt.tr tcp-rtt-results/tcp-rtt-%d.tr" % (delay),shell=True)
