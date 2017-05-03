# Copyright (C) 2011 oct0xor
# 
# This program is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 2.0.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
# GNU General Public License 2.0 for more details.
# 
# A copy of the GPL 2.0 should have been included with the program.
# If not, see http ://www.gnu.org/licenses/

import sys, os, errno, struct

def get_current_time(time, total_time, drum_n, speed):
    data1, = struct.unpack('<f', time)
    data2, = struct.unpack('<f', total_time)
    data3 = drum_n
    data4, = struct.unpack('<f', speed)
    return ((((data1 / 1000.0) + ((float(data3 + 0x30) * 240.0) / (data2 * 48.0))) * 59.826499939) - 0.5) - (((14400000.0 / (data2 * 1002.90002441)) * 1.200000) / data4)

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

def get_drum(id):
    if (id == 1):
        return 'small 1'
    if (id == 2):
        return 'small 2'
    if (id == 3):
        return 'small 3'
    if (id == 4):
        return 'small blue 1'
    if (id == 5):
        return 'small blue 2'
    if (id == 6):
        return 'small yellow'
    if (id == 7):
        return 'big 1'
    if (id == 8):
        return 'big blue 2'
    if (id == 9):
        return 'big yellow'
    if (id == 0xA):
        return 'balloon'
    if (id == 0xB):
        return 'big 2'
    if (id == 0xC):
        return 'two-sides'
    if (id == 0xD):
        return 'big blue 2'
    if (id == 0xE):
        return '{?}'

f = open('sht', 'rb')

elem_num, = struct.unpack('<L', f.read(4))
drums_table_offset, = struct.unpack('<L', f.read(4))
drums_table_size, = struct.unpack('<L', f.read(4))
unk1 = f.read(4)

for a in xrange(elem_num):
    options_table_elem = f.read(0x8C)
    pos = f.tell()

    time1 = options_table_elem[0:4]
    time2 = options_table_elem[4:8]

    flag1 = options_table_elem[8:0xC]
    flag2 = options_table_elem[0xC:0x10]
    flag3 = options_table_elem[0x14:0x18]

    time3 = options_table_elem[0x2C:0x30]

    drums_offset, = struct.unpack('<L', options_table_elem[0x44:0x48])
    drums_cnt, = struct.unpack('<L', options_table_elem[0x48:0x4C])

    flag4 = options_table_elem[0x4C:0x50]

    current_time = get_current_time(time1, time2, 0, time3)

    total_time = get_total_time(time1, time2, 0) # if less - distance between the lines is less

    print struct.unpack('<f', time1), struct.unpack('<f', time2), struct.unpack('<f', time3)
    print (struct.unpack('<f', time1)[0] / struct.unpack('<f', time2)[0]) * struct.unpack('<f', time3)[0]
    print current_time, total_time
    print total_time - current_time

    f.seek(drums_table_offset + drums_offset * 0x10)

    for b in xrange(drums_cnt):
        drum_table_elem = f.read(0x10)

        drum_id, = struct.unpack('<L', drum_table_elem[0:4])
        drum_offset = drum_table_elem[4:8]

        print '\t' + get_drum(drum_id)

    f.seek(pos)

f.close()