/*
# -*- coding: utf-8 -*-
# This source must be saved in UTF-8 encoding.
#
# p39lib.c
# Copyright (C) 999hatsune@gmail.com 2014
# BSD license
#
# dmc -WD kernel32.lib winmm.lib p39lib.c
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define __P39LIB_MAKE_DLL_
#include "p39lib.h"

#define P39NOTEOFF(ch, tone, vel) (((vel) << 16) | ((tone) << 8) | (ch) | 0x80)
#define P39NOTEON(ch, tone, vel) (((vel) << 16) | ((tone) << 8) | (ch) | 0x90)
#define P39PKPRESS(ch, tone, vel) (((vel) << 16) | ((tone) << 8) | (ch) | 0xA0)
#define P39CC(ch, ctrl, val) (((val) << 16) | ((ctrl) << 8) | (ch) | 0xB0)
#define P39PROG(ch, pg) (((pg) << 8) | (ch) | 0xC0)
#define P39CHPRESS(ch, vel) (((vel) << 8) | (ch) | 0xD0)
#define P39PITCH(ch, lsb, msb) (((msb) << 16) | ((lsb) << 8) | (ch) | 0xE0)

char *nsx39 = "NSX-39";

BYTE default_banks[] = {
  0x06, 0x7A, 0x7B, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x6A
};
size_t default_banks_len = sizeof(default_banks) / sizeof(default_banks[0]);

BYTE dtone[] = {0x3C, 0x3E, 0x40, 0x41, 0x43, 0x45, 0x47, 0x48}; // *** oct. 3
size_t dtone_len = sizeof(dtone) / sizeof(dtone[0]);

BYTE sysex0A[] = {0xF0, 0x43, 0x79, 0x09, 0x11, 0x0A, 0x00, 0x65, 0xF7};
size_t sysex0A_len = sizeof(sysex0A) / sizeof(sysex0A[0]);

char pron[][7] = {
    "あ",   "い",   "う",   "え",   "お",   "か",   "き",   "く",
    "け",   "こ",   "が",   "ぎ",   "ぐ",   "げ",   "ご", "きゃ",
  "きゅ", "きょ", "ぎゃ", "ぎゅ", "ぎょ",   "さ", "すぃ",   "す",
    "せ",   "そ",   "ざ", "ずぃ",   "ず",   "ぜ",   "ぞ", "しゃ",
    "し", "しゅ", "しぇ", "しょ", "じゃ",   "じ", "じゅ", "じぇ",
  "じょ",   "た", "てぃ", "とぅ",   "て",   "と",   "だ", "でぃ",
  "どぅ",   "で",   "ど", "てゅ", "でゅ", "ちゃ",   "ち", "ちゅ",
  "ちぇ", "ちょ", "つぁ", "つぃ",   "つ", "つぇ", "つぉ",   "な",
    "に",   "ぬ",   "ね",   "の", "にゃ", "にゅ", "にょ",   "は",
    "ひ",   "ふ",   "へ",   "ほ",   "ば",   "び",   "ぶ",   "べ",
    "ぼ",   "ぱ",   "ぴ",   "ぷ",   "ぺ",   "ぽ", "ひゃ", "ひゅ",
  "ひょ", "びゃ", "びゅ", "びょ", "ぴゃ", "ぴゅ", "ぴょ", "ふぁ",
  "ふぃ", "ふゅ", "ふぇ", "ふぉ",   "ま",   "み",   "む",   "め",
    "も", "みゃ", "みゅ", "みょ",   "や",   "ゆ",   "よ",   "ら",
    "り",   "る",   "れ",   "ろ", "りゃ", "りゅ", "りょ",   "わ",
    "ゐ",   "ゑ",   "を",  "N\\",    "m",    "N",    "J",    "n",
  // aliases (offset 0x80 to pron_ext[])
  "づぁ", "づぃ",   "づ", "づぇ", "づぉ", "うぃ", "うぇ", "うぉ",
    "ん",   "っ",   "、",   "。",   "ー"
};
size_t pron_len = sizeof(pron) / sizeof(pron[0]);

char pron_katakana[sizeof(pron) / sizeof(pron[0])][7] = {0};

char pron_ext[] = {
  0x1A, 0x1B, 0x1C, 0x1D, 0x1E, // づぁ づぃ づ づぇ づぉ
  0x78, 0x79, 0x7A, 0x7B, // うぃ うぇ うぉ ん
  0x80, 0x80, 0x80, 0xFF // っ 、 。 ー
};
size_t pron_ext_len = sizeof(pron_ext) / sizeof(pron_ext[0]);

BYTE pron_match[sizeof(pron) / sizeof(pron[0])] = {0};

BYTE p39pron_scan(char *lyric)
{
  int i;
  for(i = 0; i < pron_len; ++i){
    BYTE idx = pron_match[i];
    if(!strncmp(lyric, pron[idx], strlen(pron[idx]))) return idx;
    if(!strlen(pron_katakana[idx])) continue;
    if(!strncmp(lyric, pron_katakana[idx], strlen(pron[idx]))) return idx;
  }
  return 0xFF;
}

UINT p39conv_kana(BYTE *dst, BYTE *src, BYTE mode) // mode 0: h -> k, 1: k -> h
{
/*
  convert UTF-8 hiragana <-> katakana
  hiragana E38181 'ぁ' - E38293 'ん'
  katakana E382A1 'ァ' - E383B3 'ン' ( ignore 'ヴ' 'ヵ' 'ヶ' )
  bytes must be in range 0x80 - 0xBF
    to katakana = hiragana + 0x0120 (+ 0x00C0 when second byte is 0xA0 - 0xBF)
    to hiragana = katakana - 0x0120 (- 0x00C0 when second byte is 0x80 - 0x9F)
*/
  UINT u8h;
  dst[0] = dst[1] = dst[2] = '\0';
  if(src[0] != 0x0e3) return 0;
  u8h = (src[1] << 8) | src[2];
  if(!mode){
    if(u8h < 0x08181 || u8h > 0x08293) return 0;
    u8h += 0x0120;
    if(src[2] >= 0x0A0 && src[2] <= 0x0BF) u8h += 0x00C0;
  }else{
    if(u8h < 0x082A1 || u8h > 0x083B3) return 0;
    u8h -= 0x0120;
    if(src[2] >= 0x080 && src[2] <= 0x09F) u8h -= 0x00C0;
  }
  dst[0] = src[0];
  dst[1] = (u8h >> 8) & 0x0FF;
  dst[2] = u8h & 0x0FF;
  return 1;
}

UINT p39prepare(Pocket39 *p39)
{
  int i, j, cnt = 0;
  p39->hMO = 0;
  p39->dev_id = 1;
  p39->ch = 0;
  p39->stat = 0;
  p39->mod = 0;
  p39->tone = 'F';
  p39->sft = 1;
  p39->oct = 4;
  p39->vel = 100;
  p39->len = 120;
  p39->bend = 512;
  p39->pitch = 0;
  p39->tempo = 240;
  for(i = 0; i < pron_len; ++i){
    for(j = 0; j < (sizeof(pron[0]) / sizeof(pron[0][0])) / 3; ++j){
      int m = j * 3;
      if(!p39conv_kana((BYTE *)&pron_katakana[i][m], (BYTE *)&pron[i][m], 0))
        break;
    }
  }
//  for(i = 0; i < pron_len; ++i) fprintf(stdout, " %s", pron_katakana[i]);
  for(j = sizeof(pron[0]) / sizeof(pron[0][0]); --j > 0; )
    for(i = 0; i < pron_len; ++i)
      if(strlen(pron[i]) == j) pron_match[cnt++] = (BYTE)i;
  assert(cnt == pron_len);
//  for(i = 0; i < pron_len; ++i) fprintf(stdout, " %02x", pron_match[i]);
  return 0;
}

__PORT UINT WINAPI p39reset(Pocket39 *p39)
{
  UINT r;
  assert(p39);
  r = midiOutReset(p39->hMO);
  assert(!r);
  return 0;
}

__PORT Pocket39 * WINAPI p39open()
{
  UINT r;
  int i, n = -1;
  Pocket39 *p39 = (Pocket39 *)malloc(sizeof(Pocket39));
  assert(p39);
  r = p39prepare(p39);
  assert(!r);
  r = midiOutGetNumDevs();
  fprintf(stdout, "scanning (%d) MIDI Devices\n", r);
  for(i = r; --i >= 0; ){
    MIDIOUTCAPS moc;
    r = midiOutGetDevCaps(i, &moc, sizeof(moc));
    if(!r){
      fprintf(stdout, " %d: %s\n", i, moc.szPname);
      if(!strncmp(moc.szPname, nsx39, strlen(nsx39))) n = i;
    }
  }
  if(n < 0){
    n = 0;
    fprintf(stdout, "Device %s is not found, choose Device %d\n", nsx39, n);
    p39->bend = 4095; // *** wrong way ? must send RPN ?
  }
  p39->dev_id = n;
  r = midiOutOpen(&p39->hMO, p39->dev_id, (ULONG)NULL, 0, CALLBACK_NULL);
  assert(!r);
  r = p39reset(p39);
  assert(!r);
  return p39;
}

__PORT UINT WINAPI p39close(Pocket39 *p39)
{
  UINT r;
  assert(p39);
  p39reset(p39);
  r = midiOutClose(p39->hMO);
  assert(!r);
  free(p39);
  return 0;
}

__PORT UINT WINAPI p39programs(Pocket39 *p39, BYTE *banks, size_t len)
{
  int i;
  for(i = 0; i < len; ++i){
    UINT r = midiOutShortMsg(p39->hMO, P39PROG(i, banks[i]));
    assert(!r);
  }
  return 0;
}

UINT p39wait(Pocket39 *p39, int len)
{
#if 0
  char buf[4096];
  fputs("waiting...: ", stdout);
  fgets(buf, sizeof(buf), stdin);
#else
  Sleep(len * p39->tempo / 120);
#endif
  return 0;
}
char *p39tonestr(char *tonestr, UINT n)
{
  int s = ((n >> 8) & 0x3F) - 32;
  int o = (n >> 4) & 0x07;
  if(n & 0x08){ // quiet
    // Ex / CC / bend / mod etc...
    BYTE c = n & 0x07;
    tonestr[0] = ' ';
    if(c == 0 || c == 1){
      tonestr[1] = ' ';
      tonestr[2] = s ? (s > 0 ? '#' : '=') : ' ';
    }else if(c == 2 || c == 3){
      tonestr[1] = ' ';
      tonestr[2] = s ? (s > 0 ? '#' : '=') : ' ';
    }else if(c == 4 || c == 5){
      tonestr[0] = '0' + o;
      tonestr[1] = ' ';
      tonestr[2] = ' ';
    }else{
      tonestr[1] = ' ';
      tonestr[2] = ' ';
    }
  }else{
    BYTE k = n & 0x07;
    if(k == 7){
      tonestr[0] = ' ';
      tonestr[1] = 'R';
      tonestr[2] = ' ';
    }else{
      tonestr[0] = '0' + o;
      tonestr[1] = 'A' + k;
      tonestr[2] = s ? (s > 0 ? '#' : '=') : ' ';
    }
  }
  tonestr[3] = '\0';
  return tonestr;
}

UINT p39note(Pocket39 *p39,
  BYTE ch, BYTE flg, BYTE tone, char sft, char oct, BYTE vel, int len)
{
  UINT r, f;
  BYTE d;
  char o = (dtone[dtone_len - 1] - dtone[0]) * (oct - 4);
  BYTE t = tone - 'C' + ((tone == 'A' || tone == 'B') ? 7 : 0);
  assert(tone >= 'A' && tone <= 'G');
  d = dtone[t] + sft + o + p39->pitch;
  if(!flg) f = P39NOTEOFF(ch, d, vel);
  else if(flg == 1) f = P39NOTEON(ch, d, vel);
  else if(flg == 2) f = P39PKPRESS(ch, d, vel);
  else f = P39PKPRESS(ch, d, vel);
  r = midiOutShortMsg(p39->hMO, f);
#if 0
r = midiOutShortMsg(p39->hMO, (f & 0xFFFF00F0) | ((d + 0) << 8) | 0x0f); // 4 5 7 12
//r = midiOutShortMsg(p39->hMO, (f & 0xFFFFFFF0) | 0x09); // 4 5 7 12
//r = midiOutShortMsg(p39->hMO, (f & 0xFFFF00F0) | ((d + 12) << 8) | 0x08); // 4 5 7 12
//r = midiOutShortMsg(p39->hMO, (f & 0xFFFF00F0) | ((d + 7) << 8) | 0x05); // 4 5 7 12
//r = midiOutShortMsg(p39->hMO, (f & 0xFFFF00F0) | ((d + 4) << 8) | 0x04); // 4 5 7 12
#endif
  assert(!r);
#if 1
  if(len) p39wait(p39, len);
#else
  if(len){
    p39wait(p39, len);
    if(flg) p39note(p39, ch, 0, tone, sft, oct, 0, 0); // vel, len
  }
#endif
  return 0;
}

UINT p39bend(Pocket39 *p39, BYTE ch, int val, int len)
{
  int v = val + 0x2000;
  BYTE msb = (v >> 7) & 0x7F;
  BYTE lsb = v & 0x7F;
  UINT r = midiOutShortMsg(p39->hMO, P39PITCH(ch, lsb, msb));
#if 0
r = midiOutShortMsg(p39->hMO, P39PITCH(0x0f, lsb, msb));
//r = midiOutShortMsg(p39->hMO, P39PITCH(0x09, lsb, msb));
//r = midiOutShortMsg(p39->hMO, P39PITCH(0x08, lsb, msb));
//r = midiOutShortMsg(p39->hMO, P39PITCH(0x05, lsb, msb));
//r = midiOutShortMsg(p39->hMO, P39PITCH(0x04, lsb, msb));
#endif
  assert(!r);
  if(len) p39wait(p39, len);
  return 0;
}

UINT p39shift(Pocket39 *p39, BYTE ch, int sft, int len)
{
  p39bend(p39, ch, sft * p39->bend, len);
  return 0;
}

UINT p39modulation(Pocket39 *p39, BYTE ch, BYTE mod, int len)
{
  return 0;
}

UINT p39voice(Pocket39 *p39, BYTE ch, BYTE voice)
{
  UINT r;
  MIDIHDR mh = {
    (char *)sysex0A, sizeof(sysex0A) / sizeof(sysex0A[0]),
    0, 0, CALLBACK_NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
//  fprintf(stdout, "%s\n", pron[voice]);
  sysex0A[7] = voice;
  r = midiOutPrepareHeader(p39->hMO, &mh, sizeof(mh));
  assert(!r);
  r = midiOutLongMsg(p39->hMO, &mh, sizeof(mh));
  assert(!r);
  return 0;
}

int p39get_number(char **p)
{
  int num = 0;
  if(**p < '0' || **p > '9') return -1;
  while(**p >= '0' && **p <= '9') num = num * 10 + *(*p)++ - '0';
  return num;
}

UINT p39sing(Pocket39 *p39, char *lyrics, char *notes)
{
  int num;
  int pitch = p39->pitch;
  char sft = p39->sft;
  char oct = p39->oct;
  char *p = lyrics, *q = notes;
  BYTE voice_buf[4096]; // 0x00 - 0x7F, 0x80, 0xFF
  BYTE *v = voice_buf;
  UINT note_buf[4096]; // see below
  UINT *n = note_buf;
/*
  about 'note_buf[]':
    bits 31-21: length 0-2047
    bits 20-14: velocity 0-127
    bits 13- 8: pitch shift (center 32)
    bits     7: not use
    bits  6- 4: octave 0-7
    bits  3- 0: fkkk
        kkk:         0 1 2 3 4 5 6 7
      f = 0: (sound) A B C D E F G R (takes voice at the same time)
      f = 1: (quiet) + - # = [ ] v m (v: velocity, m: modulation)
    '#' means sharp
    '=' means flat
*/

  fprintf(stdout, "%s / %s\n", lyrics, notes);
  do{
    BYTE idx = p39pron_scan(p);
    *v = 0xFF;
    if(idx == 0xFF){
      ++p;
      continue;
    }
    p += strlen(pron[idx]);
//    fprintf(stdout, "%08x %s\n", idx, pron[idx]);
    if(idx & 0x80) if((idx = pron_ext[idx & 0x7F]) == 0xFF) continue;
    *v++ = idx;
  }while(*p);
  *v = 0xFF;

  if(notes[0] == '\0'){
    BYTE ch = p39->ch;
    for(v = voice_buf; *v != 0xFF; ){
      BYTE idx = *v++;
      if(idx & 0x80){
        p39note(p39, ch, 0, p39->tone, p39->sft, p39->oct, p39->vel, p39->len);
      }else{
        p39voice(p39, 0, idx);
        p39note(p39, ch, 1, p39->tone, p39->sft, p39->oct, p39->vel, p39->len);
      }
    }
    return 0;
  }

  do{
    BYTE d = *q++;
    UINT u = 0;
    if(d == ' '){ continue;
    }else if(d >= 'A' && d <= 'G'){
      u |= d - 'A'; // 0-6
      if(*q == '#' || *q == '='){
        sft = (*q == '#') ? 1 : -1;
        ++q;
        if(*q == '#' || *q == '='){
          sft += (*q == '#') ? 1 : -1;
          ++q;
          if(*q == '#' || *q == '='){
            sft += (*q == '#') ? 1 : -1;
            ++q;
          }
        }
      }else sft = 0;
    }else if(d == 'R'){ u |= 7;
    }else if(d == '+'){ u |= 8; ++sft; // ++pitch;
    }else if(d == '-'){ u |= 9; --sft; // --pitch;
    }else if(d == '#'){ u |= 10; ++sft;
    }else if(d == '='){ u |= 11; --sft;
    }else if(d == '[' || d == '<'){ u |= 12; ++oct;
    }else if(d == ']' || d == '>'){ u |= 13; --oct;
    }else if(d == 'v'){ u |= 14;
    }else if(d == 'm'){ u |= 15;
    }else{
      fprintf(stderr, "unexpected character [%c]\n", d);
      continue;
    }
    num = p39get_number(&q);
//    if(num) fprintf(stdout, "%d\n", num);
    u |= (sft + 32) << 8;
    u |= oct << 4;
    // velocity or modulation
    u |= ((d != 'v' && d != 'm') ? 100 : \
      ((num >= 0) ? num : (d == 'm' ? 0 : 100))) << 14;
    // length
    u |= ((d != 'v' && d != 'm') ? ((num >= 0) ? num : 120) : 120) << 21;
    *n++ = u;
  }while(*q);
  *n = 0xFF;

  for(v = voice_buf, n = note_buf; *n != 0xFF; ++n){
    char tonestr[4];
    BYTE ch = p39->ch;
    BYTE k = *n & 0x07;
    BYTE idx = *v;
    char *s = idx != 0xFF ? (idx != 0x80 ? pron[idx] : "0x80") : "0xFF";
    p39->sft = ((*n >> 8) & 0x3F) - 32;
    p39->oct = (*n >> 4) & 0x07;
    p39->vel = (*n >> 14) & 0x7F;
    p39->len = (*n >> 21) & 0x03FF;
#if 1
    fprintf(stdout, "%08x %3s %6d v(%3d) o(%d) s(%3d) p(%3d) m(%3d) %s\n",
      *n, p39tonestr(tonestr, *n),
      p39->len, p39->vel, p39->oct, p39->sft, p39->pitch, p39->mod,
      (*n & 0x08 || (k == 7 && !(idx & 0x80))) ? ".." : s);
#endif
    if(*n & 0x08){ // quiet
      if(k == 0 || k == 1) p39shift(p39, ch, p39->sft, p39->len); // pitch
      if(k == 2 || k == 3) p39shift(p39, ch, p39->sft, p39->len);
      if(k == 6)
        p39note(p39, ch, 2, p39->tone, p39->sft, p39->oct, p39->vel, p39->len);
      continue;
    }
    if(idx == 0xFF) ch = 3;
    if(k == 7){ // 'R'
      if(idx == 0x80) ++v;
      p39note(p39, ch, 0, p39->tone, p39->sft, p39->oct, p39->vel, p39->len);
    }else{
      if(idx != 0xFF) ++v;
      else if(lyrics[0] != '\0') break; // voice_buf[0] != 0xFF
      if(1) p39bend(p39, 0, 0, 0); // *** not sure
      p39->tone = 'A' + k;
      p39voice(p39, 0, idx);
      p39note(p39, ch, 1, p39->tone, p39->sft, p39->oct, p39->vel, p39->len);
    }
  }

  return 0;
}

BOOL APIENTRY DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved)
{
  switch(reason){
  case DLL_PROCESS_ATTACH:
    break;
  case DLL_PROCESS_DETACH:
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  default:
    break;
  }
  return TRUE;
}

__PORT UINT WINAPI p39singW(Pocket39 *p39,wchar_t *lyrics, wchar_t *notes)
{
#if 0
  FILE *fp = fopen("c:\\prj\\dprj\\pocket39\\p39lib.log", "ab");
  fprintf(fp, "%d, %d\n", wcslen(lyrics), wcslen(notes));
  fwprintf(fp, L"%s\n", lyrics);
  fwprintf(fp, L"%s\n", notes);
  fprintf(fp, "p39 %08x\n", p39);
  fclose(fp);
#endif
  if(p39) p39sing(p39, "あ", "F#");
  return wcslen(lyrics);
}
