/*	$NetBSD: btuart.h,v 1.1 2007/02/20 16:53:21 kiyohara Exp $	*/
/*
 * Copyright (c) 2006 KIYOHARA Takashi
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DEV_BLUETOOTH_BTUART_H_
#define _DEV_BLUETOOTH_BTUART_H_

#define BTUART_HCITYPE		_IOW ('H', 1, uint32_t)
#define BTUART_HCITYPE_ANY		0
#define BTUART_HCITYPE_ERICSSON		1
#define BTUART_HCITYPE_DIGI		2
#define BTUART_HCITYPE_TEXAS		3
#define BTUART_HCITYPE_CSR		4
#define BTUART_HCITYPE_SWAVE		5
#define BTUART_HCITYPE_ST		6
#define BTUART_HCITYPE_STLC2500		7
#define BTUART_HCITYPE_BT2000C		8
#define BTUART_HCITYPE_BCM2035		9

#define BTUART_INITSPEED	_IOW ('H', 2, uint32_t)
#define BTUART_START		_IO  ('H', 3)

#endif	/* _DEV_BLUETOOTH_BTUART_H_ */
