import subprocess
import sys

subprocess.call(["mkdir","-p","tcp-multiple-flow-results"])

null = open('/dev/null','w')

print "Simulating multiple TCP flows with queue 1000..."
p = subprocess.Popen(["../waf","--run",
                      "tcp-multiple-flow"],
                      shell=False,stdout=null,stderr=null)
p.wait()
subprocess.call("mv ../tcp-multiple-flow.tr tcp-multiple-flow-results/newreno-1000.tr",shell=True)

print "Simulating multiple TCP flows with queue 100..."
p = subprocess.Popen(["../waf","--run",
                      "tcp-multiple-flow --queueSize=100"],
                      shell=False,stdout=null,stderr=null)
p.wait()
subprocess.call("mv ../tcp-multiple-flow.tr tcp-multiple-flow-results/newreno-100.tr",shell=True)
