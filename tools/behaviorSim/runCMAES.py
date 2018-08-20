#!/usr/bin/env python
# encoding: utf-8

import json, sys, os, shutil, subprocess, re, time
from cma import CMAEvolutionStrategy, Options

NUM_GENS = 150
POP_SIZE = 50
SIGMA0   = 100
CONDOR_SLEEP_TIME = 10
EVALS_PER_IND = 20
MIN_EVAL_FRAC_SUCCEEDED = 0.7
STOP_EVAL_WHEN_COMPLETED_FRAC = 0.8

def flattenParams(conf,paramNames,paramVals,prefix):
  if prefix != '':
    prefix += '_'
  for k,v in conf.iteritems():
    if isinstance(v,dict):
      flattenParams(v,paramNames,paramVals,prefix + k)
    else:
      paramNames.append(prefix + k)
      paramVals.append(v)

def insertParam(keys,v,dest):
  if len(keys) == 1:
    dest[keys[0]] = v
  else:
    if keys[0] not in dest:
      dest[keys[0]] = {}
    insertParam(keys[1:],v,dest[keys[0]])

def expandParams(paramNames,paramVals,dest):
  for flatKeys,v in zip(paramNames,paramVals):
    keys = flatKeys.split('_')
    insertParam(keys,v,dest)

def paramsToJsonFile(filename,paramNames,x):
  conf = {}
  expandParams(paramNames,x,conf)
  json.dump(conf,open(filename,'w'),sort_keys=True,indent=2)

def setupCondor(thisGenDir,thisGenOutputDir,condorConfig):
  thisGenCondor = os.path.join(thisGenDir,'job.condor')
  conf = condorConfig
  conf = conf.replace('$(ARGUMENTS)','%s $(PROCESS) %s' % (os.path.abspath(thisGenDir),EVALS_PER_IND))
  conf = conf.replace('$(BASE_DIR)',os.path.abspath(thisGenOutputDir))
  conf = conf.replace('$(NUM_JOBS)',str(POP_SIZE * EVALS_PER_IND))
  with open(thisGenCondor,'w') as f:
    f.write(conf)
  return thisGenCondor

def readCondorResults(thisGenOutputDir,xs):
  resXs = []
  resYs = []
  for i,x in enumerate(xs):
    blue = []
    red = []
    for j in range(EVALS_PER_IND):
      try:
        with open(os.path.join(thisGenOutputDir,'%i.out'%(i * EVALS_PER_IND + j)),'r') as f:
          contents = f.read()
        res = re.search('Score:\s*([0-9]+)\s*([0-9]+)',contents)
        b = int(res.group(1))
        r = int(res.group(2))
        blue.append(b)
        red.append(r)
      except:
        pass
    if len(blue) > EVALS_PER_IND * MIN_EVAL_FRAC_SUCCEEDED:
      #print 'blue',blue,'red',red
      resXs.append(x)
      y = (sum(blue) - sum(red)) / float(len(blue)) # minimizing, so higher red score is better
      resYs.append(y)
  return resXs,resYs

def runCondor(condorName):
  p = subprocess.Popen(['condor_submit',condorName],stdout=subprocess.PIPE)
  out = p.communicate()[0]
  res = re.search('cluster\s* ([0-9]+)',out)
  assert(res!=None)
  jobNum = res.group(1)
  numRunningJobs = None
  while True:
    p = subprocess.Popen(['condor_q',jobNum],stdout=subprocess.PIPE)
    out = p.communicate()[0]
    runningJobs = re.findall('^%s.([0-9]+)' % jobNum,out,re.MULTILINE)
    if len(runningJobs) != numRunningJobs:
      numRunningJobs = len(runningJobs)
      print 'num runningJobs',numRunningJobs,time.strftime("%Y-%m-%d %H:%M:%S")
    if len(runningJobs) < (1-STOP_EVAL_WHEN_COMPLETED_FRAC) * POP_SIZE * EVALS_PER_IND:
      break
    time.sleep(CONDOR_SLEEP_TIME)
  subprocess.call(['condor_rm',jobNum])

def runEvals(paramNames,xs,gen,thisGenDir,condorConfig):
  thisGenOutputDir = os.path.join(thisGenDir,'output')
  optMkdir(thisGenOutputDir)
  condorName = setupCondor(thisGenDir,thisGenOutputDir,condorConfig)
  for i,x in enumerate(xs):
    paramsToJsonFile(os.path.join(thisGenDir,'%i.json' % i),paramNames,x)
  #print 'origXs:',xs
  runCondor(condorName)
  xs,ys = readCondorResults(thisGenOutputDir,xs)
  #print 'xs:',xs
  #print 'ys:',ys
  #sys.exit(1)
  return xs,ys

def optMkdir(dirName):
  if not(os.path.exists(dirName)):
    os.mkdir(dirName)

def main(filename,directory):
  initialFilename = os.path.join(directory,'initial.json')
  genDir = os.path.join(directory,'gens')
  finalFilename = os.path.join(directory,'final.json')
  baseCondor = 'base.condor'

  with open(baseCondor,'r') as f:
    condorConfig = f.read()

  optMkdir(directory)
  optMkdir(genDir)
  shutil.copyfile(filename,initialFilename)
  with open(filename,'r') as f:
    conf = json.load(f)
  paramNames = []
  paramVals = []
  flattenParams(conf,paramNames,paramVals,'')
  sigma0 = SIGMA0
  opts = Options()
  opts['popsize'] = POP_SIZE
  #opts.printme()
  cma = CMAEvolutionStrategy(paramVals,sigma0,opts)
  while (cma.countiter < NUM_GENS) and not(cma.stop()):
    thisGenDir = os.path.join(genDir,str(cma.countiter))
    optMkdir(thisGenDir)
    xs = cma.ask()
    xs,fits = runEvals(paramNames,xs,cma.countiter,thisGenDir,condorConfig)
    cma.tell(xs,fits)
  res = cma.result()
  paramsToJsonFile(finalFilename,paramNames,res[0])

if __name__ == '__main__':
  args = sys.argv[1:]
  if len(args) < 2:
    print >>sys.stderr,'Usage: runCMAES.py baseConfFilename directory'
    sys.exit(2)
  main(args[0],args[1])
