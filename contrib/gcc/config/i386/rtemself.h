/* Definitions for rtems targeting an ix86 using ELF.
   Copyright (C) 1996, 1997, 2000, 2001, 2002 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (joel@OARcorp.com).

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* Specify predefined symbols in preprocessor.  */

#define TARGET_OS_CPP_BUILTINS()		\
  do						\
    {						\
	builtin_define ("__rtems__");		\
	builtin_define ("__USE_INIT_FINI__");	\
	builtin_assert ("system=rtems");	\
	if (!TARGET_80387)			\
	  builtin_define ("_SOFT_FLOAT");	\
    }						\
  while (0)
