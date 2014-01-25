#!/usr/bin/env python

import ctypes
import os

class Test:
  def __init__(self):
    thispath=os.path.abspath(__file__)
    thispath=os.path.realpath(thispath)
    thispath=os.path.dirname(thispath)
    self.lib=ctypes.CDLL(os.path.join(thispath,'libtest.so'))
  def add(self,a,b):
    return self.lib.add(1,2)

if __name__=='__main__':
  app=Test()
  print app.add(1,2)

