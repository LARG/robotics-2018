#!/usr/bin/env python

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os

import re
import shutil
import sys
import tempfile
from subprocess import check_call
import pexpect


def verifyPassword(password):
    p = pexpect.spawn('sudo echo test')
    p.expect(['\[sudo\] password.*'])
    p.sendline(password)
    i = p.expect(['test', 'Sorry.*'])
    return i == 0


def call(command, shell=True):
    check_call(command, shell=shell)


def runCommand(command, password=None, output=True, exit_on_error=False):
    if output is None:
        output = True
    p = pexpect.spawn(command, timeout=7200)
    outstr = ""
    while True:
        i = p.expect(['Password:', r'\[sudo\] password.*',
                      r'Are you sure.*\(yes/no\)? *',
                      r'.*\[Y\/n\]', '.+\n', '.+', pexpect.EOF])
        if i == 6:
            break
        outstr = p.before + p.after
        if i == 0 or i == 1:
            if password is None:
                p.close()
                raise Exception("No password supplied.")
            p.sendline(password)
        if i == 2:
            p.sendline("yes")
        elif i == 3:
            p.sendline("Y")
        elif i == 4:
            outstr = outstr.strip()
            outstr = re.sub(r'\s*Connection to (\d+\.){3}\d+ closed\.\s*',
                            '', outstr)
            outstr = re.sub(r'\s*Password:\s*', '', outstr)
            if output:
                print(outstr)
        elif i == 5:
            outstr = outstr.strip()
            if output:
                sys.stdout.write('\r' + outstr)
                sys.stdout.flush()
    if exit_on_error and p.exitstatus:
        print('Error completing command: %s' % command)
        sys.exit(p.exitstatus)
    p.close()
    return outstr


def runLocalCommand(command, password=None, output=None, exit_on_error=False):
    command = command.replace("'", "'\"'\"'")
    return runCommand(command, password, output=output,
                      exit_on_error=exit_on_error)


def runRemoteCommand(ip, command, root=False, output=None, exit_on_error=False):
    command = command.replace("'", "'\"'\"'")
    if root:
        command = "su -c '%s'" % command
        command = command.replace("'", "'\"'\"'")
        command = "ssh -t nao@%s '%s'" % (ip, command)
        return runCommand(command, password='root', output=output,
                          exit_on_error=exit_on_error)
    else:
        command = "ssh nao@%s '%s'" % (ip, command)
        return runCommand(command, output=output)


def copyDirectory(ip, path, dest, root=False):
    if root:
        base = os.path.basename(path)
        directory = "/tmp"
        call('scp -r %s nao@%s:%s' % (path, ip, directory))
        runRemoteCommand(ip, "rm -rf %s && mv %s %s" % (dest, directory + "/" + base, dest), root=True)
    else:
        call('scp -r %s nao@%s:%s' % (path, ip, dest))


def copyFiles(ip, files, dest='', root=False):
    if root:
        directory = "/tmp"
        base = os.path.basename(files)
        call('scp -r %s nao@%s:%s' % (files, ip, directory))
        runRemoteCommand(ip, "mv %s %s" % (directory + "/" + base, dest),
                         root=True)
    else:
        call('scp -r %s nao@%s:%s' % (files, ip, dest))


copyFile = copyFiles


def runLocalScriptRemotely(ip, scriptpath, output=None):
    copyFiles(ip, scriptpath)
    scriptname = os.path.basename(scriptpath)
    runRemoteCommand(ip, 'bash %s && rm %s' % (scriptname, scriptname),
                     root=True, output=output)


def ask(prompt):
    while True:
        response = raw_input('%s (y/n): ' % prompt)
        if response.lower() in ['y', 'yes']:
            return True
        elif response.lower() in ['n', 'no']:
            return False
        print('Invalid response, try again')


def removeTempDir(d):
    shutil.rmtree(d)


def makeTempDir():
    d = tempfile.mkdtemp(dir=os.path.expandvars('$NAO_HOME/install'))
    return d


def readVars():
    if 'NAO_HOME' not in os.environ:
        print('NAO_HOME not set, exiting')
        sys.exit(1)
    with open('%s/install/vars' % os.environ['NAO_HOME'], 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('export'):
                m = re.search(r'export (\w+)=(.*)$', line)
                if m:
                    key, value = m.group(1), m.group(2)
                    os.environ[key] = value
