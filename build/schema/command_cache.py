import sys, os
import hashlib, binascii
import errno

def mtime(path):
  return int(os.path.getmtime(path) * 1000000)

def hash(args):
  s = " ".join(args)
  return hashlib.md5(s).hexdigest()

def mkdir_p(path):
  try:
    os.makedirs(path)
  except OSError as exc:  # Python >2.5
    if exc.errno == errno.EEXIST and os.path.isdir(path):
      pass
    else:
      raise

class CommandCache:
  def __init__(self, directory, dependencies):
    self.h = hash(" ".join(sys.argv))
    self.path = os.path.join(directory, "command_cache", self.h)
    if os.path.exists(self.path):
      self.cache_time = mtime(self.path)
    else: self.cache_time = None
    self.dependencies = dependencies

  def cached_read(self, callback):
    if self.valid(): 
      return self.read()
    else:
      content = callback()
      self.write(content or '')
      return content

  def valid(self):
    if not self.cache_time: return False
    for d in self.dependencies:
      if mtime(d) > self.cache_time:
        return False
    return True

  def read(self):
    with open(self.path, 'r') as f:
      return f.read()
  
  def write(self, content):
    mkdir_p(os.path.dirname(self.path))
    with open(self.path, 'w') as f:
      f.write(content)
