#!/usr/bin/env python

import re

def isEnd(line):
  if line == '':
    # eof
    return True
  if line[:3] == '---':
    # section is done, we're done
    return True
  return False

def readGroundTruth(f):
  while True:
    line = f.readline()
    #print 'gline:',line
    if isEnd(line):
      return None
    if line[0] == '#':
      continue
    res = re.search('(\d+)\s*cm',line)
    if res is None:
      continue
    dist = int(res.group(1))
    dist *= 10 # for cm to mm conversion
    return dist

def readMeasured(f):
  while True:
    line = f.readline()
    #print 'mline:',line
    if isEnd(line):
      return None
    if line[0] == '#':
      continue
    res = re.search('Ball distance for diameter\(([.0-9]+)\)',line)
    if res is None:
      res = re.match('\s*\n',line)
      if res is None:
        continue
      else:
        return None
    return float(res.group(1))


grounds = []
diameters = []
with open('ballfit.txt','r') as f:
  #line = f.read()
  while True:
    ground = readGroundTruth(f)
    if ground is None:
      break
    #print 'processing with ground',ground
    usedGround = False
    while True:
      diameter = readMeasured(f)
      if diameter is None:
        break
      print ground,diameter
      grounds.append(ground)
      diameters.append(diameter)
      usedGround = True
    if not usedGround:
      print 'WARNING didn\'t use ground for: %g' % ground

import numpy
grounds = numpy.array(grounds)
diameters = numpy.array(diameters)

# fit function: y = a * (x^b)
# log y = log(a) + b * log(x)

# y = grounds
# x = widths/heights

logy = numpy.log(grounds)
logd = numpy.log(diameters)

bw,logaw = numpy.polyfit(logd,logy,1)
aw = numpy.exp(logaw)
widthFunc = lambda x: aw * (x ** bw)
print '  return %f * powf(dx,%f);' % (aw,bw)

import matplotlib.pyplot as plt
plt.scatter(diameters,grounds,color='r')
ds = range(int(diameters[0])+1,int(diameters[-1])-1,-1)
plt.plot(ds,map(widthFunc,ds),color='r')

plt.show()
