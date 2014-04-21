/*
# -*- coding: utf-8 -*-
# This source must be saved in UTF-8 encoding.
#
# p39lib.h
# Copyright (C) 999hatsune@gmail.com 2014
# BSD license
#
# dmc -WD kernel32.lib winmm.lib p39lib.c
*/

#ifndef __P39LIB_H_
#define __P39LIB_H_

#ifdef __P39LIB_MAKE_DLL_
#define __PORT __declspec(dllexport) // make dll mode
#else
#define __PORT __declspec(dllimport) // use dll mode
#endif

typedef struct _Pocket39_tag {
  HMIDIOUT hMO;
  BYTE dev_id; // 1
  BYTE ch; // 0
  BYTE stat; // 0
  BYTE mod; // 0
  BYTE tone; // F (F#4)
  char sft; // 1 (F#4)
  char oct; // 4 (F#4)
  BYTE vel; // 100
  UINT len; // 120
  UINT bend; // 50% 512 (1024: C -> D), (-512: F -> E)
  int pitch; // 0
  UINT tempo; // 240
} Pocket39;

__PORT BOOL APIENTRY DllMain(HINSTANCE, DWORD, LPVOID);
__PORT Pocket39 * WINAPI p39open();
__PORT UINT WINAPI p39close(Pocket39 *);
__PORT UINT WINAPI p39reset(Pocket39 *);
__PORT UINT WINAPI p39programs(Pocket39 *, BYTE *, size_t);
__PORT UINT WINAPI p39singW(Pocket39 *, wchar_t *, wchar_t *);

#endif // __P39LIB_H_
