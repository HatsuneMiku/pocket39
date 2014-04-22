#!/usr/local/bin/python
# -*- coding: utf-8 -*-
'''pocket39
'''

import ctypes

def main():
  p39lib = ctypes.windll.LoadLibrary('p39lib.dll')
  if p39lib is None:
    print 'Load Error'
    return
  print 'Load OK'
  p39 = p39lib.p39open()
  if not p39:
    print 'cannot open'
    return
  print 'open OK'
  if p39lib.p39reset(p39):
    print 'cannot reset'
  else:
    print 'reset OK'
    # p39lib.p39programs(p39, banks, 16)
    r = p39lib.p39singW(p39, u'うちゃいます', u'E=GB=GE=R')
    if r:
      print 'good %d: you may use p39lib.dll (kaerunouta.xls pocket39.exe)' % r
  p39lib.p39close(p39)
  print 'close OK'

if __name__ == '__main__':
  main()
