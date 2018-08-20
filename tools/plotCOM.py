#!/usr/bin/env python

def main(ind,filenames):
  import matplotlib.pyplot as plt
  import os.path
  for filename in filenames:
    coms = loadCOMs(filename)
    graphCOMs(ind,coms)
  labels = [os.path.splitext(os.path.basename(x))[0] for x in filenames]
  plt.legend(labels)
  plt.show()

def loadCOMs(filename):
  f = open(filename,'r')
  coms = []
  for line in f:
    if line[0] != '(':
      continue
    line = line.replace('(','').replace(')','').strip()
    com = map(float,line.split(','))
    coms.append(com)
  return coms

def graphCOMs(ind,coms):
  import matplotlib.pyplot as plt
  xs = range(len(coms))
  ys = [com[ind] for com in coms]
  plt.plot(xs,ys)
  plt.scatter(xs,ys)
  #plt.show()

if __name__ == '__main__':
  import sys

  ind = 1
  for i in range(5):
    if str(i) in sys.argv:
      sys.argv.remove(str(i))
      ind = i
      break
  if (len(sys.argv) <= 1) or (sys.argv[1] in ['-h','--help','-help']):
    print 'usage: ./plotCOM.py filename [filename ...]'
  main(ind,sys.argv[1:])
