#!/usr/local/bin/python
# -*- coding: utf-8 -*-
'''
DLLs from OpenMIDI project http://openmidiproject.sourceforge.jp/
C:\prj\tools\python\MIDI

import ctypes
import time

mididatalib = ctypes.windll.LoadLibrary(r".\MIDIData.dll")
# mididata = mididatalib.MIDIData_Create(2, 1, 24, 120)
mididata = mididatalib.MIDIData_Create(0, 1, 0, 120)
if not mididata:
  print 'error'
miditrack = mididatalib.MIDIData_GetFirstTrack(mididata)
mididatalib.MIDITrack_InsertTrackName(miditrack, 0, 'Kimigayo')
mididatalib.MIDITrack_InsertTempo(miditrack, 0, 2 * 60000000 / 120)
mididatalib.MIDITrack_InsertProgramChange(miditrack, 0, 0, 110)
mididatalib.MIDITrack_InsertProgramChange(miditrack, 0, 1, 111)
mididatalib.MIDITrack_InsertNote(miditrack,   0, 0, 62, 80, 120)
mididatalib.MIDITrack_InsertNote(miditrack,   0, 1, 73, 40, 120)
mididatalib.MIDITrack_InsertNote(miditrack, 120, 0, 60, 80, 120)
mididatalib.MIDITrack_InsertNote(miditrack, 120, 1, 71, 40, 120)
mididatalib.MIDITrack_InsertNote(miditrack, 240, 0, 62, 80, 120)
mididatalib.MIDITrack_InsertNote(miditrack, 240, 1, 73, 40, 120)
mididatalib.MIDITrack_InsertNote(miditrack, 360, 0, 64, 80, 120)
mididatalib.MIDITrack_InsertNote(miditrack, 360, 1, 75, 40, 120)
mididatalib.MIDITrack_InsertNote(miditrack, 480, 0, 67, 80, 120)
mididatalib.MIDITrack_InsertNote(miditrack, 480, 1, 78, 40, 120)
mididatalib.MIDITrack_InsertNote(miditrack, 600, 0, 64, 80, 120)
mididatalib.MIDITrack_InsertNote(miditrack, 600, 1, 75, 40, 120)
mididatalib.MIDITrack_InsertNote(miditrack, 720, 0, 62, 80, 240)
mididatalib.MIDITrack_InsertNote(miditrack, 720, 1, 73, 40, 240)
mididatalib.MIDITrack_InsertEndofTrack(miditrack, 960)
mididatalib.MIDIData_SaveAsSMF(mididata, "smftest0.mid")
mididatalib.MIDIData_Delete(mididata)
'''

import operator

def val2vlv(val):
  vlv = 0
  for i in range(3, -1, -1):
    v = (val >> (i * 7)) & 0x7F
    vlv |= (v | (0x80 if (v or vlv) and i else 0)) << (i * 8)
  return vlv

def vlv2val(vlv):
  return reduce(operator.ior,
    (((vlv & (0x7F << (i * 8))) >> i) for i in range(4)), 0)

if __name__ == '__main__':
  print '%08X, %08X' % (val2vlv(0x00000000), vlv2val(0x00000000))
  print '%08X, %08X' % (val2vlv(0x00000040), vlv2val(0x00000040))
  print '%08X, %08X' % (val2vlv(0x0000007F), vlv2val(0x0000007F))
  print '%08X, %08X' % (val2vlv(0x00000080), vlv2val(0x00008100))
  print '%08X, %08X' % (val2vlv(0x00002000), vlv2val(0x0000C000))
  print '%08X, %08X' % (val2vlv(0x00003FFF), vlv2val(0x0000FF7F))
  print '%08X, %08X' % (val2vlv(0x00004000), vlv2val(0x00818000))
  print '%08X, %08X' % (val2vlv(0x00100000), vlv2val(0x00C08000))
  print '%08X, %08X' % (val2vlv(0x001FFFFF), vlv2val(0x00FFFF7F))
  print '%08X, %08X' % (val2vlv(0x00200000), vlv2val(0x81808000))
  print '%08X, %08X' % (val2vlv(0x08000000), vlv2val(0xC0808000))
  print '%08X, %08X' % (val2vlv(0x0FFFFFFF), vlv2val(0xFFFFFF7F))

  print '...'
  '''
  for i in xrange(0x10000000):
    if vlv2val(val2vlv(i)) != i: print 'error A %08X' % i
  '''
  c = 0
  m = 0x00000080
  for p in xrange(m):
    for q in xrange(m):
      for r in xrange(m):
        for s in xrange(m):
          vp = p | (m if p else 0)
          vq = q | (m if q else (m if p else 0))
          vr = r | (m if r else (m if p or q else 0))
          v = (vp << 24) | (vq << 16) | (vr << 8) | s
          a = vlv2val(v)
          if a != c: print 'error B1 %08X, %08X' % (v, a)
          l = val2vlv(a)
          if l != v: print 'error B2 %08X, %08X, %08X' % (v, a, l)
          c += 1
  if c != 0x10000000: print 'error C %08X' % c
  print 'done'
