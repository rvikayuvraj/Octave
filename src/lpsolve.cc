// f-lpsolve.cc                                          -*- C++ -*-
/*

Copyright (C) 1993, 1994, 1995 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "LPsolve.h"

#include "tree-const.h"
#include "error.h"
#include "help.h"
#include "defun-dld.h"

DEFUN_DLD_BUILTIN ("lp_solve", Flp_solve, Slp_solve, 11, 3,
  "lp_solve (): solve linear programs using lp_solve.")
{
  Octave_object retval;

// Force a bad value of inform, and empty matrices for x and phi.
  Matrix m;
  retval(2) = -1.0;
  retval(1) = m;
  retval(0) = m;

  error ("lp_solve: not implemented yet");

  return retval;
}

DEFUN_DLD_BUILTIN ("lp_solve_options", Flp_solve_options,
		   Slp_solve_options, -1, 1, 
  "lp_solve_options (KEYWORD, VALUE)\n\
\n\
Set or show options for lp_solve.  Keywords may be abbreviated\n\
to the shortest match.")
{
  Octave_object retval;

  error ("lp_solve_options: not implemented yet");

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
