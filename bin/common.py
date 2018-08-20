#!/usr/bin/env python

import subprocess, os

def runBin(part1,part2,args=None,options=None):
  if options is None:
    options,args = parseArgs(args)
  
  path = os.getenv('NAO_HOME') + '/build2/build/build-%s%s/%s/sdk/bin/%s'
  #arch = 'atom' if options.atom else 'geode'
  arch = 'linux64'
  release = '' if options.debug else '-release'

  cmd = os.path.normpath(path % (arch,release,part1,part2))
  runCommand(cmd,options,args)

def runCommand(cmd,options=None,args=None,ldAdditional='',gtkPath=None):
  if options is None:
    options,args = parseArgs(args)

  fullCmd = [cmd] + args
  if options.gdb:
    fullCmd = ['gdb','-ex','run','--args'] + fullCmd
  elif options.valgrind:
    fullCmd = ['valgrind'] + fullCmd
    print fullCmd

  #arch = 'atom' if options.atom else 'geode'
  #arch = 'linux64'
  #ldpath = os.getenv('NAO_HOME') + ('/naoqi/crosstoolchain/%s/sysroot/usr/lib/' % arch) + ldAdditional
  ldpath = ldAdditional

  os.putenv('LD_LIBRARY_PATH',os.getenv('LD_LIBRARY_PATH','') + ':' + ldpath)
  if gtkPath is not None:
    os.putenv('GTK_PATH',gtkPath)
  p = subprocess.Popen(fullCmd)
  try:
    p.wait()
  finally:
    if p.poll() == None:
      p.terminate()

def parseArgs(args=None):
  from optparse import OptionParser
  import sys
  usage = 'usage: %prog [options] [teamNum [playerNum]]'
  parser = OptionParser(usage)
  parser.add_option('--geode',action='store_false',dest='atom',help='run the geode binaries',default=True)
  parser.add_option('--atom',action='store_true',dest='atom',help='run the atom binaries',default=True)
  parser.add_option('--gdb',action='store_true',dest='gdb',help='run gdb on the binaries (you may also want to use --debug to use the debug binaries',default=False)
  parser.add_option('--valgrind',action='store_true',dest='valgrind',help='run valgrind on the binaries (you may also want to use --debug to use the debug binaries',default=False)
  parser.add_option('--debug',action='store_true',dest='debug',help='use the debug binaries',default=False)
  if args is None:
    args = sys.argv[1:]
    otherArgs = []
    i = 0
    while i < len(args):
      if (args[i][0] == '-') and (args[i][1] != '-'):
        otherArgs.append(args[i])
        del args[i]
      else:
        i += 1
  options,args = parser.parse_args(args)
  args = otherArgs + args
  return options,args

def onLabMachine():
  from getpass import getuser
  return getuser() == 'sbarrett'
