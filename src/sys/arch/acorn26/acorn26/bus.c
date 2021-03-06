/* $NetBSD: bus.c,v 1.4 2003/07/14 22:48:20 lukem Exp $ */
/*-
 * Copyright (c) 1999, 2000 Ben Harris
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * bus.c - bus space functions for Archimedes I/O bus
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: bus.c,v 1.4 2003/07/14 22:48:20 lukem Exp $");

#include <sys/param.h>
#include <machine/bus.h>
#include <machine/memcreg.h>

#include <arm/blockio.h>

int
bus_space_map(bus_space_tag_t bst, bus_addr_t addr, bus_size_t size,
	      int flags, bus_space_handle_t *bshp)
{

	if (flags & BUS_SPACE_MAP_LINEAR)
		return -1;
	bshp->a1 = bshp->a2 = addr;
	return 0;
}

int
bus_space_subregion(bus_space_tag_t bst, bus_space_handle_t bsh,
		    bus_size_t offset, bus_size_t size,
		    bus_space_handle_t *nbshp)
{

	nbshp->a1 = bsh.a1 + (offset << bst);
	nbshp->a2 = bsh.a2 + (offset << bst);
	return 0;
}

int
bus_space_shift(bus_space_tag_t bst, bus_space_handle_t bsh, int shift,
		bus_space_tag_t *nbstp, bus_space_handle_t *nbshp)
{

	*nbstp = shift;
	*nbshp = bsh;
	return 0;
}

void
bus_space_read_multi_2(bus_space_tag_t bst, bus_space_handle_t bsh,
		       bus_size_t offset, u_int16_t *datap, bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		datap[i] = bus_space_read_2(bst, bsh, offset);
}

void
bus_space_write_multi_2(bus_space_tag_t bst, bus_space_handle_t bsh,
			bus_size_t offset, u_int16_t const *datap,
			bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		bus_space_write_2(bst, bsh, offset, datap[i]);
}

void
bus_space_set_multi_1(bus_space_tag_t bst, bus_space_handle_t bsh,
		      bus_size_t offset,  u_int8_t value, bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		bus_space_write_1(bst, bsh, offset, value);
}

void
bus_space_set_multi_2(bus_space_tag_t bst, bus_space_handle_t bsh,
		      bus_size_t offset, u_int16_t value, bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		bus_space_write_2(bst, bsh, offset, value);
}

void
bus_space_read_region_1(bus_space_tag_t bst, bus_space_handle_t bsh,
			bus_size_t offset, u_int8_t *datap, bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		datap[i] = bus_space_read_1(bst, bsh, offset + i);
}

void
bus_space_read_region_2(bus_space_tag_t bst, bus_space_handle_t bsh,
			bus_size_t offset, u_int16_t *datap, bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		datap[i] = bus_space_read_2(bst, bsh, offset + i);
}

void
bus_space_write_region_1(bus_space_tag_t bst, bus_space_handle_t bsh,
			 bus_size_t offset, u_int8_t const *datap,
			 bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		bus_space_write_1(bst, bsh, offset + i, datap[i]);
}

void
bus_space_write_region_2(bus_space_tag_t bst, bus_space_handle_t bsh,
			 bus_size_t offset, u_int16_t const *datap,
			 bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		bus_space_write_2(bst, bsh, offset + i, datap[i]);
}

void
bus_space_set_region_1(bus_space_tag_t bst, bus_space_handle_t bsh,
		       bus_size_t offset, u_int8_t value, bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		bus_space_write_1(bst, bsh, offset + i, value);
}

void
bus_space_set_region_2(bus_space_tag_t bst, bus_space_handle_t bsh,
		       bus_size_t offset, u_int16_t value, bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		bus_space_write_2(bst, bsh, offset + i, value);
}

void
bus_space_copy_region_1(bus_space_tag_t bst,
		 bus_space_handle_t bsh1, bus_size_t offset1,
		 bus_space_handle_t bsh2, bus_size_t offset2, bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		bus_space_write_1(bst, bsh2, offset2 + i,
				  bus_space_read_1(bst, bsh1, offset1 + i));
}

void
bus_space_copy_region_2(bus_space_tag_t bst,
		 bus_space_handle_t bsh1, bus_size_t offset1,
		 bus_space_handle_t bsh2, bus_size_t offset2, bus_size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		bus_space_write_2(bst, bsh2, offset2 + i,
				  bus_space_read_2(bst, bsh1, offset1 + i));
}



