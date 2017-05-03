// Copyright (C) 2011 oct0xor
// 
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License 2.0 for more details.
// 
// A copy of the GPL 2.0 should have been included with the program.
// If not, see http ://www.gnu.org/licenses/

#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "errno.h"

#include "Windows.h"

struct frame
{
	float time1;
	float time2;
	unsigned int flag1;
	unsigned int flag2;
	unsigned int unk1;
	unsigned int flag3;
	unsigned char unk2[0x14];
	float speed;
	unsigned char unk3[0x14];
	unsigned int drums_offset;
	unsigned int drums_cnt;
	unsigned int flag4;
	unsigned char unk4[0x1C];
	unsigned int drums_cnt2;
	unsigned char unk5[0x1C];
};

struct drum
{
	unsigned int id;
	unsigned int offset;
	unsigned int unk1;
	unsigned int unk2;
};

struct table_header
{
	int num_of_frames;
	unsigned int drums_table_start;
	unsigned int num_of_drums;
	unsigned int unk;
};

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("Usage:\n");
		printf("%s in_file out_file\n", argv[0]);
		exit(0);
	}

	FILE *fd;
	frame *buf1;
	drum *buf2;
	table_header hdr;
	char str[0x100];
	float cur_time;
	float min;
	float sec;
	int id;
	int frame_n = 0;
	int drum_n = 0;
	int offset;
	float span;
	float gap;
	float frame_time = 0;
	bool new_frame;
	float time1_delta;
	float time2;
	float speed;
	float frame_in_secs;

	buf1 = (frame *)malloc(0x4000);
	buf2 = (drum *)malloc(0x4000);

	time1_delta = 2000.0;
	time2 = 120.0;
	speed = 1.0;

	frame_in_secs = 2.0;

	span = frame_in_secs / 48.0;

	fopen_s(&fd, argv[1], "rb");

	while (fgets(str, sizeof(str), fd))
	{
		printf("current_line: %s", str);

		sscanf_s(str, "%d:%f - %d", &min, &sec, &id);

		cur_time = min * 60 + sec;

		if (frame_time < cur_time)
		{
			// create new frame

			memset(buf1 + frame_n, 0, sizeof(struct frame));

			(buf1 + frame_n)->time2 = time2;
			(buf1 + frame_n)->flag1 = 1;
			(buf1 + frame_n)->flag3 = 0xFFFFFFFF;
			(buf1 + frame_n)->speed = speed;
			(buf1 + frame_n)->flag4 = 0x5B;

			(buf1 + frame_n)->time1 = -time1_delta + time1_delta * frame_n;
			(buf1 + frame_n)->drums_cnt = 0;
			(buf1 + frame_n)->drums_cnt2 = 0;
			(buf1 + frame_n)->drums_offset = 0;

			frame_n += 1;
			frame_time += frame_in_secs;

			new_frame = TRUE;
		}

		if (frame_time > cur_time)
		{
			(buf1 + frame_n - 1)->drums_cnt += 1;
			(buf1 + frame_n - 1)->drums_cnt2 += 1;

			if (new_frame == TRUE)
			{
				(buf1 + frame_n - 1)->drums_offset = drum_n;
				new_frame = FALSE;
			}

			offset = 0;

			if (cur_time - (frame_time - frame_in_secs) > span)
			{
				gap = 0;
				while (cur_time - (frame_time - frame_in_secs) > gap)
				{
					offset += 1;
					gap += span;
				}
			}

			(buf2 + drum_n)->id = id;
			(buf2 + drum_n)->offset = offset;
			(buf2 + drum_n)->unk1 = -1;
			(buf2 + drum_n)->unk2 = -1;

			drum_n += 1;
		}
	}

	fclose(fd);

	hdr.num_of_frames = frame_n;
	hdr.drums_table_start = frame_n * sizeof(struct frame) + sizeof(struct table_header);
	hdr.num_of_drums = drum_n;
	hdr.unk = 0;

	fopen_s(&fd, argv[2], "wb");

	fwrite(&hdr, sizeof(struct table_header), 1, fd);
	fwrite(buf1, frame_n * sizeof(struct frame), 1, fd);
	fwrite(buf2, drum_n * sizeof(struct drum), 1, fd);

	fclose(fd);

	free(buf1);
	free(buf2);

	return 0;
}
