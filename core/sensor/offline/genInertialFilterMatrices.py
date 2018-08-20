#!/usr/bin/env python

import math
import scipy.io
import subprocess, sys

def writeMatrix(name,res,indentation=2):
  mat = res[name]
  output = indentation * '  ' + '%s = NMatrix(%i,%i,false);\n' % (name,mat.shape[0],mat.shape[1])
  for i in range(mat.shape[0]):
    output += indentation * '  '
    for j in range(mat.shape[1]):
      output += '%s[%i][%i]=% e; ' % (name,i,j,mat[i,j])
    output += '\n'
  output += '\n'
  return output

DEG_T_RAD = math.pi / 180

params = {
  'iu_height': 0.30, # measured on the robot in m
  'gyro_noise_rms':  1 * DEG_T_RAD, # rad/s # higher is laggier and smoother # lower responds faster, noisier
  'acc_noise_rms':  0.33, # m / s^2 # higher just scales more, lower oscillates a lot!
  'motion_rms':  3, # rad/s^3 # higher increases overshoot (slightly) # lower is laggier and smoother
  'gyro_drift_rms':  0.0001, # rad/sample # higher is smoother
}
#params = {
  #'iu_height': 0.30, # measured on the robot in m
  #'gyro_noise_rms':  3 * DEG_T_RAD, # rad/s
  #'acc_noise_rms':  1, # m / s^2
  #'motion_rms':  1, # rad/s^3
  #'gyro_drift_rms':  0.0001, # rad/sample
#}

argList = [[False,0.01],[True,0.02]] # args in format (in_simulation,dt)
matrixNames = ['A','B','C','Cz','Cz1','Q','R','L']

template = '''
#include "InertialFilter.h"

/************************************************
 * GENERATED via %s *
 ************************************************/

void InertialFilter::initMatrices(bool in_simulation) {
%s
}

void InertialFilter::updateIUHeight(float torsoZ) {
  // torso (which is actually between hip) + 85mm (from specs) - gyro offset
  float iu_height = (torsoZ + 85) * 0.001 - 0.029; // convert to m
  C[0][2] = iu_height;
  //std::cout << "iu_height = " << iu_height << std::endl;
}

'''

body = ''

for inSimulation,dt in argList:
  currentParams = dict(params)
  currentParams['dt'] = dt
  for k,v in currentParams.iteritems():
    currentParams[k] = float(v)
  scipy.io.savemat('params.mat',currentParams,oned_as='column')
  sys.stdout.flush()
  subprocess.check_call(['octave','FindGain.m'],stdout=subprocess.PIPE)
  
  res = scipy.io.loadmat('results.mat')
  if inSimulation:
    body += '  if (in_simulation) {\n'
  else:
    body += '  if (!in_simulation) {\n'
  for name in matrixNames:
    body += writeMatrix(name,res)
  body += '  }\n'

contents = template % (sys.argv[0],body)

with open('../initInertialFilter.cpp','w') as f:
  f.write(contents)
