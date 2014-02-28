#!/usr/bin/env python

import os,sys

def obj_quad2tri(src,dst):
  """
  convert quad face mesh to regular 3 face formed mesh
  """
  with open(src,'rt') as fp:
    with open(dst,'wt') as fout:
      for line in fp.read().split('\n'):
        if line[0:2]=='f ':
          faces=map(lambda x:str(x),line[2:].split(' '))
          if len(faces)==4:
            fout.write('f '+' '.join(faces[0:3])+'\n')
            fout.write('f '+' '.join([faces[2],faces[3],faces[0]])+'\n')
          elif len(faces)==3:
            fout.write(line+'\n')
          else:
            raise IOError
        else:
          fout.write(line+'\n')

if __name__=='__main__':
  obj_quad2tri(sys.argv[1],sys.argv[2])
