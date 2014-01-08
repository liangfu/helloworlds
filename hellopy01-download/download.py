#!/usr/bin/env python

import urllib2
import sys
import os
import time
import threading

class StoppableThread(threading.Thread):
  """Thread class with a stop() method. The thread itself 
  has to check regularly for the stopped() condition."""
  def __init__(self,url,fname):
    super(StoppableThread, self).__init__()
    self._stop = threading.Event()
    self.url=url
    self.fname=fname
    self.now=0
    self.dlsize=0
    self.dlspeed=0
  def stop(self):
    self._stop.set()
  def stopped(self):
    return self._stop.isSet()
  def run(self):
    if not self._stop.is_set():
      if os.path.exists(self.fname):
        dlsize=os.stat(self.fname).st_size
        now=time.time()
        if not self.now:
          sys.stderr.write('\r%d bytes' % (dlsize))
        else:
          self.dlspeed=(dlsize-self.dlsize)/1000./(now-self.now)
          sys.stderr.write('\r%d bytes [%.1f KB/s]' % (dlsize,self.dlspeed))
        self.dlsize=dlsize
        self.now=now
        time.sleep(1)
        self.run()

# function available in shutil library
def copyfileobj(fsrc, fdst, length=16*1024):
  """copy data from file-like object fsrc to file-like object fdst"""
  while 1:
    buf = fsrc.read(length)
    if not buf:
      break
    fdst.write(buf)

def download(url,fname):
  req=urllib2.urlopen(url)
  sys.stderr.write('downloading %s ...\n' % fname)
  t=StoppableThread(url,fname)
  t.start()
  with open(fname,'wb') as fp:
    copyfileobj(req,fp)
  t.stop()
  dlsize=os.stat(fname).st_size
  if t.dlspeed!=0:
    sys.stderr.write('\r%d bytes [%.1f KB/s]' % (dlsize,t.dlspeed))
  else:
    sys.stderr.write('\r%d bytes' % (dlsize))
  sys.stderr.write('\ndone.\n')

# print help
def print_help():
  sys.stderr.write('usage example:\n\t'+sys.argv[0]+
                   ' http://www.google.com.hk/images/srpr/logo11w.png\n')

# main entry
def main():
  if len(sys.argv)==1:
    print_help()
  elif sys.argv[1][0:2]=='-h':
    print_help()
  elif len(sys.argv)==2:
    download(sys.argv[1], sys.argv[1].split('/')[-1])
  elif len(sys.argv)==3:
    download(sys.argv[1], sys.argv[2])
  else:
    raise ValueError('invalid input arguments\n')
    print_help()

if __name__=='__main__':
  main()
