// f-find.cc                                           -*- C++ -*-
/*

Copyright (C) 1994, 1995 John W. Eaton

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

#include "tree-const.h"
#include "error.h"
#include "gripes.h"
#include "help.h"
#include "defun-dld.h"

static Octave_object
find_to_fortran_idx (const ColumnVector i_idx, const ColumnVector j_idx,
		     const tree_constant& val, int nr, int nc, int nargout)
{
  Octave_object retval;

  switch (nargout)
    {
    case 0:
    case 1:
      {
	int count = i_idx.length ();
	ColumnVector tmp (count);
	for (int i = 0; i < count; i++)
	  tmp (i) = nr * (j_idx (i) - 1.0) + i_idx (i);
	retval(0) = tree_constant (tmp, 1);
// If you want this to work more like Matlab, use the following line
// instead of the previous one.
//	retval(0) = tree_constant (tmp, (nr != 1));
      }
      break;

    case 3:
      retval(2) = val;
// Fall through!

    case 2:
      retval(1) = tree_constant (j_idx, 1);
      retval(0) = tree_constant (i_idx, 1);
// If you want this to work more like Matlab, use the following line
// instead of the previous one.
//    retval(0) = tree_constant (i_idx, (nr != 1));
      break;

    default:
      panic_impossible ();
      break;
    }

  return retval;
}

static Octave_object
find_nonzero_elem_idx (const Matrix& m, int nargout)
{
  int count = 0;
  int m_nr = m.rows ();
  int m_nc = m.columns ();

  int i, j;
  for (j = 0; j < m_nc; j++)
    for (i = 0; i < m_nr; i++)
      if (m.elem (i, j) != 0.0)
	count++;

  Octave_object retval (((nargout == 0) ? 1 : nargout), Matrix ());

  if (count == 0)
    return retval;

  ColumnVector i_idx (count);
  ColumnVector j_idx (count);
  ColumnVector v (count);

  count = 0;
  for (j = 0; j < m_nc; j++)
    for (i = 0; i < m_nr; i++)
      {
	double d = m.elem (i, j);
	if (d != 0.0)
	  {
	    i_idx (count) = i + 1;
	    j_idx (count) = j + 1;
	    v (count) = d;
	    count++;
	  }
      }

  tree_constant tmp (v, 1);
  return find_to_fortran_idx (i_idx, j_idx, tmp, m_nr, m_nc, nargout);
}

static Octave_object
find_nonzero_elem_idx (const ComplexMatrix& m, int nargout)
{
  int count = 0;
  int m_nr = m.rows ();
  int m_nc = m.columns ();

  int i, j;
  for (j = 0; j < m_nc; j++)
    for (i = 0; i < m_nr; i++)
      if (m.elem (i, j) != 0.0)
	count++;

  Octave_object retval (((nargout == 0) ? 1 : nargout), Matrix ());

  if (count == 0)
    return retval;

  ColumnVector i_idx (count);
  ColumnVector j_idx (count);
  ComplexColumnVector v (count);

  count = 0;
  for (j = 0; j < m_nc; j++)
    for (i = 0; i < m_nr; i++)
      {
	Complex c = m.elem (i, j);
	if (c != 0.0)
	  {
	    i_idx (count) = i;
	    j_idx (count) = j;
	    v (count) = c;
	    count++;
	  }
      }

  tree_constant tmp (v, 1);
  return find_to_fortran_idx (i_idx, j_idx, tmp, m_nr, m_nc, nargout);
}

DEFUN_DLD_BUILTIN ("find", Ffind, Sfind, 2, 3,
  "find (X) or [I, J, V] = find (X): Return indices of nonzero elements")
{
  Octave_object retval;

  int nargin = args.length ();

  if (nargin != 1 || nargout > 3)
    {
      print_usage ("find");
      return retval;
    }

  tree_constant arg = args(0);

  if (arg.is_real_type ())
    {
      Matrix m = arg.matrix_value ();

      if (! error_state)
	retval = find_nonzero_elem_idx (m, nargout);
    }
  else if (arg.is_complex_type ())
    {
      ComplexMatrix m = arg.complex_matrix_value ();

      if (! error_state)
	retval = find_nonzero_elem_idx (m, nargout);
    }
  else
    {
      gripe_wrong_type_arg ("find", arg);
    }

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
