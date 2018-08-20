#!/usr/bin/env python
"""Utils for behavior logging."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import core
# import memory
import logging
logging.disable(logging.ERROR)  # kill hashlib errors from random
from timeit import default_timer as timer

TRACE = True
TIME = False


def log(loglevel, *args):
    message = ""
    for arg in args:
        message += str(arg) + " "
    if loglevel == 0:
        print(message)
    core.pythonC.log(loglevel, message)


def taskTrace(msg, task):
    if not TRACE: return
    indent = ""
    for i in range(task.getDepth()):
        indent += "  "

    ts = str(task)
    message = "%s: %s" % (ts, msg)
    console = indent + message
    if TIME and not core.TOOL and msg == "Finishing" and ts in timers:
        tm = timers[ts]
        t = tm.getavg()
        console += " (%2.2f ms)" % t
    print(console)
    log(40, message)

timers = {}


def stimer(name):
    if not TIME: return
    if name not in timers:
        timers[name] = Timer(name)
    timers[name].start()


def etimer(name):
    if not TIME: return
    if name not in timers:
        timers[name] = Timer(name)
    t = timers[name]
    t.stop()
    t.printavg()


def ttimer(name):
    if not TIME: return
    if name not in timers:
        timers[name] = Timer(name)
    t = timers[name]
    t.stop()


class Timer(object):
    def __init__(self, name, interval=10):
        self.name = name
        self.interval = interval
        self.reset()

    def start(self):
        self._start = timer()

    def stop(self):
        e = timer() - self._start
        self._elapsed += e
        self.times.append(e)

    def reset(self):
        self._elapsed = 0.0
        self.times = []
        self.start()

    def elapsed(self):
        return timer() - self._start + self._elapsed

    def getavg(self):
        if len(self.times) == 0: return 0
        return sum(self.times) / len(self.times) * 1000

    def printavg(self, override=False):
        if core.TOOL: return
        fmt = self.name + ": %2.2f ms"
        if len(self.times) == self.interval or override:
            if len(self.times) == 0: print(fmt % 0.0)
            else:
                total = sum(self.times)
                avg = total / len(self.times) * 1000
                print(fmt % avg)
            self.times = []


def profile(function, sortby='cumulative', iterations=1):
    import cProfile
    import pstats
    import StringIO
    pr = cProfile.Profile()
    pr.enable()
    for i in range(iterations):
        function()
    pr.disable()
    s = StringIO.StringIO()
    ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
    ps.print_stats()
    print(s.getvalue())


import os
_proc_status = '/proc/%d/status' % os.getpid()

_scale = {'kB': 1024.0, 'mB': 1024.0 * 1024.0,
          'KB': 1024.0, 'MB': 1024.0 * 1024.0}


def _VmB(VmKey):
    """Private."""
    global _proc_status, _scale
    # get pseudo file  /proc/<pid>/status
    try:
        t = open(_proc_status)
        v = t.read()
        t.close()
    except:
        return 0.0  # non-Linux?
    # get VmKey line e.g. 'VmRSS:  9999  kB\n ...'
    i = v.index(VmKey)
    v = v[i:].split(None, 3)  # whitespace
    if len(v) < 3:
        return 0.0  # invalid format?
    # convert Vm value to bytes
    return float(v[1]) * _scale[v[2]]


def total_memory(since=0.0):
    """Return total memory usage in bytes."""
    return _VmB('VmSize:') - since


def resident(since=0.0):
    """Return resident memory usage in bytes."""
    return _VmB('VmRSS:') - since


def stacksize(since=0.0):
    """Return stack size in bytes."""
    return _VmB('VmStk:') - since
