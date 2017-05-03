# Taiko no Tatsujin DS Custom Songs

Here is my rom hacking attempt that I made in 2011 for this great rhythm game.

Reverse engineered necessary minimum for making custom tracks, there are some problems with timing (explained later) and no gui, etc. not handy to use at all, so its PoC.

I never had intention to finish this project, because of who would ever want to make custom lo-fi mono songs for 2007 handheld game? But it's still was fun to figure out.

## Contents

`rom_tool` - tool for packing & unpacking rom by [theli](https://github.com/theli-ua), found on gbatemp. Compression dosnt work, was lazy to check why.

`taiko_music` - tool for decoding & encoding Taiko no Tatsujin DS music with Siren14 codec. Music files stored at "data.000\siren" folder.

`taiko_fumen_decode` - tool for decoding fumens (tracks). Fumens stored at "data.000\fumen". "e" suffix means "easy", "n" = "normal", etc.

`taiko_fumen` - tool for encoding fumens (tracks) to binary format.

`example` - fumen for beginning of "Village People - Y.M.C.A." that you can see in the video.

## Timing Problem

Game takes values from fumens and do such math (code rewritten to python) and uses new values.

```
def get_current_time(time, total_time, drum_n, speed):
    data1, = struct.unpack('<f', time)
    data2, = struct.unpack('<f', total_time)
    data3 = drum_n
    data4, = struct.unpack('<f', speed)
    return ((((data1 / 1000.0) + ((float(data3 + 0x30) * 240.0) / (data2 * 48.0))) * 59.826499939) - 0.5) - (((14400000.0 / (data2 * 1002.90002441)) * 1.200000) / data4)
```
```
def get_total_time(time, total_time, drum_n):
    data1, = struct.unpack('<f', time)
    data2, = struct.unpack('<f', total_time)
    data3 = drum_n
    v0 = data1 / 1000.0
    v1 = float(data3 + 0x30) * 240.0
    v2 = data2 * 48.0
    v4 = v1 / v2
    v5 = v0 + v4
    v6 = v5 * 59.826499939
    return v6 - 0.5
```

Doesnt make much sense for me. Correct timing can be determined by brute.

## Drum ID's

- 1 - small 1
- 2 - small 2
- 3 - small 3
- 4 - small blue 1
- 5 - small blue 2
- 6 - small yellow
- 7 - big 1
- 8 - big blue 2
- 9 - big yellow
- 0xA - balloon
- 0xB - big 2
- 0xC - two-sides
- 0xD - big blue 2

Author: oct0xor <oct0xor@gmail.com>

Licensed under the GPLv2 license.

## Dependencies

Siren14 codec licensed by Polycom.

/*
 * g722_1 - a library for the G.722.1 and Annex C codecs
 *
 * g722_1.h
 *
 * Adapted by Steve Underwood <steveu@coppice.org> from the reference
 * code supplied with ITU G.722.1, which is:
 *
 *   (C) 2004 Polycom, Inc.
 *   All rights reserved.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */