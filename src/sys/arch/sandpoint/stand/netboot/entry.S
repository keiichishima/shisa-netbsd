/* $NetBSD: entry.S,v 1.3 2007/11/05 13:41:48 nisimura Exp $ */

#include <powerpc/spr.h>
#include <powerpc/oea/hid.h>

	.text
	.globl _start
_start:
	mfspr	11,SPR_HID0
	andi.	0,11,HID0_DCE
	ori	11,11,HID0_ICE|HID0_DCE
	ori	8,11,HID0_ICFI
	bne	1f			/* don't invalidate the D-cache */
	ori	8,8,HID0_DCFI		/* unless it wasn't enabled */
1:
	sync
	mtspr	SPR_HID0,8		/* enable and invalidate caches */
	sync
	mtspr	SPR_HID0,11		/* enable caches */
	sync
	isync

	/* make sure .bss gets zeroed. */
	li	0,0
	lis	8,edata@ha
	addi	8,8,edata@l
	lis	9,end@ha
	addi	9,9,end@l
5:	cmpw	0,8,9			/* edata & end are >= word aligned */
	bge	6f
	stw	0,0(8)
	addi	8,8,4
	b	5b

6:
	/* prepare stack at +1MB from _start. */
	lis	1,_start@h
	ori	1,1,_start@l
	addis	1,1,0x10
	addi	1,1,-4

	bl	brdsetup
	bl	main

hang:	b	hang
	/* NOTREACHED */

/*
 * run(startsym, endsym, howto, bootinfo, entry)
 */
	.globl	run
run:
	mtctr	7 	/* hat trick jump to entry point */
	bctr

/*
 * reverse endian access to mimic outw/outl/inw/inl
 */
	.globl out16rb
	.globl iohtole16
out16rb:
iohtole16:
	sthbrx	4,0,3
	eieio
	blr 

	.globl out32rb
	.globl iohtole32
out32rb:
iohtole32:
	stwbrx	4,0,3
	eieio
	blr

	.global in16rb
	.global iole16toh
in16rb:
iole16toh:
	lhbrx	3,0,3
	eieio
	blr

	.global in32rb
	.global iole32toh
in32rb:
iole32toh:
	lwbrx	3,0,3
	eieio
	blr
