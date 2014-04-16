/*
# -*- coding: utf-8 -*-
# This source must be saved in UTF-8 encoding.
#
# pocket39.c
# Copyright (C) 999hatsune@gmail.com 2014
# BSD license
#
# dmc winmm.lib pocket39.c
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define P39NOTEOFF(ch, tone, vel) (((vel) << 16) | ((tone) << 8) | (ch) | 0x80)
#define P39NOTEON(ch, tone, vel) (((vel) << 16) | ((tone) << 8) | (ch) | 0x90)
#define P39CC(ch, ctrl, val) (((val) << 16) | ((ctrl) << 8) | (ch) | 0xB0)
#define P39PROG(ch, pg) (((pg) << 8) | (ch) | 0xC0)
#define P39PITCH(ch, lsb, msb) (((msb) << 16) | ((lsb) << 8) | (ch) | 0xE0)

char *nsx39 = "NSX-39";

typedef struct _Pocket39_tag {
  HMIDIOUT hMO;
  BYTE tone; // F (F#4)
  char sft; // 1 (F#4)
  char oct; // 4 (F#4)
  BYTE vel; // 100
  UINT len; // 120
  UINT bend; // 50% 512 (1024: C -> D), (-512: F -> E)
  int pitch; // 0
  UINT tempo; // 240
} Pocket39;

BYTE default_banks[] = {
  0x06, 0x7A, 0x7B, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};
size_t default_banks_len = sizeof(default_banks) / sizeof(default_banks[0]);

BYTE dtone[] = {0x3C, 0x3E, 0x40, 0x41, 0x43, 0x45, 0x47, 0x48};
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
    "ん",   "ー",   "っ",   "、",   "。"
};
size_t pron_len = sizeof(pron) / sizeof(pron[0]);

char pron_ext[] = {
  0x1A, 0x1B, 0x1C, 0x1D, 0x1E, // づぁ づぃ づ づぇ づぉ
  0x78, 0x79, 0x7A, 0x7B, // うぃ うぇ うぉ ん
  0xFF, 0xFF, 0xFF, 0xFF // ー っ 、 。
};

UINT pron_match[sizeof(pron) / sizeof(pron[0])] = {0};

UINT p39prepare(Pocket39 *p39)
{
  int i, j, cnt = 0;
  p39->hMO = 0;
  p39->tone = 'F';
  p39->sft = 1;
  p39->oct = 4;
  p39->vel = 100;
  p39->len = 120;
  p39->bend = 512;
  p39->pitch = 0;
  p39->tempo = 240;
  for(j = sizeof(pron[0]) / sizeof(pron[0][0]); --j > 0; )
    for(i = 0; i < pron_len; ++i)
      if(strlen(pron[i]) == j) pron_match[cnt++] = i;
  assert(cnt == pron_len);
//  for(i = 0; i < pron_len; ++i) fprintf(stdout, " %02x", pron_match[i]);
  return 0;
}

Pocket39 *p39open()
{
  UINT r;
  int i, id = 1;
  Pocket39 *p39 = (Pocket39 *)malloc(sizeof(Pocket39));
  assert(p39);
  r = p39prepare(p39);
  assert(!r);
  r = midiOutGetNumDevs();
  for(i = r; --i >= 0; ){
    MIDIOUTCAPS moc;
    r = midiOutGetDevCaps(i, &moc, sizeof(moc));
    if(!r) fprintf(stdout, " %d: %s\n", i, moc.szPname);
  }
  r = midiOutOpen(&p39->hMO, id, (ULONG)NULL, 0, CALLBACK_NULL);
  if(r){
    r = midiOutOpen(&p39->hMO, --id, (ULONG)NULL, 0, CALLBACK_NULL);
    p39->bend = 4095;
  }
  assert(!r);
  return p39;
}

UINT p39reset(Pocket39 *p39)
{
  UINT r;
  assert(p39);
  r = midiOutReset(p39->hMO);
  assert(!r);
  return 0;
}

UINT p39close(Pocket39 *p39)
{
  UINT r;
  assert(p39);
  p39reset(p39);
  r = midiOutClose(p39->hMO);
  assert(!r);
  free(p39);
  return 0;
}

UINT p39programs(Pocket39 *p39, BYTE *banks, size_t len)
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

UINT p39note(Pocket39 *p39,
  BYTE ch, BYTE flg, BYTE tone, char sft, char oct, BYTE vel, int len)
{
  UINT r;
  BYTE d;
  char o = (dtone[dtone_len - 1] - dtone[0]) * (oct - 4);
  BYTE t = tone - 'C' + ((tone == 'A' || tone == 'B') ? 7 : 0);
  assert(tone >= 'A' && tone <= 'G');
  d = dtone[t] + sft + o;
  r = midiOutShortMsg(p39->hMO,
    flg ? P39NOTEON(ch, d, vel) : P39NOTEOFF(ch, d, vel));
  assert(!r);
  if(len) p39wait(p39, len);
  return 0;
}

UINT p39bend(Pocket39 *p39, BYTE ch, int val, int len)
{
  int v = val + 0x2000;
  BYTE msb = (v >> 7) & 0x7F;
  BYTE lsb = v & 0x7F;
  UINT r = midiOutShortMsg(p39->hMO, P39PITCH(ch, lsb, msb));
  assert(!r);
  if(len) p39wait(p39, len);
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

UINT p39sing(Pocket39 *p39, char *lyrics, char *notes)
{
  fprintf(stdout, "%s / %s\n", lyrics, notes);
  return 0;
}

int main(int ac, char **av)
{
  Pocket39 *p39 = p39open();
  p39reset(p39);
  p39programs(p39, default_banks, default_banks_len);
  p39note(p39, 1, 1, 'E', 0, 4, 100, 120);

  p39voice(p39, 0, 0x00);
  p39note(p39, 0, 1, 'G', 0, 4, 100, 360);
  p39voice(p39, 0, 0x29);
  p39note(p39, 0, 1, 'G', 0, 4, 100, 120);
  p39voice(p39, 0, 0x64);
  p39note(p39, 0, 1, 'A', 0, 4, 100, 240);
  p39voice(p39, 0, 0x04);
  p39note(p39, 0, 1, 'G', 0, 4, 100, 240);

  p39voice(p39, 0, 0x07);
  p39note(p39, 0, 1, 'E', 0, 4, 100, 240);
  p39voice(p39, 0, 0x68);
  p39note(p39, 0, 1, 'C', 0, 4, 100, 120);
  p39bend(p39, 0, 2 * p39->bend, 120);
  p39note(p39, 0, 0, 'C', 0, 4, 100, 0);
  p39bend(p39, 0, 0, 0);
  p39voice(p39, 0, 0x43);
  p39note(p39, 0, 1, 'E', 0, 4, 100, 360);
  p39note(p39, 0, 0, 'E', 0, 4, 100, 120);

  p39voice(p39, 0, 0x02);
  p39note(p39, 0, 1, 'D', 0, 4, 100, 360);
  p39voice(p39, 0, 0x03);
  p39note(p39, 0, 1, 'G', 0, 4, 100, 120);
  p39voice(p39, 0, 0x40);
  p39note(p39, 0, 1, 'G', 0, 4, 100, 240);
  p39voice(p39, 0, 0x2E);
  p39note(p39, 0, 1, 'F', 0, 4, 100, 120);
  p39bend(p39, 0, -1 * p39->bend, 120);
  p39note(p39, 0, 0, 'F', 0, 4, 100, 0);
  p39bend(p39, 0, 0, 0);
  p39voice(p39, 0, 0x20);
  p39note(p39, 0, 1, 'D', 0, 4, 100, 840);
  p39note(p39, 0, 0, 'D', 0, 4, 100, 120);

  p39voice(p39, 0, 0x20);
  p39note(p39, 0, 1, 'G', 0, 4, 100, 360);
  p39voice(p39, 0, 0x4B);
  p39note(p39, 0, 1, 'G', 0, 4, 100, 120);
  p39voice(p39, 0, 0x04);
  p39note(p39, 0, 1, 'E', 0, 4, 100, 240);
  p39voice(p39, 0, 0x43);
  p39note(p39, 0, 1, 'C', 0, 4, 100, 240);
  p39voice(p39, 0, 0x6C);
  p39note(p39, 0, 1, 'A', 0, 4, 100, 360);
  p39voice(p39, 0, 0x00);
  p39note(p39, 0, 1, 'B', 0, 4, 100, 120);
  p39voice(p39, 0, 0x64);
  p39note(p39, 0, 1, 'C', 0, 5, 100, 240);
  p39voice(p39, 0, 0x04);
  p39note(p39, 0, 1, 'A', 0, 4, 100, 240);

  p39voice(p39, 0, 0x65);
  p39note(p39, 0, 1, 'G', 0, 4, 100, 360);
  p39voice(p39, 0, 0x04);
  p39note(p39, 0, 1, 'A', 0, 4, 100, 120);
  p39voice(p39, 0, 0x73);
  p39note(p39, 0, 1, 'G', 0, 4, 100, 120);
  p39voice(p39, 0, 0x04);
  p39note(p39, 0, 1, 'F', 0, 4, 100, 120);
  p39voice(p39, 0, 0x20);
  p39note(p39, 0, 1, 'E', 0, 4, 100, 120);
  p39voice(p39, 0, 0x01);
  p39note(p39, 0, 1, 'D', 0, 4, 100, 120);
  p39voice(p39, 0, 0x2C);
  p39note(p39, 0, 1, 'C', 0, 4, 100, 840);

  p39sing(p39, "あたまを", "G360GA240G240");
  p39sing(p39, "くもの", "E240C120#0#E360R");
  p39sing(p39, "うえにだし", "D360GG240F120=D840R");
  p39sing(p39, "しほーの", "G360G120=0=0=240C240");
  p39sing(p39, "やまを", "A360#0#[C240]A240");
  p39sing(p39, "みおろして", "G360AG120=0=E120=0=C840R");

  p39close(p39);
  return 0;
}
