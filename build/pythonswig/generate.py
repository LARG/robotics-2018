#!/usr/bin/env python
import sys, os, subprocess, re, shutil

class ConsoleColors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

VERBOSE = False
def console(message):
  if not VERBOSE: return
  print message

typedefs = ["typedef %s %s;" % (x,y) for x,y in [
  ("int", "int32_t"), ("unsigned int", "uint32_t"), 
  ("short", "int16_t"), ("unsigned short", "uint16_t"),
  ("unsigned char", "uint8_t")
]]
copy_constructors = ['KickStrategy', 'RoleStrategy']

def generateTemplates(includes, tfile):
  content = "%module templates\n"
  content += '%include "std_string.i"\n'
  content += '%include "std_vector.i"\n'
  content += '%include "std_array.i"\n'
  content += "%{\n"
  content += " ".join(typedefs) + "\n"
  for include in includes:
    content += '#include "%s"\n' % include
  content += "%}\n\n"
  content += " ".join(typedefs) + "\n"
  for include in includes:
    content += '%%include "%s"\n' % include
  content += '%template(vector2_float) Vector2<float>;\n'
  content += '%template(vector3_float) Vector3<float>;\n'
  content += '%template(vector_float) std::vector<float>;\n'
  with open(tfile, 'w') as f:
    f.write(content)

def generateCPP(ifile, cppfile):
  command = 'swig3.0 -w312,451,325,509,362,389,401,454,462,503 -python -c++ -o %s %s' % (cppfile,ifile)
  subprocess.check_call(command,shell=True)

def generateI(header, ifile):
  module_name = os.path.splitext(os.path.basename(header))[0]
  content = "%%module %s\n" % module_name
  content += '%include "std_string.i"\n'
  content += '%include "std_vector.i"\n'
  resources = ["Enum", "RobotInfo", "Pose3D"]
  if module_name not in resources:
    for r in resources:
      content += '%%include "%s.i"\n' % r
  for cc in copy_constructors:
    content += '%%copyctor %s;\n' % cc
  content += "%{\n"
  content += '#include "%s"\n' % header
  content += "%}\n\n"
  content += '%%include "%s"\n' % header
  with open(ifile, 'w') as f:
    f.write(content)

def generateImporter(modules, importer):
  content = "%module pythonswig_module\n"
  for m in modules:
    module_name = os.path.splitext(os.path.basename(m))[0]
    content += '%%include "%s.i"\n' % module_name
  with open(importer, 'w') as f:
    f.write(content)

def movePy(cppfile, pyfile):
  module_name = os.path.splitext(os.path.basename(cppfile))[0]
  module_dir = os.path.dirname(cppfile)
  module = os.path.join(module_dir, module_name + ".py")
  print "Moving '%s' to '%s'" % (module, pyfile)
  shutil.move(module, pyfile)

def clean(cppfile):
  outfile = cppfile.replace("_base","")
  with open(cppfile, 'r') as cppin:
    with open(outfile, 'w') as cppout:
      for line in cppin:
        if line.startswith("#include"):
          line = re.sub('#include "[\w/]+/include/(\w+Block\.h)"', "#include <memory/\g<1>>", line)
        cppout.write(line)

import argparse
def parse_args():
  parser = argparse.ArgumentParser(description='SWIG Generator')
  parser.add_argument('--action', dest='action', action='store', type=str,
                     help='The action to be performed', default=None)
  parser.add_argument('--header', dest='header', action='store', type=str,
                     help='The path to the generated output file', default=None)
  parser.add_argument('--ifile', dest='ifile', action='store', type=str,
                     help='The path to the generated I file', default=None)
  parser.add_argument('--cppfile', dest='cppfile', action='store', type=str,
                     help='The path to the generated CPP file', default=None)
  parser.add_argument('--pyfile', dest='pyfile', action='store', type=str,
                     help='The path to the generated Py file', default=None)
  parser.add_argument('--tfile', dest='tfile', action='store', type=str,
                     help='The path to the generated template file', default=None)
  parser.add_argument('--importer', dest='importer', action='store', type=str,
                     help='The path to the generated importer', default=None)
  parser.add_argument('--module', dest='modules', action='append', type=str,
                     help='Path to a single module to be imported into a composite', default=None)
  parser.add_argument('--include', dest='includes', action='append', type=str,
                     help='Path to a single header to support template instantiations', default=None)
  parser.add_argument('--verbose', dest='verbose', action='store_true',
                     help='Output debugging information to the console', default=False)
  args, unk = parser.parse_known_args()
  global VERBOSE
  VERBOSE = args.verbose
  return args

def main(args):
  console("starting SWIG generator with arguments:")
  console(args)
  if args.action not in ['i', 'cpp', 'templates', 'importer', 'movepy', 'clean']:
    print "Invalid action selected: %s" % args.action
    sys.exit(1)
  if args.action == "i":
    if args.header == None or args.ifile == None:
      sys.exit(1)
    generateI(args.header, args.ifile)
  elif args.action == "cpp":
    if args.cppfile == None or args.ifile == None:
      sys.exit(1)
    generateCPP(args.ifile, args.cppfile)
  elif args.action == "templates":
    if args.tfile == None:
      sys.exit(1)
    generateTemplates(args.includes, args.tfile)
  elif args.action == "importer":
    if len(args.modules) == 0 or args.importer == None:
      sys.exit(1)
    generateImporter(args.modules, args.importer)
  elif args.action == "movepy":
    if args.cppfile == None or args.pyfile == None:
      sys.exit(1)
    movePy(args.cppfile, args.pyfile)
  elif args.action == "clean":
    if args.cppfile == None:
      sys.exit(1)
    clean(args.cppfile)

if __name__ == '__main__':
  args = parse_args()
  try:
    main(args)
  except:
    print "Invalid arguments"
    print args
    raise
