#!/usr/bin/env python

import matplotlib.pyplot as plt
import math

g = -9806.65
k = math.sqrt(abs(g / 185))

def calcShiftFrac(t):
  return -1 + 0.5 * (math.cosh(k * t * 0.5) - 1)

def calcOffsetLeftFrac(t,left_swing):
  if left_swing:
    if t + 0.5 < fraction_still:
      return 0
    elif t + 0.5 < fraction_still + fraction_moving:
      return 0.5 * (1 - math.cos(math.pi * (t  + 0.5 - fraction_still) / fraction_moving))
    else:
      return 1
  else:
    if t < 0:
      return 0.25 * (1 - math.cos(math.pi * t / 0.5))
    else: 
      return 0

def calcOffsetRightFrac(t,left_swing):
  if left_swing:
    if t < 0:
      return 0
    else:
      return 0.25 * (math.cos(math.pi * t / 0.5) - 1)
  else:
    if t + 0.5 < fraction_still:
      return -1.0
    elif t + 0.5 < fraction_still + fraction_moving:
      return 0.5 * (-1 - math.cos(math.pi * (t + 0.5 - fraction_still) / fraction_moving))
    else:
      return 0

def calcLeft(t,left_swing):
  if left_swing:
    val = 100 + calcRight(t,left_swing)
  else:
    val = 50 + shift_amount * calcShiftFrac(t)
  val += step_side_size * calcOffsetLeftFrac(t,left_swing)
  return val

def calcRight(t,left_swing):
  if left_swing:
    val = -50 - shift_amount * calcShiftFrac(t)
  else:
    #left = 50 + shift_amount * calcShiftFrac(t)
    left = calcLeft(t,left_swing)
    val = left - 100
  val += step_side_size * calcOffsetRightFrac(t,left_swing)
  return val

def calcSeparationOffset(t,left_swing):
  left = calcLeft(t,left_swing)
  right = calcRight(t,left_swing)
  return left - right - 100

step_side_size = -20
shift_amount = 20
fraction_still = 0.225
fraction_moving = 0.55

numVals = 1000
ts = [x / float(numVals) - 0.5 for x in range(numVals)]

left = []
right = []
xs = []
offset = []
vertLines = []

left_swing = True
for i in range(5):
  left.extend([calcLeft(t,left_swing) for t in ts])
  right.extend([calcRight(t,left_swing) for t in ts])
  offset.extend([calcSeparationOffset(t,left_swing) for t in ts])
  xs.extend([t + i for t in ts])
  left_swing = not(left_swing)

  lines = [-0.5,-0.5 + fraction_still,-0.5+fraction_still + fraction_moving,0.5]
  vertLines.extend([x + i for x in lines])

interestPts = [(-0.5,True),(0,True),(0.5,True),(-0.5,False),(0,False),(0.5,False)]
for t,left_swing in interestPts:
  #print '--',t,left_swing,'--'
  print '%2.2f %2.2f' % (calcLeft(t,left_swing),calcRight(t,left_swing))

plt.plot(xs,left)
plt.plot(xs,right)
plt.plot(xs,offset)
for x in vertLines:
  plt.vlines(x,-100,100)
plt.show()
