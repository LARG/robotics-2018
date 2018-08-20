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
    #res = re.search('Goal distance for Width\(([.0-9]+)\),Height\(([.0-9]+)\)',line)
    res = re.search('Goal distance for Width.*,Height.*,\s*from width:\s*([.0-9]+), height:\s*([.0-9]+).*\n',line)
    if res is None:
      res = re.match('\s*\n',line)
      if res is None:
        continue
      else:
        return None
    return float(res.group(1)),float(res.group(2))
    #return float(res.group(1)) * 640.0/1280.0,float(res.group(2)) * 480.0/960.0 # for when you screw up for the higher res


grounds = []
widths = []
heights = []
with open('goalfit.txt','r') as f:
  #line = f.read()
  while True:
    ground = readGroundTruth(f)
    if ground is None:
      break
    #print 'processing with ground',ground
    usedGround = False
    while True:
      res = readMeasured(f)
      if res is None:
        break
      width,height = res
      print ground,width,height
      grounds.append(ground)
      widths.append(width)
      heights.append(height)
      usedGround = True
    if not usedGround:
      print 'WARNING didn\'t use ground for: %g' % ground

import numpy
grounds = numpy.array(grounds)
widths = numpy.array(widths)
heights = numpy.array(heights)

# fit function: y = a * (x^b)
# log y = log(a) + b * log(x)

# y = grounds
# x = widths/heights

logy = numpy.log(grounds)
logw = numpy.log(widths)
logh = numpy.log(heights)

bw,logaw = numpy.polyfit(logw,logy,1)
aw = numpy.exp(logaw)
widthFunc = lambda x: aw * (x ** bw)
print 'return %f * powf(width,%f);' % (aw,bw)

bh,logah = numpy.polyfit(logh,logy,1)
ah = numpy.exp(logah)
heightFunc = lambda x: ah * (x ** bh)
print 'return %f * powf(height,%f);' % (ah,bh)

import matplotlib.pyplot as plt
plt.scatter(widths,grounds,color='r')
ws = range(int(widths[0])+1,int(widths[-1])-1,-1)
ws.append(ws[-1]-1)
plt.plot(ws,map(widthFunc,ws),color='r')

plt.scatter(heights,grounds,color='b')
hs = range(int(heights[0])+1,int(heights[-1])-1,-1)
hs.append(hs[-1] - 0.025 * (max(heights) - min(heights)))
plt.plot(hs,map(heightFunc,hs),color='b')

plt.show()
