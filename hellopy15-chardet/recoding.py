#!/usr/bin/env python

import cchardet
import sys,os

def convert_encoding(data, new_coding = 'UTF-8'):
  encoding = cchardet.detect(data)['encoding']
  print encoding.lower(),'=>',new_coding.lower(),
  if new_coding.upper() != encoding.upper():
    data = data.decode(encoding, data).encode(new_coding)
  return data

def main():
  if len(sys.argv)<2:
    print os.path.basename(sys.argv[0]).upper(),
    print 'converts file encoding to utf-8'
    print 'basic usage:\n\t',sys.argv[0],'file.txt'
    return 
  with open(sys.argv[1],'rt') as fp:
    print 'recoding',sys.argv[1],'...',
    newdata = convert_encoding(fp.read())
    fp.close()
    with open(sys.argv[1],'wt') as fp:
      fp.write(newdata)
      fp.close()
      print 'done!'

if __name__=='__main__':
  main()
