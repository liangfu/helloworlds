#!/usr/bin/env python

import time
import sys
import threading

class StoppableThread(threading.Thread):
  """Thread class with a stop() method. The thread itself 
  has to check regularly for the stopped() condition."""

  def __init__(self):
    super(StoppableThread, self).__init__()
    self._stop = threading.Event()

  def stop(self):
    self._stop.set()

  def stopped(self):
    return self._stop.isSet()

  def run(self):
    if not self._stop.is_set():
      sys.stderr.write('x,')
      time.sleep(.1)
      self.run()

if __name__=='__main__':
  m=StoppableThread()
  m.start()
  time.sleep(2)
  m.stop()
