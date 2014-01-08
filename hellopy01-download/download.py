#!/usr/bin/env python

import urllib2
import sys
import os
import time
import threading
import multiprocessing
import pprint

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
          sys.stderr.write('\rfile %s: %d bytes' % (self.fname,dlsize))
        else:
          self.dlspeed=(dlsize-self.dlsize)/1000./(now-self.now)
          sys.stderr.write('\rfile %s: %d bytes [%.1f KB/s]' % (self.fname,dlsize,self.dlspeed))
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
    sys.stderr.write('\rfile %s: %d bytes [%.1f KB/s]' % (fname,dlsize,t.dlspeed))
  else:
    sys.stderr.write('\rfile %s: %d bytes' % (fname,dlsize))
  sys.stderr.write('\nfinish downloading %s!\n' % (fname))

def download_multi(x):
  download(x[0],x[1])

# print help
def print_help():
  # http://www.google.com.hk/images/srpr/logo11w.png
  sys.stderr.write('Usage '+sys.argv[0].split('/')[-1]+
                   ' link [-h|help] [-t|target arg] [-n(um) arg]\n')
  sys.stderr.write('\n')
  sys.stderr.write('Options and arguments:\n'+
                   ' -h     : display this help information\n'+
                   ' -t arg : assign target file name\n'+
                   ' -n arg : assign number of files to download with given template\n')
  sys.stderr.write('\n')
  sys.stderr.write('An example of downloading multiple files:\n'+
                   ' $ download.py http://www.google.com.hk/images/srpr/logo{0:d}w.png -n 11\n')
  sys.stderr.write('\n')
  sys.stderr.write('For more information, contact <liangfu.chen@nlpr.ia.ac.cn>\n')


# main entry
def main():
  # in case of no argument given
  if len(sys.argv)==1:
    print_help()
    exit(0)

  # initial values
  dllink=''
  dltarget=''
  dlnum=1

  # parse command line input
  i=1;
  while i<len(sys.argv):
    if sys.argv[i]=='-h' or sys.argv[i]=='-help':
      print_help()
      exit(0)
    elif sys.argv[i]=='-t' or sys.argv[i]=='-target':
      dltarget=sys.argv[i+1]
      i+=1
    elif sys.argv[i]=='-n' or sys.argv[i]=='-num':
      dlnum=sys.argv[i+1]
      i+=1
    elif sys.argv[i][0:4]=='http':
      dllink=sys.argv[i]
    else:
      raise ValueError('invalid input arguments\n')
      print_help()
    i+=1

  if dlnum==1:
    # download a single file
    if len(dltarget)==0:
      download(dllink, dllink.split('/')[-1])
    else:
      download(dllink,dltarget)
  elif dlnum>1:
    # download a list of files
    
    # sequential
    # for i in range(1,int(dlnum)+1):
    #   a=dllink % i
    #   download(a,a.split('/')[-1])
    
    # parallel
    p=multiprocessing.Pool(int(dlnum))
    dllist=map(lambda i:(dllink.format(i),(dllink.format(i)).split('/')[-1]),range(1,int(dlnum)+1))
    pprint.pprint(dllist)
    p.map(download_multi,dllist)
  else:
    raise ValueError('invalid input argument in `-num`\n')
    print_help()

if __name__=='__main__':
  main()
