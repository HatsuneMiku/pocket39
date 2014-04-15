#!/usr/local/bin/python
# -*- coding: utf-8 -*-
'''
DLLs from OpenMIDI project
C:\prj\tools\python\MIDI

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
mididatalib.MIDIData_SaveAsSMF(mididata, "test_midi_04.mid")
mididatalib.MIDIData_Delete(mididata)
'''

import ctypes
import time

midiiolib = ctypes.windll.LoadLibrary(r".\MIDIIO.dll")
c_deviceName = ctypes.create_string_buffer(32)
midiiolib.MIDIOut_GetDeviceName(1, c_deviceName, 32)
print c_deviceName.value
if c_deviceName.value == '':
  midiiolib.MIDIOut_GetDeviceName(0, c_deviceName, 32)
  print c_deviceName.value
midiout = midiiolib.MIDIOut_Open(c_deviceName.value)

# 121: 尺八, 122: さざ波, 123: 小鳥, 124: ベル, 125: ヘリ, 126: 歓声, 127: 銃
banks = [  0, 121, 122, 123, 124, 125, 126, 127,
           8,   0,  10,  11,  12,  13,  14, 15]

# [[channel, tone, velocity], [], ...]
notes = [[ 1, 60, 100],
         [ 2, 62, 100],
         [ 3, 64, 100],
         [ 4, 65, 100],
         [ 0, 67, 100],
         [ 9, 69, 100],
         [10, 71, 100],
         [ 0, 72, 100]]

c_mess = ctypes.create_string_buffer(3)
for i, bank in enumerate(banks):
  c_mess.value = '%c%c' % (chr(0xC0 + i), chr(bank))
  midiiolib.MIDIOut_PutMIDIMessage(midiout, c_mess.value, 2) # ProgramChange
for note in notes:
  c_mess.value = '%c%c%c' % (chr(0x90 + note[0]), chr(note[1]), chr(note[2]))
  midiiolib.MIDIOut_PutMIDIMessage(midiout, c_mess.value, 3) # Note On
  time.sleep(0.5)
time.sleep(1.0)
midiiolib.MIDIOut_Close(midiout)
