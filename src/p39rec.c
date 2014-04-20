/*
# -*- coding: utf-8 -*-
# This source must be saved in UTF-8 encoding.
#
# p39rec.c
# Copyright (C) 999hatsune@gmail.com 2014
# BSD license
#
# dmc winmm.lib p39rec.c
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

char *nsx39 = "NSX-39";

BYTE dtone[] = {0x3C, 0x3E, 0x40, 0x41, 0x43, 0x45, 0x47, 0x48}; // *** oct. 3
size_t dtone_len = sizeof(dtone) / sizeof(dtone[0]);

BYTE mbuf[4096];

void CALLBACK MyMidiInProc(HMIDIIN hMidiIn,
  UINT msg, DWORD inst, DWORD p1, DWORD p2)
{
  static UINT tick = 0;
  static BYTE flg = 0;
  static BYTE tone = 0x4E;
  static short pitch = 0;
  UINT bend = (p1 >> 8) & 0x0FFFF;
  UINT vel = (p1 >> 16) & 0x0FF;
  BYTE d = (p1 >> 8) & 0x0FF;
  BYTE f = (p1 >> 4) & 0x0F;
  char oct = 99;
  char tonestr[][4] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
  };
  if(msg != 0x03C3) return;
  if(f == 0x0E) pitch = (bend - 0x4000) / 0x0400; // touch after
  else if(f == 0x08) flg = 0; // Note off
  else if(f == 0x09) flg = 1, tone = d + pitch, tick = p2; // Note on
  else ;
  fprintf(stdout, "%08x, %08x, %08x, %08x : b(%04x), p(%6d), %2d %4s %6d\n",
    msg, inst, p1, p2, bend, pitch,
    (tone / 12) - 2,
    f == 0x0E ? (flg ? (bend >= 0x4000 ? "+" : "-") : "") : tonestr[tone % 12],
    p2 - tick);
  if(f == 0x80) tick = p2;
}

int main(int ac, char **av)
{
  HMIDIIN hMI;
  UINT r;
  int i, n = -1;
  r = midiInGetNumDevs();
  fprintf(stdout, "scanning (%d) MIDI Devices\n", r);
  for(i = r; --i >= 0; ){
    MIDIINCAPS mic;
    r = midiInGetDevCaps(i, &mic, sizeof(mic));
    if(!r){
      fprintf(stdout, " %d: %s\n", i, mic.szPname);
      if(!strncmp(mic.szPname, nsx39, strlen(nsx39))) n = i;
    }
  }
  if(n < 0){
    n = 0;
    fprintf(stdout, "Device %s is not found, choose Device %d\n", nsx39, n);
  }
  // _FUNCTION / _THREAD / _WINDOW
  r = midiInOpen(&hMI, n, (ULONG)MyMidiInProc, 0, CALLBACK_FUNCTION);
  assert(!r);
  r = midiInReset(hMI);
  assert(!r);

  {
    MIDIHDR mh = {
      (char *)mbuf, sizeof(mbuf) / sizeof(mbuf[0]),
      0, 0, CALLBACK_NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    r = midiInPrepareHeader(hMI, &mh, sizeof(mh));
    assert(!r);

    r = midiInStart(hMI);
    assert(!r);

    Sleep(30 * 1000);

    r = midiInStop(hMI);
    assert(!r);
  }

  r = midiInReset(hMI);
  assert(!r);
  r = midiInClose(hMI);
  assert(!r);
  return 0;
}
