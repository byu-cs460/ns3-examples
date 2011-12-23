import subprocess
import sys

subprocess.call(["mkdir","-p","tcp-single-flow-results"])

null = open('/dev/null','w')

print "Simulating a single TCP flow with NewReno..."
p = subprocess.Popen(["../waf","--run",
                      "tcp-single-flow"],
                      shell=False,stdout=null,stderr=null)
p.wait()
subprocess.call("mv ../tcp-single-flow.tr tcp-single-flow-results/newreno.tr",shell=True)
subprocess.call("mv ../tcp-single-flow.cwnd tcp-single-flow-results/newreno.cwnd",shell=True)


print "Simulating a single TCP flow with Tahoe..."
p = subprocess.Popen(["../waf","--run",
                      "tcp-single-flow --protocol=TcpTahoe"],
                      shell=False,stdout=null,stderr=null)
p.wait()
subprocess.call("mv ../tcp-single-flow.tr tcp-single-flow-results/tahoe.tr",shell=True)
subprocess.call("mv ../tcp-single-flow.cwnd tcp-single-flow-results/tahoe.cwnd",shell=True)
