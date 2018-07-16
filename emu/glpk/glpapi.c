/* glpapi.c (obsolete api routines) */

/*----------------------------------------------------------------------
-- This code is part of GNU Linear Programming Kit (GLPK).
--
-- Copyright (C) 2000, 01, 02, 03, 04, 05, 06, 07 Andrew Makhorin,
-- Department for Applied Informatics, Moscow Aviation Institute,
-- Moscow, Russia. All rights reserved. E-mail: <mao@mai2.rcnet.ru>.
--
-- GLPK is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2, or (at your option)
-- any later version.
--
-- GLPK is distributed in the hope that it will be useful, but WITHOUT
-- ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
-- or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
-- License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GLPK; see the file COPYING. If not, write to the Free
-- Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
-- 02110-1301, USA.
----------------------------------------------------------------------*/

#include "glpk.h"
#include "glplib.h"

LPX *lpx_create_prob(void)
{     /* create problem object */
      return glp_create_prob();
}

void lpx_set_prob_name(LPX *lp, const char *name)
{     /* assign (change) problem name */
      glp_set_prob_name(lp, name);
      return;
}

void lpx_set_obj_name(LPX *lp, const char *name)
{     /* assign (change) objective function name */
      glp_set_obj_name(lp, name);
      return;
}

void lpx_set_obj_dir(LPX *lp, int dir)
{     /* set (change) optimization direction flag */
      glp_set_obj_dir(lp, dir - LPX_MIN + GLP_MIN);
      return;
}

int lpx_add_rows(LPX *lp, int nrs)
{     /* add new rows to problem object */
      return glp_add_rows(lp, nrs);
}

int lpx_add_cols(LPX *lp, int ncs)
{     /* add new columns to problem object */
      return glp_add_cols(lp, ncs);
}

void lpx_set_row_name(LPX *lp, int i, const char *name)
{     /* assign (change) row name */
      glp_set_row_name(lp, i, name);
      return;
}

void lpx_set_col_name(LPX *lp, int j, const char *name)
{     /* assign (change) column name */
      glp_set_col_name(lp, j, name);
      return;
}

void lpx_set_row_bnds(LPX *lp, int i, int type, double lb, double ub)
{     /* set (change) row bounds */
      glp_set_row_bnds(lp, i, type - LPX_FR + GLP_FR, lb, ub);
      return;
}

void lpx_set_col_bnds(LPX *lp, int j, int type, double lb, double ub)
{     /* set (change) column bounds */
      glp_set_col_bnds(lp, j, type - LPX_FR + GLP_FR, lb, ub);
      return;
}

void lpx_set_obj_coef(glp_prob *lp, int j, double coef)
{     /* set (change) obj. coefficient or constant term */
      glp_set_obj_coef(lp, j, coef);
      return;
}

void lpx_set_mat_row(LPX *lp, int i, int len, const int ind[],
      const double val[])
{     /* set (replace) row of the constraint matrix */
      glp_set_mat_row(lp, i, len, ind, val);
      return;
}

void lpx_set_mat_col(LPX *lp, int j, int len, const int ind[],
      const double val[])
{     /* set (replace) column of the constraint matrix */
      glp_set_mat_col(lp, j, len, ind, val);
      return;
}

void lpx_load_matrix(LPX *lp, int ne, const int ia[], const int ja[],
      const double ar[])
{     /* load (replace) the whole constraint matrix */
      glp_load_matrix(lp, ne, ia, ja, ar);
      return;
}

void lpx_del_rows(LPX *lp, int nrs, const int num[])
{     /* delete specified rows from problem object */
      glp_del_rows(lp, nrs, num);
      return;
}

void lpx_del_cols(LPX *lp, int ncs, const int num[])
{     /* delete specified columns from problem object */
      glp_del_cols(lp, ncs, num);
      return;
}

void lpx_delete_prob(LPX *lp)
{     /* delete problem object */
      glp_delete_prob(lp);
      return;
}

const char *lpx_get_prob_name(LPX *lp)
{     /* retrieve problem name */
      return glp_get_prob_name(lp);
}

const char *lpx_get_obj_name(LPX *lp)
{     /* retrieve objective function name */
      return glp_get_obj_name(lp);
}

int lpx_get_obj_dir(LPX *lp)
{     /* retrieve optimization direction flag */
      return glp_get_obj_dir(lp) - GLP_MIN + LPX_MIN;
}

int lpx_get_num_rows(LPX *lp)
{     /* retrieve number of rows */
      return glp_get_num_rows(lp);
}

int lpx_get_num_cols(LPX *lp)
{     /* retrieve number of columns */
      return glp_get_num_cols(lp);
}

const char *lpx_get_row_name(LPX *lp, int i)
{     /* retrieve row name */
      return glp_get_row_name(lp, i);
}

const char *lpx_get_col_name(LPX *lp, int j)
{     /* retrieve column name */
      return glp_get_col_name(lp, j);
}

int lpx_get_row_type(LPX *lp, int i)
{     /* retrieve row type */
      return glp_get_row_type(lp, i) - GLP_FR + LPX_FR;
}

int lpx_get_col_type(LPX *lp, int j)
{     /* retrieve column type */
      return glp_get_col_type(lp, j) - GLP_FR + LPX_FR;
}

double lpx_get_obj_coef(LPX *lp, int j)
{     /* retrieve obj. coefficient or constant term */
      return glp_get_obj_coef(lp, j);
}

int lpx_get_num_nz(LPX *lp)
{     /* retrieve number of constraint coefficients */
      return glp_get_num_nz(lp);
}

int lpx_get_mat_row(LPX *lp, int i, int ind[], double val[])
{     /* retrieve row of the constraint matrix */
      return glp_get_mat_row(lp, i, ind, val);
}

int lpx_get_mat_col(LPX *lp, int j, int ind[], double val[])
{     /* retrieve column of the constraint matrix */
      return glp_get_mat_col(lp, j, ind, val);
}

void lpx_create_index(LPX *lp)
{     /* create the name index */
      glp_create_index(lp);
      return;
}

int lpx_find_row(LPX *lp, const char *name)
{     /* find row by its name */
      return glp_find_row(lp, name);
}

int lpx_find_col(LPX *lp, const char *name)
{     /* find column by its name */
      return glp_find_col(lp, name);
}

void lpx_delete_index(LPX *lp)
{     /* delete the name index */
      glp_delete_index(lp);
      return;
}

void lpx_set_row_stat(LPX *lp, int i, int stat)
{     /* set (change) row status */
      glp_set_row_stat(lp, i, stat - LPX_BS + GLP_BS);
      return;
}

void lpx_set_col_stat(LPX *lp, int j, int stat)
{     /* set (change) column status */
      glp_set_col_stat(lp, j, stat - LPX_BS + GLP_BS);
      return;
}

double lpx_get_obj_val(LPX *lp)
{     /* retrieve objective value (basic solution) */
      return glp_get_obj_val(lp);
}

int lpx_get_row_stat(LPX *lp, int i)
{     /* retrieve row status (basic solution) */
      return glp_get_row_stat(lp, i) - GLP_BS + LPX_BS;
}

double lpx_get_row_prim(LPX *lp, int i)
{     /* retrieve row primal value (basic solution) */
      return glp_get_row_prim(lp, i);
}

double lpx_get_row_dual(LPX *lp, int i)
{     /* retrieve row dual value (basic solution) */
      return glp_get_row_dual(lp, i);
}

int lpx_get_col_stat(LPX *lp, int j)
{     /* retrieve column status (basic solution) */
      return glp_get_col_stat(lp, j) - GLP_BS + LPX_BS;
}

double lpx_get_col_prim(LPX *lp, int j)
{     /* retrieve column primal value (basic solution) */
      return glp_get_col_prim(lp, j);
}

double lpx_get_col_dual(glp_prob *lp, int j)
{     /* retrieve column dual value (basic solution) */
      return glp_get_col_dual(lp, j);
}

double lpx_ipt_obj_val(LPX *lp)
{     /* retrieve objective value (interior point) */
      return glp_ipt_obj_val(lp);
}

double lpx_ipt_row_prim(LPX *lp, int i)
{     /* retrieve row primal value (interior point) */
      return glp_ipt_row_prim(lp, i);
}

double lpx_ipt_row_dual(LPX *lp, int i)
{     /* retrieve row dual value (interior point) */
      return glp_ipt_row_dual(lp, i);
}

double lpx_ipt_col_prim(LPX *lp, int j)
{     /* retrieve column primal value (interior point) */
      return glp_ipt_col_prim(lp, j);
}

double lpx_ipt_col_dual(LPX *lp, int j)
{     /* retrieve column dual value (interior point) */
      return glp_ipt_col_dual(lp, j);
}

void lpx_set_class(LPX *lp, int klass)
{     /* set problem class */
      xassert(lp == lp);
      if (!(klass == LPX_LP || klass == LPX_MIP))
         xfault("lpx_set_class: invalid problem class\n");
      return;
}

int lpx_get_class(LPX *lp)
{     /* determine problem klass */
      return glp_get_num_int(lp) == 0 ? LPX_LP : LPX_MIP;
}

void lpx_set_col_kind(LPX *lp, int j, int kind)
{     /* set (change) column kind */
      glp_set_col_kind(lp, j, kind - LPX_CV + GLP_CV);
      return;
}

int lpx_get_col_kind(LPX *lp, int j)
{     /* retrieve column kind */
      return glp_get_col_kind(lp, j) == GLP_CV ? LPX_CV : LPX_IV;
}

int lpx_get_num_int(LPX *lp)
{     /* retrieve number of integer columns */
      return glp_get_num_int(lp);
}

int lpx_get_num_bin(LPX *lp)
{     /* retrieve number of binary columns */
      return glp_get_num_bin(lp);
}

double lpx_mip_obj_val(LPX *lp)
{     /* retrieve objective value (MIP solution) */
      return glp_mip_obj_val(lp);
}

double lpx_mip_row_val(LPX *lp, int i)
{     /* retrieve row value (MIP solution) */
      return glp_mip_row_val(lp, i);
}

double lpx_mip_col_val(LPX *lp, int j)
{     /* retrieve column value (MIP solution) */
      return glp_mip_col_val(lp, j);
}

/* eof */
