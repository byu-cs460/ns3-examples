import subprocess
import sys

subprocess.call(["mkdir","-p","link-results"])

null = open('/dev/null','w')
f = open('link.output','w')

queues = [1,10,100,1000]
rates = [0.25,0.5,0.75,0.85,0.9,0.95,1.0,1.05,1.10,1.25,1.5,1.75]

for queue in queues:
    for rate in rates:
        print "Simulating queue size %d rate %f ..." % (queue,rate)
        null = open('/dev/null','w')
        f = open('link.output','w')
        p = subprocess.Popen(["../waf","--run",
                              "link --queueSize=%d --udpRate=%f" % (queue,rate)],
                             shell=False,stdout=null,stderr=f)
        p.wait()
        subprocess.call(['mv',"../link.tr","link-results/link-%d-%f.tr" % (queue,rate)])
        subprocess.call(['mv',"link.output","link-results/link-%d-%f.output" % (queue,rate)])

