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
#include "g722_1.h"

#include "Windows.h"

struct audec_state {
	g722_1_decode_state_t dec;
};

struct auenc_state {
	g722_1_encode_state_t enc;
};

int g7221_decode_init(struct audec_state **adsp, int bitrate, int srate)
{
	struct audec_state *ads;

	if (!adsp)
		return EINVAL;

	ads = *adsp;

	ads = (audec_state*)malloc(sizeof(*ads));
	if (!ads)
		return ENOMEM;

	if (!g722_1_decode_init(&ads->dec, bitrate, srate)) {
		free(ads);
		return EPROTO;
	}

	*adsp = ads;

	return 0;
}

int g7221_decode(struct audec_state *ads, int16_t *sampv, size_t *sampc, const uint8_t *buf, size_t len)
{
	g722_1_decode(&ads->dec, sampv, buf, (int)len);

	return 0;
}

int g7221_encode_init(struct auenc_state **aesp, int bitrate, int srate)
{
	struct auenc_state *aes;

	if (!aesp)
		return EINVAL;

	aes = *aesp;

	aes = (auenc_state*)malloc(sizeof(*aes));
	if (!aes)
		return ENOMEM;

	if (!g722_1_encode_init(&aes->enc, bitrate, srate)) {
		free(aes);
		return EPROTO;
	}

	*aesp = aes;

	return 0;
}

int g7221_encode(struct auenc_state *aes, uint8_t *buf, size_t *len, const int16_t *sampv, size_t sampc)
{
	*len = g722_1_encode(&aes->enc, buf, sampv, (int)sampc);

	return 0;
}

void main(int argc, char*argv[])
{
	FILE *fd;
	uint32_t enc_size = 0;
	uint32_t dec_size = 0;
	uint8_t *buf_in;
	uint8_t *buf_out;
	uint8_t header[0x2C];
	int16_t sample_buf[0x1000];
	struct audec_state* ads;
	struct auenc_state* aesp;
	int bitrate = 48000;
	int samplerate = 32000;
	size_t samples_num;
	int len_enc = 0x78;
	int len_dec = 0x500;
	size_t len;
	uint32_t i;

	if (argc != 4)
	{
		printf("Usage:\n");
		printf("Decode: %s d in_file out_file\n", argv[0]);
		printf("Encode: %s e in_file out_file\n", argv[0]);
		exit(0);
	}

	if (*(uint8_t*)(argv[1]) == 'd')
	{
		fd = fopen(argv[2], "rb");

		fseek(fd, 0, 2);

		enc_size = ftell(fd);

		fseek(fd, 0, 0);

		buf_in = (uint8_t*)malloc(enc_size);
		buf_out = (uint8_t*)malloc(enc_size * 0x10);

		fread(buf_in, enc_size, 1, fd);

		fclose(fd);

		for (i = 0; i < enc_size; i += 2) 
		{
			*(uint16_t*)(buf_in + i) = ((*(uint16_t*)(buf_in + i)) >> 8 | ((*(uint16_t*)(buf_in + i)) & 0xFF) << 8);
		}

		g7221_decode_init(&ads, bitrate, samplerate);

		for (i = 0; i < enc_size; i += len_enc) 
		{
			g7221_decode(ads, sample_buf, &samples_num, buf_in + i, len_enc);

			memcpy(buf_out + 0x2C + dec_size, sample_buf, len_dec);

			dec_size += len_dec;
		}

		*(uint32_t*)buf_out = 0x46464952;			// "RIFF"
		*(uint32_t*)(buf_out + 4) = dec_size + 0x24;// chunk size
		*(uint32_t*)(buf_out + 8) = 0x45564157;		// "WAVE"
		*(uint32_t*)(buf_out + 0xC) = 0x20746d66;	// "fmt "
		*(uint32_t*)(buf_out + 0x10) = 0x10; 		// PCM
		*(uint16_t*)(buf_out + 0x14) = 1;			// PCM
		*(uint16_t*)(buf_out + 0x16) = 1;			// mono
		*(uint32_t*)(buf_out + 0x18) = 32000;		// samplerate
		*(uint32_t*)(buf_out + 0x1C) = 128000;		// bitrate
		*(uint16_t*)(buf_out + 0x20) = 4;			// block align
		*(uint16_t*)(buf_out + 0x22) = 0x10;		// PCM
		*(uint32_t*)(buf_out + 0x24) = 0x61746164;	// "data"
		*(uint32_t*)(buf_out + 0x28) = dec_size;	// size

		fd = fopen(argv[3], "wb");

		fwrite(buf_out, dec_size + 0x2C, 1, fd);

		fclose(fd);

		free(buf_in);
		free(buf_out);
	}

	if (*(uint8_t*)(argv[1]) == 'e')
	{
		fd = fopen(argv[2], "rb");

		fread(header, 0x2C, 1, fd);

		if (*(uint32_t*)(header + 8) != 0x45564157)
		{
			printf("Error: %s is not WAV\n", argv[2]);
			exit(0);
		}

		if (*(uint32_t*)(header + 0x18) != 32000)
		{
			printf("Error: Samplerate is not equal to 32000\n");
			exit(0);
		}

		if (*(uint32_t*)(header + 0x24) != 0x61746164)
			exit(0);

		dec_size = *(uint32_t*)(header + 0x28);

		buf_in = (uint8_t*)malloc(dec_size);
		buf_out = (uint8_t*)malloc(dec_size);

		fread(buf_in, dec_size, 1, fd);

		fclose(fd);

		g7221_encode_init(&aesp, bitrate, samplerate);

		for (i = 0; i < dec_size; i += len_dec) 
		{
			memset(sample_buf, 0, sizeof(sample_buf));

			if (i + len_dec > dec_size)
				memcpy(sample_buf, buf_in + i, dec_size - i);
			else
				memcpy(sample_buf, buf_in + i, len_dec);

			g7221_encode(aesp, buf_out + enc_size, &len, sample_buf, len_dec / 2);

			enc_size += len;
		}

		for (i = 0; i < enc_size; i += 2) 
		{
			*(uint16_t*)(buf_out + i) = ((*(uint16_t*)(buf_out + i)) >> 8 | ((*(uint16_t*)(buf_out + i)) & 0xFF) << 8);
		}

		fd = fopen(argv[3], "wb");

		if (enc_size > 0xE6000)
			printf("Error: out size is more than 0xE6000 bytes\n");

		fwrite(buf_out, enc_size, 1, fd);

		fclose(fd);

		free(buf_in);
		free(buf_out);
	}
}