/* Writing C# satellite assemblies.
   Copyright (C) 2003 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _WRITE_CSHARP_H
#define _WRITE_CSHARP_H

#include "message.h"

/* Write a C# satellite assembly DLL file.  mlp is a list containing the
   messages to be output.  resource_name is the name of the resource
   (with dot separators), locale_name is the locale name (with underscore
   separators) or NULL, directory is the base directory.
   Return 0 if ok, nonzero on error.  */
extern int
       msgdomain_write_csharp (message_list_ty *mlp,
			       const char *canon_encoding,
			       const char *resource_name,
			       const char *locale_name,
			       const char *directory);

#endif /* _WRITE_CSHARP_H */
