#!/usr/bin/env python

import subprocess, re, time

class WirelessTester(object):
  """docstring for WirelessTester"""
  def __init__(self):
    super(WirelessTester, self).__init__()
    self.lastRx, self.lastTx = self.getNumBytes()

  def getNumBytes(self):
    cmd = 'ifconfig wlan0'
    p = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE)
    out,err = p.communicate()
    try:
      rxBytes = re.findall('RX bytes:(\d+)',out)[0]
    except:
      rxBytes = 0
    try:
      txBytes = re.findall('TX bytes:(\d+)',out)[0]
    except:
      txBytes = 0
    return rxBytes,txBytes

  def test(self):
    rx,tx = self.getNumBytes()
    if (rx != self.lastRx) and (tx != self.lastTx):
      res = True
    else:
      res = False
    self.lastRx = rx
    self.lastTx = tx
    return res
    
#def isPingWorking():
  #cmd = 'ping -c 1 -W 1 192.168.1.1'
  #p = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE)
  #out,err = p.communicate()
  #res = re.findall('(\d+)% packet loss',out)
  #if res is None:
    #return False
  #else:
    #return res[0] == '0'

def restartWireless():
  cmd = 'su -c "/etc/init.d/utwireless restart"'
  try:
    subprocess.call(cmd,shell=True)
  except:
    print 'restart failed'

def main():
  failCount = 0
  tester = WirelessTester()
  while True:
    #if isPingWorking():
    if tester.test():
      #print 'wireless is working'
      failCount = 0
      time.sleep(5)
    else:
      failCount += 1
      #print 'wireless test failed:',failCount
      time.sleep(1)
    if failCount >= 5:
      print 'Restarting wireless'
      restartWireless()
      failCount = 0
      time.sleep(10)

if __name__ == '__main__':
  main()
