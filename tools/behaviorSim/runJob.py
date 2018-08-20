#!/usr/bin/env python
# encoding: utf-8

import sys
dir = sys.argv[1]
jobNum = int(sys.argv[2])
numEvals = int(sys.argv[3])

ind = jobNum / numEvals
filename = dir + '/%i.json' % ind

import subprocess
subprocess.call(['/u/sbarrett/Nao/trunk/bin/behaviorSim',filename])
