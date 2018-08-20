import imp, os, sys
sys.path.append(os.path.expanduser("~/.vim/bundle/YouCompleteMe/third_party/ycmd"))
ycm_config = imp.load_source('ycm_config', os.path.expanduser("~/.vim/bundle/.ycm_extra_conf.py"))
from ycm_config import *

paths = [
  '$NAO_HOME/core',
  '$NAO_HOME/build/include',
  '$NAO_HOME/tools/trainers/include'
]

for p in paths:
  flags.append('-I')
  flags.append(os.path.expandvars(p))
