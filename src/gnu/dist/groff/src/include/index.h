/*	$NetBSD: index.h,v 1.1.1.1 2001/04/19 12:50:45 wiz Exp $	*/

// -*- C++ -*-
/* Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com)

This file is part of groff.

groff is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

groff is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with groff; see the file COPYING.  If not, write to the Free Software
Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#define INDEX_MAGIC 0x23021964
#define INDEX_VERSION 1

struct index_header {
  int magic;
  int version;
  int tags_size;
  int table_size;
  int lists_size;
  int strings_size;
  int truncate;
  int shortest;
  int common;
};

struct tag {
  int filename_index;
  int start;
  int length;
};

unsigned hash(const char *s, int len);
