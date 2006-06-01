/*	$NetBSD: destest.c,v 1.3 2005/02/06 06:05:19 perry Exp $	*/
/*	$KAME: destest.c,v 1.3 2000/11/08 05:58:25 itojun Exp $	*/

/*
 * Copyright (C) 2000 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <sys/cdefs.h>
#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <crypto/des/des.h>

#define NUM_TESTS 34
static unsigned char key_data[NUM_TESTS][8]={
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
	{0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11},
	{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF},
	{0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10},
	{0x7C,0xA1,0x10,0x45,0x4A,0x1A,0x6E,0x57},
	{0x01,0x31,0xD9,0x61,0x9D,0xC1,0x37,0x6E},
	{0x07,0xA1,0x13,0x3E,0x4A,0x0B,0x26,0x86},
	{0x38,0x49,0x67,0x4C,0x26,0x02,0x31,0x9E},
	{0x04,0xB9,0x15,0xBA,0x43,0xFE,0xB5,0xB6},
	{0x01,0x13,0xB9,0x70,0xFD,0x34,0xF2,0xCE},
	{0x01,0x70,0xF1,0x75,0x46,0x8F,0xB5,0xE6},
	{0x43,0x29,0x7F,0xAD,0x38,0xE3,0x73,0xFE},
	{0x07,0xA7,0x13,0x70,0x45,0xDA,0x2A,0x16},
	{0x04,0x68,0x91,0x04,0xC2,0xFD,0x3B,0x2F},
	{0x37,0xD0,0x6B,0xB5,0x16,0xCB,0x75,0x46},
	{0x1F,0x08,0x26,0x0D,0x1A,0xC2,0x46,0x5E},
	{0x58,0x40,0x23,0x64,0x1A,0xBA,0x61,0x76},
	{0x02,0x58,0x16,0x16,0x46,0x29,0xB0,0x07},
	{0x49,0x79,0x3E,0xBC,0x79,0xB3,0x25,0x8F},
	{0x4F,0xB0,0x5E,0x15,0x15,0xAB,0x73,0xA7},
	{0x49,0xE9,0x5D,0x6D,0x4C,0xA2,0x29,0xBF},
	{0x01,0x83,0x10,0xDC,0x40,0x9B,0x26,0xD6},
	{0x1C,0x58,0x7F,0x1C,0x13,0x92,0x4F,0xEF},
	{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01},
	{0x1F,0x1F,0x1F,0x1F,0x0E,0x0E,0x0E,0x0E},
	{0xE0,0xFE,0xE0,0xFE,0xF1,0xFE,0xF1,0xFE},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
	{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF},
	{0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10}};

static unsigned char plain_data[NUM_TESTS][8]={
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
	{0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
	{0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11},
	{0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11},
	{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF},
	{0x01,0xA1,0xD6,0xD0,0x39,0x77,0x67,0x42},
	{0x5C,0xD5,0x4C,0xA8,0x3D,0xEF,0x57,0xDA},
	{0x02,0x48,0xD4,0x38,0x06,0xF6,0x71,0x72},
	{0x51,0x45,0x4B,0x58,0x2D,0xDF,0x44,0x0A},
	{0x42,0xFD,0x44,0x30,0x59,0x57,0x7F,0xA2},
	{0x05,0x9B,0x5E,0x08,0x51,0xCF,0x14,0x3A},
	{0x07,0x56,0xD8,0xE0,0x77,0x47,0x61,0xD2},
	{0x76,0x25,0x14,0xB8,0x29,0xBF,0x48,0x6A},
	{0x3B,0xDD,0x11,0x90,0x49,0x37,0x28,0x02},
	{0x26,0x95,0x5F,0x68,0x35,0xAF,0x60,0x9A},
	{0x16,0x4D,0x5E,0x40,0x4F,0x27,0x52,0x32},
	{0x6B,0x05,0x6E,0x18,0x75,0x9F,0x5C,0xCA},
	{0x00,0x4B,0xD6,0xEF,0x09,0x17,0x60,0x62},
	{0x48,0x0D,0x39,0x00,0x6E,0xE7,0x62,0xF2},
	{0x43,0x75,0x40,0xC8,0x69,0x8F,0x3C,0xFA},
	{0x07,0x2D,0x43,0xA0,0x77,0x07,0x52,0x92},
	{0x02,0xFE,0x55,0x77,0x81,0x17,0xF1,0x2A},
	{0x1D,0x9D,0x5C,0x50,0x18,0xF7,0x28,0xC2},
	{0x30,0x55,0x32,0x28,0x6D,0x6F,0x29,0x5A},
	{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF},
	{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF},
	{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF},
	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}};

static unsigned char cipher_data[NUM_TESTS][8]={
	{0x8C,0xA6,0x4D,0xE9,0xC1,0xB1,0x23,0xA7},
	{0x73,0x59,0xB2,0x16,0x3E,0x4E,0xDC,0x58},
	{0x95,0x8E,0x6E,0x62,0x7A,0x05,0x55,0x7B},
	{0xF4,0x03,0x79,0xAB,0x9E,0x0E,0xC5,0x33},
	{0x17,0x66,0x8D,0xFC,0x72,0x92,0x53,0x2D},
	{0x8A,0x5A,0xE1,0xF8,0x1A,0xB8,0xF2,0xDD},
	{0x8C,0xA6,0x4D,0xE9,0xC1,0xB1,0x23,0xA7},
	{0xED,0x39,0xD9,0x50,0xFA,0x74,0xBC,0xC4},
	{0x69,0x0F,0x5B,0x0D,0x9A,0x26,0x93,0x9B},
	{0x7A,0x38,0x9D,0x10,0x35,0x4B,0xD2,0x71},
	{0x86,0x8E,0xBB,0x51,0xCA,0xB4,0x59,0x9A},
	{0x71,0x78,0x87,0x6E,0x01,0xF1,0x9B,0x2A},
	{0xAF,0x37,0xFB,0x42,0x1F,0x8C,0x40,0x95},
	{0x86,0xA5,0x60,0xF1,0x0E,0xC6,0xD8,0x5B},
	{0x0C,0xD3,0xDA,0x02,0x00,0x21,0xDC,0x09},
	{0xEA,0x67,0x6B,0x2C,0xB7,0xDB,0x2B,0x7A},
	{0xDF,0xD6,0x4A,0x81,0x5C,0xAF,0x1A,0x0F},
	{0x5C,0x51,0x3C,0x9C,0x48,0x86,0xC0,0x88},
	{0x0A,0x2A,0xEE,0xAE,0x3F,0xF4,0xAB,0x77},
	{0xEF,0x1B,0xF0,0x3E,0x5D,0xFA,0x57,0x5A},
	{0x88,0xBF,0x0D,0xB6,0xD7,0x0D,0xEE,0x56},
	{0xA1,0xF9,0x91,0x55,0x41,0x02,0x0B,0x56},
	{0x6F,0xBF,0x1C,0xAF,0xCF,0xFD,0x05,0x56},
	{0x2F,0x22,0xE4,0x9B,0xAB,0x7C,0xA1,0xAC},
	{0x5A,0x6B,0x61,0x2C,0xC2,0x6C,0xCE,0x4A},
	{0x5F,0x4C,0x03,0x8E,0xD1,0x2B,0x2E,0x41},
	{0x63,0xFA,0xC0,0xD0,0x34,0xD9,0xF7,0x93},
	{0x61,0x7B,0x3A,0x0C,0xE8,0xF0,0x71,0x00},
	{0xDB,0x95,0x86,0x05,0xF8,0xC8,0xC6,0x06},
	{0xED,0xBF,0xD1,0xC6,0x6C,0x29,0xCC,0xC7},
	{0x35,0x55,0x50,0xB2,0x15,0x0E,0x24,0x51},
	{0xCA,0xAA,0xAF,0x4D,0xEA,0xF1,0xDB,0xAE},
	{0xD5,0xD4,0x4F,0xF7,0x20,0x68,0x3D,0x0D},
	{0x2A,0x2B,0xB0,0x08,0xDF,0x97,0xC2,0xF2}};


static const char *pt(unsigned char *);
int main(int, char **);

static const char *
pt(p)
	unsigned char *p;
{
	static char bufs[10][20];
	static int bnum = 0;
	char *ret;
	int i;

	ret = bufs[bnum++];
	bnum %= 10;
	for (i = 0; i < 8; i++)
		snprintf(&ret[i * 2], 3, "%02x", p[i]);
	ret[8 * 2] = '\0';
	return(ret);
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	des_key_schedule ks;
	des_cblock in, out, outin;
	int i;
	int error = 0;
	int rounds;

	if (argc > 1)
		rounds = atoi(argv[1]);
	else
		rounds = 1;

	printf("Doing DES ecb\n");

again:
	for (i = 0; i < NUM_TESTS - 1; i++) {
		des_set_key(&key_data[i], ks);
		memcpy(in, plain_data[i],8);
		memset(out, 0, 8);
		memset(outin, 0, 8);
		des_ecb_encrypt(&in, &out, ks, DES_ENCRYPT);
		des_ecb_encrypt(&out, &outin, ks, DES_DECRYPT);

		if (memcmp(out, cipher_data[i], 8) != 0) {
			printf("Encryption error %2d\nk=%s p=%s o=%s act=%s\n", 
			    i + 1, pt(key_data[i]), pt(in), pt(cipher_data[i]), 
			    pt(out));
			error = 1;
		}
		if (memcmp(in, outin, 8) != 0) {
			printf("Decryption error %2d\nk=%s p=%s o=%s act=%s\n", 
			    i + 1, pt(key_data[i]), pt(out), pt(in), pt(outin));
			error = 1;
		}
	}

	if (--rounds > 0)
		goto again;

	exit(error);
}
