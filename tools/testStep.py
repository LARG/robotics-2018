#!/usr/bin/env python

import matplotlib.pyplot as plt
import math

step_size = 40
fraction_still = 0.225
fraction_moving = 0.55

def stance_step_fwd(t):
  return 0.5 - t + fraction_still
  #if (t + 0.5) < fraction_still:
    #return swing_step_fwd(t) + 1.0
  #elif (t + 0.5) < fraction_still + fraction_moving:
    ##return -0.5 * math.cos(math.pi * ((t + 0.5) - fraction_still) / fraction_moving)
    ##scale = 10
    ##t = scale * (-0.5 + (t + 0.5 - fraction_still) / fraction_moving)
    ##minVal = sigmoid(-scale * 0.5)
    ##maxVal = sigmoid(scale * 0.5)
    
    ###print -2 * (t - fraction_still) / fraction_moving
    ##return (1 - 2 * m) * (0.5 - (sigmoid(t) - minVal) / (maxVal - minVal))
    #x = (t + 0.5 - fraction_still) / fraction_moving
    #m2 = 0.75# / fraction_moving
    #return -m2 * x + 0.25 + 0.125
  #else:
    #return swing_step_fwd(t) - 1.0
  

def sigmoid(t):
  return 1.0 / (1.0 + math.exp(-t))

def swing_step_fwd(t):
  if (t + 0.5) < fraction_still:
    return -0.5 - t + fraction_still
  if (t + 0.5) < fraction_still + fraction_moving:
    #return -0.5 * math.cos(math.pi * ((t + 0.5) - fraction_still) / fraction_moving)
    scale = 10
    t = scale * (-0.5 + (t + 0.5 - fraction_still) / fraction_moving)
    minVal = sigmoid(-scale * 0.5)
    maxVal = sigmoid(scale * 0.5)
    
    #print -2 * (t - fraction_still) / fraction_moving
    return (1.0 + fraction_still * 2) * (-0.5 + (sigmoid(t) - minVal) / (maxVal - minVal)) + 0.5 + fraction_still
  else:
    return 1.5 - t + fraction_still

numVals = 1000
ts = [x / float(numVals) - 0.5 for x in range(numVals)]

left = []
right = []
xs = []

right.extend(map(stance_step_fwd,ts))
left.extend(map(swing_step_fwd,ts))
xs.extend(ts)

right.extend(map(swing_step_fwd,ts))
left.extend(map(stance_step_fwd,ts))
xs.extend([t + 1.0 for t in ts])

right.extend(map(stance_step_fwd,ts))
left.extend(map(swing_step_fwd,ts))
xs.extend([t + 2.0 for t in ts])

right = [step_size * y for y in right]
left = [step_size * y for y in left]

offset = [abs(l - r) for l,r in zip(left,right)]

plt.plot(xs,left)
plt.plot(xs,right)
plt.plot(xs,offset)
plt.show()
