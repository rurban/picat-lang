/* glplpx14.c (additional utility routines) */

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

#include "glpapi.h"
#include "glpk.h"
#include "glpavl.h"
#include "glplib.h"
#define print xprint1
#define fault xfault1

/*----------------------------------------------------------------------
-- lpx_read_mps - read problem data in fixed MPS format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- LPX *lpx_read_mps(char *fname);
--
-- *Description*
--
-- The routine lpx_read_mps reads LP/MIP problem data in fixed MPS
-- format from an input text file whose name is the character string
-- fname.
--
-- *Returns*
--
-- If no error occurred, the routine returns a pointer to the created
-- problem object. Otherwise the routine returns NULL. */

struct dsa
{     /* working area used by MPS routines */
      LPX *lp;
      /* LP/MIP problem object */
      const char *fname;
      /* name of input text file */
      FILE *fp;
      /* stream assigned to input text file */
      int count;
      /* card count */
      char card[80+1];
      /* card image */
      char f1[2+1], f2[8+1], f3[8+1], f4[12+1], f5[8+1], f6[12+1];
      /* data card fields */
      int obj;
      /* ordinal number of free (unbounded) row used as the objective
         function */
};

static int read_card(struct dsa *dsa)
{     /* read image of 80-column punched card from input file */
      int j, c;
loop: dsa->count++;
      memset(dsa->card, ' ', 80), dsa->card[80] = '\0';
      j = 0;
      for (;;)
      {  c = fgetc(dsa->fp);
         if (ferror(dsa->fp))
         {  print("%s:%d: read error - %s", dsa->fname, dsa->count,
               strerror(errno));
            goto fail;
         }
         if (feof(dsa->fp))
         {  if (j == 0)
               print("%s:%d: unexpected EOF", dsa->fname, dsa->count);
            else
               print("%s:%d: missing final NL", dsa->fname, dsa->count);
            goto fail;
         }
         if (c == '\r') continue;
         if (c == '\n') break;
         if (iscntrl(c))
         {  print("%s:%d: invalid control character 0x%02X", dsa->fname,
               dsa->count, c);
            goto fail;
         }
         if (j == 80)
         {  print("%s:%d: card image exceeds 80 chars", dsa->fname,
               dsa->count);
            goto fail;
         }
         dsa->card[j++] = (char)c;
      }
      /* asterisk in the first column begins a comment */
      if (dsa->card[0] == '*') goto loop;
      return 0;
fail: return 1;
}

static int blank = '_';
/* character code to replace blanks in symbolic names; it may be any
   printable ASCII character (including blank) or '\0', in which case
   all blanks are simply removed */

static void adjust_name(char name[8+1])
{     /* adjust symbolic name by replacing or removing blanks */
      if (blank == '\0')
      {  /* remove all blanks */
         strspx(name);
      }
      else
      {  int k;
         /* remove trailing blanks */
         strtrim(name);
         /* and replace remaining blanks by given character */
         for (k = 0; name[k] != '\0'; k++)
            if (name[k] == ' ') name[k] = (char)blank;
      }
      return;
}

static int split_card(struct dsa *dsa)
{     /* split data card into six fields */
      /* column 1 must be blank */
      xassert(dsa->card[0] == ' ');
      /* scan field 1 (code) in columns 2-3 */
      memcpy(dsa->f1, dsa->card+1, 2);
      dsa->f1[2] = '\0'; strspx(dsa->f1);
      /* column 4 must be blank */
      if (dsa->card[3] != ' ')
      {  print("%s:%d: invalid data card; column 4 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
      /* scan field 2 (name) in columns 5-12 */
      memcpy(dsa->f2, dsa->card+4, 8);
      dsa->f2[8] = '\0'; adjust_name(dsa->f2);
      /* columns 13-14 must be blank */
      if (memcmp(dsa->card+12, "  ", 2) != 0)
      {  print("%s:%d: invalid data card; columns 13-14 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
      /* dollar sign in column 15 begins a comment */
      if (dsa->card[14] == '$')
      {  dsa->f3[0] = dsa->f4[0] = dsa->f5[0] = dsa->f6[0] = '\0';
         goto done;
      }
      /* scan field 3 (name) in columns 15-22 */
      memcpy(dsa->f3, dsa->card+14, 8);
      dsa->f3[8] = '\0'; adjust_name(dsa->f3);
      /* columns 23-24 must be blank */
      if (memcmp(dsa->card+22, "  ", 2) != 0)
      {  print("%s:%d: invalid data card; columns 23-24 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
      /* scan field 4 (number) in columns 25-36 */
      memcpy(dsa->f4, dsa->card+24, 12);
      dsa->f4[12] = '\0'; strspx(dsa->f4);
      /* columns 37-39 must be blank */
      if (memcmp(dsa->card+36, "   ", 3) != 0)
      {  print("%s:%d: invalid data card; columns 37-39 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
      /* dollar sign in column 40 begins a comment */
      if (dsa->card[39] == '$')
      {  dsa->f5[0] = dsa->f6[0] = '\0';
         goto done;
      }
      /* scan field 5 (name) in columns 40-47 */
      memcpy(dsa->f5, dsa->card+39, 8);
      dsa->f5[8] = '\0'; adjust_name(dsa->f5);
      /* columns 48-49 must be blank */
      if (memcmp(dsa->card+47, "  ", 2) != 0)
      {  print("%s:%d: invalid data card; columns 48-49 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
      /* scan field 6 (number) in columns 50-61 */
      memcpy(dsa->f6, dsa->card+49, 12);
      dsa->f6[12] = '\0'; strspx(dsa->f6);
      /* columns 62-72 must be blank */
      if (memcmp(dsa->card+61, "           ", 11) != 0)
      {  print("%s:%d: invalid data card; columns 62-72 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
done: return 0;
fail: return 1;
}

static int read_rows(struct dsa *dsa)
{     /* process data cards in ROWS section */
      int i, type;
loop: /* read and split next data card */
      if (read_card(dsa)) goto fail;
      if (dsa->card[0] != ' ') goto done;
      if (split_card(dsa)) goto fail;
      /* create new row */
      i = lpx_add_rows(dsa->lp, 1);
      /* scan row type in field 1 */
      if (dsa->f1[0] == '\0')
      {  print("%s:%d: missing row type in field 1", dsa->fname,
            dsa->count);
         goto fail;
      }
      else if (strcmp(dsa->f1, "N") == 0)
      {  type = LPX_FR;
         /* the first free (unbounded) row is used as the objective
            function */
         if (dsa->obj == 0) dsa->obj = i;
      }
      else if (strcmp(dsa->f1, "L") == 0)
         type = LPX_UP;
      else if (strcmp(dsa->f1, "G") == 0)
         type = LPX_LO;
      else if (strcmp(dsa->f1, "E") == 0)
         type = LPX_FX;
      else
      {  print("%s:%d: invalid row type in field 1", dsa->fname,
            dsa->count);
         goto fail;
      }
      lpx_set_row_bnds(dsa->lp, i, type, 0.0, 0.0);
      /* scan row name in field 2 */
      if (dsa->f2[0] == '\0')
      {  print("%s:%d: missing row name in field 2", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (lpx_find_row(dsa->lp, dsa->f2) != 0)
      {  print("%s:%d: row %s multiply specified", dsa->fname,
            dsa->count);
         goto fail;
      }
      lpx_set_row_name(dsa->lp, i, dsa->f2);
      if (dsa->obj == i) lpx_set_obj_name(dsa->lp, dsa->f2);
      /* fields 3-6 must be blank */
      if (!(dsa->f3[0] == '\0' && dsa->f4[0] == '\0' &&
            dsa->f5[0] == '\0' && dsa->f6[0] == '\0'))
      {  print("%s:%d: invalid data card; fields 3-6 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
      goto loop;
done: return 0;
fail: return 1;
}

static void put_column(struct dsa *dsa, int j, int len, int ind[],
      double aij[])
{     /* store j-th column to the constraint matrix */
      int k, newlen = 0;
      for (k = 1; k <= len; k++)
      {  if (aij[k] != 0.0)
         {  newlen++;
            ind[newlen] = ind[k];
            aij[newlen] = aij[k];
            if (dsa->obj == ind[newlen])
               lpx_set_obj_coef(dsa->lp, j, aij[newlen]);
         }
      }
      lpx_set_mat_col(dsa->lp, j, newlen, ind, aij);
      return;
}

static int read_columns(struct dsa *dsa)
{     /* process data cards in COLUMNS section */
      int i, j = 0, k, len = 0, m, marker = 0, *map, *ind;
      double *aij, temp;
      /* allocate working arrays */
      m = lpx_get_num_rows(dsa->lp);
      map = xcalloc(1+m, sizeof(int));
      for (i = 1; i <= m; i++) map[i] = 0;
      ind = xcalloc(1+m, sizeof(int));
      aij = xcalloc(1+m, sizeof(double));
loop: /* read and split next data card */
      if (read_card(dsa)) goto fail;
      if (dsa->card[0] != ' ') goto done;
      if (split_card(dsa)) goto fail;
      /* field 1 must be blank */
      if (dsa->f1[0] != '\0')
      {  print("%s:%d: invalid data card; field 1 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
      /* scan optional INTORG/INTEND marker */
      if (strcmp(dsa->f3, "'MARKER'") == 0)
      {  /* fields 4 and 6 must be blank */
         if (!(dsa->f4[0] == '\0' && dsa->f6[0] == '\0'))
         {  print("%s:%d: invalid data card; fields 4 and 6 must be bla"
               "nk", dsa->fname, dsa->count);
            goto fail;
         }
         /* scan marker name in field 2 */
         if (dsa->f2[0] == '\0')
         {  print("%s:%d: missing marker name in field 2", dsa->fname,
               dsa->count);
            goto fail;
         }
         /* scan marker type in field 5 */
         if (dsa->f5[0] == '\0')
         {  print("%s:%d: missing marker type in field 5", dsa->fname,
               dsa->count);
            goto fail;
         }
         else if (strcmp(dsa->f5, "'INTORG'") == 0)
            marker = 1;
         else if (strcmp(dsa->f5, "'INTEND'") == 0)
            marker = 0;
         else
         {  print("%s:%d: invalid marker type in field 5", dsa->fname,
               dsa->count);
            goto fail;
         }
         goto loop;
      }
      /* scan column name in field 2 */
      if (dsa->f2[0] == '\0')
      {  if (j == 0)
         {  print("%s:%d: missing column name in field 2", dsa->fname,
               dsa->count);
            goto fail;
         }
         /* still the current column */
         goto skip;
      }
      if (j != 0 && strcmp(lpx_get_col_name(dsa->lp, j), dsa->f2) == 0)
      {  /* still the current column */
         goto skip;
      }
      /* new column begins, so finish the current column */
      if (j != 0)
      {  for (k = 1; k <= len; k++) map[ind[k]] = 0;
         put_column(dsa, j, len, ind, aij), len = 0;
      }
      /* create new column */
      if (lpx_find_col(dsa->lp, dsa->f2) != 0)
      {  print("%s:%d: column %s multiply specified", dsa->fname,
            dsa->count, dsa->f2);
         goto fail;
      }
      j = lpx_add_cols(dsa->lp, 1);
      lpx_set_col_name(dsa->lp, j, dsa->f2);
      if (marker)
      {  /* lpx_set_class(dsa->lp, LPX_MIP); */
         lpx_set_col_kind(dsa->lp, j, LPX_IV);
      }
      lpx_set_col_bnds(dsa->lp, j, LPX_LO, 0.0, 0.0);
skip: /* scan row name and coefficient in fields 3 and 4 */
      if (dsa->f3[0] == '\0')
      {  print("%s:%d: missing row name in field 3", dsa->fname,
            dsa->count);
         goto fail;
      }
      i = lpx_find_row(dsa->lp, dsa->f3);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            dsa->f3);
         goto fail;
      }
      if (dsa->f4[0] == '\0')
      {  print("%s:%d: missing coefficient value in field 4",
            dsa->fname, dsa->count);
         goto fail;
      }
      if (str2num(dsa->f4, &temp))
      {  print("%s:%d: invalid coefficient value in field 4",
            dsa->fname, dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: coefficient in row %s multiply specified",
            dsa->fname, dsa->count, dsa->f3);
         goto fail;
      }
      map[i] = 1, len++, ind[len] = i, aij[len] = temp;
      /* scan optional row name and coefficient in fields 5 and 6 */
      if (dsa->f5[0] == '\0' && dsa->f6[0] == '\0') goto loop;
      if (dsa->f5[0] == '\0')
      {  print("%s:%d: missing row name in field 5", dsa->fname,
            dsa->count);
         goto fail;
      }
      i = lpx_find_row(dsa->lp, dsa->f5);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            dsa->f5);
         goto fail;
      }
      if (dsa->f6[0] == '\0')
      {  print("%s:%d: missing coefficient value in field 6",
            dsa->fname, dsa->count);
         goto fail;
      }
      if (str2num(dsa->f6, &temp))
      {  print("%s:%d: invalid coefficient value in field 6",
            dsa->fname, dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: coefficient in row %s multiply specified",
            dsa->fname, dsa->count, dsa->f5);
         goto fail;
      }
      map[i] = 1, len++, ind[len] = i, aij[len] = temp;
      goto loop;
done: /* finish the last column */
      if (j != 0) put_column(dsa, j, len, ind, aij);
      xfree(map), xfree(ind), xfree(aij);
      return 0;
fail: xfree(map), xfree(ind), xfree(aij);
      return 1;
}

static void set_rhs(struct dsa *dsa, int i, double b)
{     /* set right-hand side for i-th row */
      switch (lpx_get_row_type(dsa->lp, i))
      {  case LPX_FR:
            if (dsa->obj == i)
               lpx_set_obj_coef(dsa->lp, 0, b);
            else if (b != 0.0)
               print("%s:%d: warning: non-zero rhs for free row %s igno"
                  "red", dsa->fname, dsa->count, lpx_get_row_name(dsa->
                  lp, i));
            break;
         case LPX_LO:
            lpx_set_row_bnds(dsa->lp, i, LPX_LO, b, 0.0);
            break;
         case LPX_UP:
            lpx_set_row_bnds(dsa->lp, i, LPX_UP, 0.0, b);
            break;
         case LPX_FX:
            lpx_set_row_bnds(dsa->lp, i, LPX_FX, b, b);
            break;
         default:
            xassert(dsa != dsa);
      }
      return;
}

static int read_rhs(struct dsa *dsa)
{     /* process data cards in RHS section */
      int i, m, *map;
      double temp;
      char name[8+1] = "\n";
      /* allocate working array */
      m = lpx_get_num_rows(dsa->lp);
      map = xcalloc(1+m, sizeof(int));
      for (i = 1; i <= m; i++) map[i] = 0;
loop: /* read and split next data card */
      if (read_card(dsa)) goto fail;
      if (dsa->card[0] != ' ') goto done;
      if (split_card(dsa)) goto fail;
      /* field 1 must be blank */
      if (dsa->f1[0] != '\0')
      {  print("%s:%d: invalid data card; field 1 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
      /* scan optional rhs vector name in field 2 */
      if (name[0] == '\n') strcpy(name, dsa->f2);
      if (!(dsa->f2[0] == '\0' || strcmp(dsa->f2, name) == 0))
      {  print("%s:%d: at most one rhs vector allowed", dsa->fname,
            dsa->count);
         goto fail;
      }
      /* scan row name and rhs value in fields 3 and 4 */
      if (dsa->f3[0] == '\0')
      {  print("%s:%d: missing row name in field 3", dsa->fname,
            dsa->count);
         goto fail;
      }
      i = lpx_find_row(dsa->lp, dsa->f3);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            dsa->f3);
         goto fail;
      }
      if (dsa->f4[0] == '\0')
      {  print("%s:%d: missing rhs value in field 4", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (str2num(dsa->f4, &temp))
      {  print("%s:%d: invalid rhs value in field 4", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: rhs for row %s multiply specified", dsa->fname,
            dsa->count, dsa->f3);
         goto fail;
      }
      set_rhs(dsa, i, temp), map[i] = 1;
      /* scan optional row name and rhs value in fields 5 and 6 */
      if (dsa->f5[0] == '\0' && dsa->f6[0] == '\0') goto loop;
      if (dsa->f5[0] == '\0')
      {  print("%s:%d: missing row name in field 5", dsa->fname,
            dsa->count);
         goto fail;
      }
      i = lpx_find_row(dsa->lp, dsa->f5);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            dsa->f5);
         goto fail;
      }
      if (dsa->f6[0] == '\0')
      {  print("%s:%d: missing rhs value in field 6", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (str2num(dsa->f6, &temp))
      {  print("%s:%d: invalid rhs value in field 6", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: rhs for row %s multiply specified", dsa->fname,
            dsa->count, dsa->f5);
         goto fail;
      }
      set_rhs(dsa, i, temp), map[i] = 1;
      goto loop;
done: xfree(map);
      return 0;
fail: xfree(map);
      return 1;
}

static void set_range(struct dsa *dsa, int i, double r)
{     /* set range value for i-th row */
      double b;
      switch (lpx_get_row_type(dsa->lp, i))
      {  case LPX_FR:
            print("%s:%d: warning: range value for free row %s ignored",
               dsa->fname, dsa->count, lpx_get_row_name(dsa->lp, i));
            break;
         case LPX_LO:
            b = lpx_get_row_lb(dsa->lp, i);
            if (r == 0.0)
               lpx_set_row_bnds(dsa->lp, i, LPX_FX, b, b);
            else
               lpx_set_row_bnds(dsa->lp, i, LPX_DB, b, b + fabs(r));
            break;
         case LPX_UP:
            b = lpx_get_row_ub(dsa->lp, i);
            if (r == 0.0)
               lpx_set_row_bnds(dsa->lp, i, LPX_FX, b, b);
            else
               lpx_set_row_bnds(dsa->lp, i, LPX_DB, b - fabs(r), b);
            break;
         case LPX_FX:
            b = lpx_get_row_lb(dsa->lp, i);
            if (r > 0.0)
               lpx_set_row_bnds(dsa->lp, i, LPX_DB, b, b + fabs(r));
            else if (r < 0.0)
               lpx_set_row_bnds(dsa->lp, i, LPX_DB, b - fabs(r), b);
            break;
         default:
            xassert(dsa != dsa);
      }
      return;
}

static int read_ranges(struct dsa *dsa)
{     /* process data cards in RANGES section */
      int i, m, *map;
      double temp;
      char name[8+1] = "\n";
      /* allocate working array */
      m = lpx_get_num_rows(dsa->lp);
      map = xcalloc(1+m, sizeof(int));
      for (i = 1; i <= m; i++) map[i] = 0;
loop: /* read and split next data card */
      if (read_card(dsa)) goto fail;
      if (dsa->card[0] != ' ') goto done;
      if (split_card(dsa)) goto fail;
      /* field 1 must be blank */
      if (dsa->f1[0] != '\0')
      {  print("%s:%d: invalid data card; field 1 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
      /* scan optional ranges vector name in field 2 */
      if (name[0] == '\n') strcpy(name, dsa->f2);
      if (!(dsa->f2[0] == '\0' || strcmp(dsa->f2, name) == 0))
      {  print("%s:%d: at most one ranges vector allowed", dsa->fname,
            dsa->count);
         goto fail;
      }
      /* scan row name and range value in fields 3 and 4 */
      if (dsa->f3[0] == '\0')
      {  print("%s:%d: missing row name in field 3", dsa->fname,
            dsa->count);
         goto fail;
      }
      i = lpx_find_row(dsa->lp, dsa->f3);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            dsa->f3);
         goto fail;
      }
      if (dsa->f4[0] == '\0')
      {  print("%s:%d: missing range value in field 4", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (str2num(dsa->f4, &temp))
      {  print("%s:%d: invalid range value in field 4", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: range for row %s multiply specified", dsa->fname,
            dsa->count, dsa->f3);
         goto fail;
      }
      set_range(dsa, i, temp), map[i] = 1;
      /* scan optional row name and range value in fields 5 and 6 */
      if (dsa->f5[0] == '\0' && dsa->f6[0] == '\0') goto loop;
      if (dsa->f5[0] == '\0')
      {  print("%s:%d: missing row name in field 5", dsa->fname,
            dsa->count);
         goto fail;
      }
      i = lpx_find_row(dsa->lp, dsa->f5);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            dsa->f5);
         goto fail;
      }
      if (dsa->f6[0] == '\0')
      {  print("%s:%d: missing range value in field 6", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (str2num(dsa->f6, &temp))
      {  print("%s:%d: invalid range value in field 6", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: range for row %s multiply specified", dsa->fname,
            dsa->count, dsa->f5);
         goto fail;
      }
      set_range(dsa, i, temp), map[i] = 1;
      goto loop;
done: xfree(map);
      return 0;
fail: xfree(map);
      return 1;
}

static int read_bounds(struct dsa *dsa)
{     /* process data cards in BOUNDS section */
      int j, type;
      double lb, ub, temp;
      char name[8+1] = "\n";
loop: /* read and split next data card */
      if (read_card(dsa)) goto fail;
      if (dsa->card[0] != ' ') goto done;
      if (split_card(dsa)) goto fail;
      /* scan bound type in field 1 */
      if (dsa->f1[0] == '\0')
      {  print("%s:%d: missing bound type in field 1", dsa->fname,
            dsa->count);
         goto fail;
      }
      /* scan optional bounds vector name in field 2 */
      if (name[0] == '\n') strcpy(name, dsa->f2);
      if (!(dsa->f2[0] == '\0' || strcmp(dsa->f2, name) == 0))
      {  print("%s:%d: at most one bounds vector allowed", dsa->fname,
            dsa->count);
         goto fail;
      }
      /* scan column name in field 3 */
      if (dsa->f3[0] == '\0')
      {  print("%s:%d: missing column name in field 3", dsa->fname,
            dsa->count);
         goto fail;
      }
      j = lpx_find_col(dsa->lp, dsa->f3);
      if (j == 0)
      {  print("%s:%d: column %s not found", dsa->fname, dsa->count,
            dsa->f3);
         goto fail;
      }
      /* scan optional bound value in field 4 */
      if (!(strcmp(dsa->f1, "LO") == 0 || strcmp(dsa->f1, "UP") == 0 ||
            strcmp(dsa->f1, "FX") == 0 || strcmp(dsa->f1, "UI") == 0))
         goto skip;
      if (dsa->f4[0] == '\0')
      {  print("%s:%d: missing bound value in field 4", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (str2num(dsa->f4, &temp))
      {  print("%s:%d: invalid bound value in field 4", dsa->fname,
            dsa->count);
         goto fail;
      }
skip: /* change column bounds */
      type = lpx_get_col_type(dsa->lp, j);
      if (type == LPX_FR || type == LPX_UP)
         lb = -DBL_MAX;
      else
         lb = lpx_get_col_lb(dsa->lp, j);
      if (type == LPX_FR || type == LPX_LO)
         ub = +DBL_MAX;
      else
         ub = lpx_get_col_ub(dsa->lp, j);
      if (strcmp(dsa->f1, "LO") == 0)
         lb = temp;
      else if (strcmp(dsa->f1, "UP") == 0)
         ub = temp;
      else if (strcmp(dsa->f1, "FX") == 0)
         lb = ub = temp;
      else if (strcmp(dsa->f1, "FR") == 0)
         lb = -DBL_MAX, ub = +DBL_MAX;
      else if (strcmp(dsa->f1, "MI") == 0)
         lb = -DBL_MAX;
      else if (strcmp(dsa->f1, "PL") == 0)
         ub = +DBL_MAX;
#if 1 /* 11/V-2006 */
      else if (strcmp(dsa->f1, "LI") == 0)
      {  /* lpx_set_class(dsa->lp, LPX_MIP); */
         lpx_set_col_kind(dsa->lp, j, LPX_IV);
         lb = temp;
      }
#endif
      else if (strcmp(dsa->f1, "UI") == 0)
      {  /* lpx_set_class(dsa->lp, LPX_MIP); */
         lpx_set_col_kind(dsa->lp, j, LPX_IV);
         ub = temp;
      }
      else if (strcmp(dsa->f1, "BV") == 0)
      {  /* lpx_set_class(dsa->lp, LPX_MIP); */
         lpx_set_col_kind(dsa->lp, j, LPX_IV);
         lb = 0.0, ub = 1.0;
      }
      else
      {  print("%s:%d: invalid bound type in field 1", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (lb == -DBL_MAX && ub == +DBL_MAX)
         type = LPX_FR;
      else if (ub == +DBL_MAX)
         type = LPX_LO;
      else if (lb == -DBL_MAX)
         type = LPX_UP;
      else if (lb != ub)
         type = LPX_DB;
      else
         type = LPX_FX;
      lpx_set_col_bnds(dsa->lp, j, type, lb, ub);
      /* fields 5-6 must be blank */
      if (!(dsa->f5[0] == '\0' && dsa->f6[0] == '\0'))
      {  print("%s:%d: invalid data card; fields 5-6 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
      goto loop;
done: return 0;
fail: return 1;
}

LPX *lpx_read_mps(const char *fname)
{     /* read problem data in MPS format */
      struct dsa _dsa, *dsa = &_dsa;
      dsa->lp = NULL;
      dsa->fname = fname;
      dsa->fp = NULL;
      dsa->count = 0;
      dsa->obj = 0;
      print("lpx_read_mps: reading problem data from `%s'...",
         dsa->fname);
      dsa->fp = xfopen(dsa->fname, "r");
      if (dsa->fp == NULL)
      {  print("lpx_read_mps: unable to open `%s' - %s", dsa->fname,
            strerror(errno));
         goto fail;
      }
      dsa->lp = lpx_create_prob();
      lpx_create_index(dsa->lp);
      /* read NAME indicator card */
      if (read_card(dsa)) goto fail;
      if (memcmp(dsa->card, "NAME ", 5) != 0)
      {  print("%s:%d: NAME indicator card missing", dsa->fname,
            dsa->count);
         goto fail;
      }
      memcpy(dsa->f3, dsa->card+14, 8);
      dsa->f3[8] = '\0'; adjust_name(dsa->f3);
      lpx_set_prob_name(dsa->lp, dsa->f3);
      if (dsa->f3[0] == '\0')
         print("lpx_read_mps: problem name not specified");
      else
         print("lpx_read_mps: problem %s", dsa->f3);
      /* read ROWS section */
      if (read_card(dsa)) goto fail;
      if (memcmp(dsa->card, "ROWS ", 5) != 0)
      {  print("%s:%d: ROWS indicator card missing", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (read_rows(dsa)) goto fail;
      /* read COLUMNS section */
      if (memcmp(dsa->card, "COLUMNS ", 8) != 0)
      {  print("%s:%d: COLUMNS indicator card missing", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (read_columns(dsa)) goto fail;
      {  int m = lpx_get_num_rows(dsa->lp);
         int n = lpx_get_num_cols(dsa->lp);
         int nnz = lpx_get_num_nz(dsa->lp);
         print("lpx_read_mps: %d row%s, %d column%s, %d non-zero%s",
            m, m == 1 ? "" : "s", n, n == 1 ? "" : "s", nnz, nnz == 1 ?
            "" : "s");
      }
      /* read RHS section (optional) */
      if (memcmp(dsa->card, "RHS ", 4) == 0)
         if (read_rhs(dsa)) goto fail;
      /* read RANGES section (optional) */
      if (memcmp(dsa->card, "RANGES ", 7) == 0)
         if (read_ranges(dsa)) goto fail;
      /* read BOUNDS section (optional) */
      if (memcmp(dsa->card, "BOUNDS ", 7) == 0)
         if (read_bounds(dsa)) goto fail;
      /* check ENDATA indicator card */
      if (memcmp(dsa->card, "ENDATA ", 7) != 0)
      {  print("%s:%d: ENDATA indicator card missing", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (lpx_get_class(dsa->lp) == LPX_MIP)
      {  int ni = lpx_get_num_int(dsa->lp);
         int nb = lpx_get_num_bin(dsa->lp);
         char s[50];
         if (nb == 0)
            strcpy(s, "none of");
         else if (ni == 1 && nb == 1)
            strcpy(s, "");
         else if (nb == 1)
            strcpy(s, "one of");
         else if (nb == ni)
            strcpy(s, "all of");
         else
            sprintf(s, "%d of", nb);
         print("lpx_read_mps: %d integer column%s, %s which %s binary",
            ni, ni == 1 ? "" : "s", s, nb == 1 ? "is" : "are");
      }
      print("lpx_read_mps: %d cards were read", dsa->count);
      xfclose(dsa->fp);
      lpx_delete_index(dsa->lp);
      lpx_order_matrix(dsa->lp);
      return dsa->lp;
fail: if (dsa->lp != NULL) lpx_delete_prob(dsa->lp);
      if (dsa->fp != NULL) xfclose(dsa->fp);
      return NULL;
}

/*----------------------------------------------------------------------
-- lpx_write_mps - write problem data in fixed MPS format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_write_mps(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_write_mps writes problem data in fixed MPS format to
-- an output text file whose name is the character string fname.
--
-- *Returns*
--
-- If the operation was successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

static char *row_name(LPX *lp, int i, char rname[8+1])
{     const char *str = NULL;
      if (lpx_get_int_parm(lp, LPX_K_MPSORIG))
      {  if (i == 0)
            str = lpx_get_obj_name(lp);
         else
            str = lpx_get_row_name(lp, i);
         if (str != NULL && strlen(str) > 8) str = NULL;
      }
      if (str == NULL)
         sprintf(rname, "R%07d", i);
      else
         strcpy(rname, str);
      return rname;
}

static char *col_name(LPX *lp, int j, char cname[8+1])
{     const char *str = NULL;
      if (lpx_get_int_parm(lp, LPX_K_MPSORIG))
      {  str = lpx_get_col_name(lp, j);
         if (str != NULL && strlen(str) > 8) str = NULL;
      }
      if (str == NULL)
         sprintf(cname, "C%07d", j);
      else
         strcpy(cname, str);
      return cname;
}

static char *mps_numb(double val, char numb[12+1])
{     int n;
      char str[255+1], *e;
      for (n = 12; n >= 6; n--)
      {  if (val != 0.0 && fabs(val) < 0.002)
#if 0
            sprintf(str, "%.*E", n, val);
#else
            /* n is number of desired decimal places, but in case of E
               format precision means number of digits that follow the
               decimal point */
            sprintf(str, "%.*E", n-1, val);
#endif
         else
            sprintf(str, "%.*G", n, val);
         xassert(strlen(str) <= 255);
         e = strchr(str, 'E');
         if (e != NULL) sprintf(e+1, "%d", atoi(e+1));
         if (strlen(str) <= 12) return strcpy(numb, str);
      }
      fault("lpx_write_mps: unable to convert floating point number %g "
         "to character string", val);
      return NULL; /* make the compiler happy */
}

int lpx_write_mps(LPX *lp, const char *fname)
{     FILE *fp;
      int wide = lpx_get_int_parm(lp, LPX_K_MPSWIDE);
      int frei = lpx_get_int_parm(lp, LPX_K_MPSFREE);
      int skip = lpx_get_int_parm(lp, LPX_K_MPSSKIP);
      int marker = 0; /* intorg/intend marker count */
      int mip, make_obj, nrows, ncols, i, j, flag, *ndx;
      double *obj, *val;
      char rname[8+1], cname[8+1], vname[8+1], numb[12+1];
      print("lpx_write_mps: writing problem data to `%s'...", fname);
      /* open the output text file */
      fp = xfopen(fname, "w");
      if (fp == NULL)
      {  print("lpx_write_mps: unable to create `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      /* determine whether the problem is LP or MIP */
      mip = (lpx_get_class(lp) == LPX_MIP);
      /* determine number of rows and number of columns */
      nrows = lpx_get_num_rows(lp);
      ncols = lpx_get_num_cols(lp);
      /* the problem should contain at least one row and one column */
      if (!(nrows > 0 && ncols > 0))
         fault("lpx_write_mps: problem has no rows/columns");
      /* determine if the routine should output the objective row */
      make_obj = lpx_get_int_parm(lp, LPX_K_MPSOBJ);
      if (make_obj == 2)
      {  for (i = 1; i <= nrows; i++)
         {  int typx;
            lpx_get_row_bnds(lp, i, &typx, NULL, NULL);
            if (typx == LPX_FR)
            {  make_obj = 0;
               break;
            }
         }
      }
      /* write comment cards (if required) */
      if (lpx_get_int_parm(lp, LPX_K_MPSINFO))
      {  const char *name = lpx_get_prob_name(lp);
         if (name == NULL) name = "UNKNOWN";
         fprintf(fp, "* Problem:    %.31s\n", name);
         fprintf(fp, "* Class:      %s\n", !mip ? "LP" : "MIP");
         fprintf(fp, "* Rows:       %d\n", nrows);
         if (!mip)
            fprintf(fp, "* Columns:    %d\n", ncols);
         else
            fprintf(fp, "* Columns:    %d (%d integer, %d binary)\n",
               ncols, lpx_get_num_int(lp), lpx_get_num_bin(lp));
         fprintf(fp, "* Non-zeros:  %d\n", lpx_get_num_nz(lp));
         fprintf(fp, "* Format:     Fixed MPS\n");
         fprintf(fp, "*\n");
      }
      /* write NAME indicator card */
      {  const char *name = lpx_get_prob_name(lp);
         if (name == NULL)
            fprintf(fp, "NAME\n");
         else
            fprintf(fp, "NAME          %.8s\n", name);
      }
      /* write ROWS section */
      fprintf(fp, "ROWS\n");
      if (make_obj)
         fprintf(fp, " %c  %s\n", 'N', row_name(lp, 0, rname));
      for (i = 1; i <= nrows; i++)
      {  int typx;
         lpx_get_row_bnds(lp, i, &typx, NULL, NULL);
         switch (typx)
         {  case LPX_FR: typx = 'N'; break;
            case LPX_LO: typx = 'G'; break;
            case LPX_UP: typx = 'L'; break;
            case LPX_DB: typx = 'E'; break;
            case LPX_FX: typx = 'E'; break;
            default: xassert(typx != typx);
         }
         fprintf(fp, " %c  %s\n", typx, row_name(lp, i, rname));
      }
      /* prepare coefficients of the objective function */
      obj = xcalloc(1+ncols, sizeof(double));
      obj[0] = lpx_get_obj_coef(lp, 0);
      for (j = 1; j <= ncols; j++)
         obj[j] = lpx_get_obj_coef(lp, j);
      ndx = xcalloc(1+ncols, sizeof(int));
      val = xcalloc(1+ncols, sizeof(double));
      for (i = 1; i <= nrows; i++)
#if 0
      {  double ci = lpx_get_row_coef(lp, i);
#else
      {  double ci = 0.0;
#endif
         if (ci != 0.0)
         {  int len, t;
            len = lpx_get_mat_row(lp, i, ndx, val);
            for (t = 1; t <= len; t++)
               obj[ndx[t]] += ci * val[t];
         }
      }
      xfree(ndx);
      xfree(val);
      for (j = 0; j <= ncols; j++)
         if (fabs(obj[j]) < 1e-12) obj[j] = 0.0;
      /* write COLUMNS section */
      fprintf(fp, "COLUMNS\n");
      ndx = xcalloc(1+nrows, sizeof(int));
      val = xcalloc(1+nrows, sizeof(double));
      for (j = 1; j <= ncols; j++)
      {  int nl = 1, iv, len, t;
         col_name(lp, j, cname);
         iv = (mip && lpx_get_col_kind(lp, j) == LPX_IV);
         if (iv && marker % 2 == 0)
         {  /* open new intorg/intend group */
            marker++;
            fprintf(fp, "    M%07d  'MARKER'                 'INTORG'\n"
               , marker);
         }
         else if (!iv && marker % 2 == 1)
         {  /* close the current intorg/intend group */
            marker++;
            fprintf(fp, "    M%07d  'MARKER'                 'INTEND'\n"
               , marker);
         }
         /* obtain j-th column of the constraint matrix */
         len = lpx_get_mat_col(lp, j, ndx, val);
         ndx[0] = 0;
         val[0] = (make_obj ? obj[j] : 0.0);
         if (len == 0 && val[0] == 0.0 && !skip)
            fprintf(fp, "    %-8s  %-8s  %12s   $ empty column\n",
               cname, row_name(lp, 1, rname), mps_numb(0.0, numb));
         for (t = val[0] != 0.0 ? 0 : 1; t <= len; t++)
         {  if (nl)
               fprintf(fp, "    %-8s  ", cname);
            else
               fprintf(fp, "   ");
            fprintf(fp, "%-8s  %12s",
               row_name(lp, ndx[t], rname), mps_numb(val[t], numb));
            if (wide) nl = 1 - nl;
            if (nl) fprintf(fp, "\n");
            if (frei) strcpy(cname, "");
         }
         if (!nl) fprintf(fp, "\n");
      }
      if (marker % 2 == 1)
      {  /* close the last intorg/intend group (if not closed) */
         marker++;
         fprintf(fp, "    M%07d  'MARKER'                 'INTEND'\n",
            marker);
      }
      xfree(ndx);
      xfree(val);
      /* write RHS section */
      flag = 0;
      {  int nl = 1;
         strcpy(vname, frei ? "" : "RHS1");
         for (i = make_obj ? 0 : 1; i <= nrows; i++)
         {  int typx;
            double lb, ub, rhs;
            if (i == 0)
               typx = LPX_FR, lb = ub = 0.0;
            else
               lpx_get_row_bnds(lp, i, &typx, &lb, &ub);
            switch (typx)
            {  case LPX_FR:
                  rhs = (make_obj && i == 0 ? obj[0] : 0.0); break;
               case LPX_LO:
                  rhs = lb; break;
               case LPX_UP:
                  rhs = ub; break;
               case LPX_DB:
                  rhs = (ub > 0.0 ? lb : ub); break;
               case LPX_FX:
                  rhs = lb; break;
               default:
                  xassert(typx != typx);
            }
            if (rhs == 0.0) continue;
            if (!flag) fprintf(fp, "RHS\n"), flag = 1;
            if (nl)
                fprintf(fp, "    %-8s  ", vname);
            else
                fprintf(fp, "   ");
            fprintf(fp, "%-8s  %12s",
               row_name(lp, i, rname), mps_numb(rhs, numb));
            if (wide) nl = 1 - nl;
            if (nl) fprintf(fp, "\n");
         }
         if (!nl) fprintf(fp, "\n");
      }
      xfree(obj);
      /* write RANGES section */
      flag = 0;
      {  int nl = 1;
         strcpy(vname, frei ? "" : "RNG1");
         for (i = 1; i <= nrows; i++)
         {  int typx;
            double lb, ub, rng;
            lpx_get_row_bnds(lp, i, &typx, &lb, &ub);
            if (typx != LPX_DB) continue;
            if (!flag) fprintf(fp, "RANGES\n"), flag = 1;
            if (nl)
                fprintf(fp, "    %-8s  ", vname);
            else
                fprintf(fp, "   ");
            rng = (ub > 0.0 ? ub - lb : lb - ub);
            fprintf(fp, "%-8s  %12s",
               row_name(lp, i, rname), mps_numb(rng, numb));
            if (wide) nl = 1 - nl;
            if (nl) fprintf(fp, "\n");
         }
         if (!nl) fprintf(fp, "\n");
      }
      /* write BOUNDS section */
      flag = 0;
      {  strcpy(vname, frei ? "" : "BND1");
         for (j = 1; j <= ncols; j++)
         {  int typx;
            double lb, ub;
            lpx_get_col_bnds(lp, j, &typx, &lb, &ub);
            if (typx == LPX_LO && lb == 0.0) continue;
            if (!flag) fprintf(fp, "BOUNDS\n"), flag = 1;
            switch (typx)
            {  case LPX_FR:
                  fprintf(fp, " FR %-8s  %-8s\n", vname,
                     col_name(lp, j, cname));
                  break;
               case LPX_LO:
                  fprintf(fp, " LO %-8s  %-8s  %12s\n", vname,
                     col_name(lp, j, cname), mps_numb(lb, numb));
                  break;
               case LPX_UP:
                  fprintf(fp, " MI %-8s  %-8s\n", vname,
                     col_name(lp, j, cname));
                  fprintf(fp, " UP %-8s  %-8s  %12s\n", vname,
                     col_name(lp, j, cname), mps_numb(ub, numb));
                  break;
               case LPX_DB:
                  if (lb != 0.0)
                  fprintf(fp, " LO %-8s  %-8s  %12s\n", vname,
                     col_name(lp, j, cname), mps_numb(lb, numb));
                  fprintf(fp, " UP %-8s  %-8s  %12s\n", vname,
                     col_name(lp, j, cname), mps_numb(ub, numb));
                  break;
               case LPX_FX:
                  fprintf(fp, " FX %-8s  %-8s  %12s\n", vname,
                     col_name(lp, j, cname), mps_numb(lb, numb));
                  break;
               default:
                  xassert(typx != typx);
            }
         }
      }
      /* write ENDATA indicator card */
      fprintf(fp, "ENDATA\n");
      /* close the output text file */
      fflush(fp);
      if (ferror(fp))
      {  print("lpx_write_mps: write error on `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      xfclose(fp);
      /* return to the calling program */
      return 0;
fail: /* the operation failed */
      if (fp != NULL) xfclose(fp);
      return 1;
}

/*----------------------------------------------------------------------
-- lpx_read_bas - read LP basis in fixed MPS format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_read_bas(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_read_bas reads LP basis in fixed MPS format from an
-- input text file whose name is the character string fname.
--
-- *Returns*
--
-- If the operation was successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

int lpx_read_bas(LPX *lp, const char *fname)
{     struct dsa _dsa, *dsa = &_dsa;
      int i, j;
      dsa->lp = lp;
      dsa->fname = fname;
      dsa->fp = NULL;
      dsa->count = 0;
      dsa->obj = 0;
      print("lpx_read_bas: reading LP basis from `%s'...",
         dsa->fname);
      dsa->fp = xfopen(dsa->fname, "r");
      if (dsa->fp == NULL)
      {  print("lpx_read_bas: unable to open `%s' - %s", dsa->fname,
            strerror(errno));
         goto fail;
      }
      lpx_create_index(dsa->lp);
      /* read NAME indicator card */
      if (read_card(dsa)) goto fail;
      if (memcmp(dsa->card, "NAME ", 5) != 0)
      {  print("%s:%d: NAME indicator card missing", dsa->fname,
            dsa->count);
         goto fail;
      }
      /* build the "standard" basis of all slacks */
      lpx_std_basis(dsa->lp);
loop: /* read and split next data card */
      if (read_card(dsa)) goto fail;
      if (dsa->card[0] != ' ') goto fini;
      if (split_card(dsa)) goto fail;
      /* check indicator in field 1 */
      if (!(strcmp(dsa->f1, "XL") == 0 || strcmp(dsa->f1, "XU") == 0 ||
            strcmp(dsa->f1, "LL") == 0 || strcmp(dsa->f1, "UL") == 0))
      {  print("%s:%d: invalid indicator in filed 1", dsa->fname,
            dsa->count);
         goto fail;
      }
      /* scan column name in field 2 */
      if (dsa->f2[0] == '\0')
      {  print("%s:%d: missing column name in field 2", dsa->fname,
            dsa->count);
         goto fail;
      }
      j = lpx_find_col(dsa->lp, dsa->f2);
      if (j == 0)
      {  print("%s:%d: column %s not found", dsa->fname, dsa->count,
            dsa->f2);
         goto fail;
      }
      /* process field 3 */
      if (dsa->f1[0] == 'X')
      {  /* scan row name in field 3 */
         if (dsa->f3[0] == '\0')
         {  print("%s:%d: missing row name in field 3", dsa->fname,
               dsa->count);
            goto fail;
         }
         i = lpx_find_row(dsa->lp, dsa->f3);
         if (i == 0)
         {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
               dsa->f3);
            goto fail;
         }
      }
      else
      {  /* field 3 must be blank */
         if (dsa->f3[0] != '\0')
         {  print("%s:%d: invalid data card; field 3 must be blank",
               dsa->fname, dsa->count);
            goto fail;
         }
         i = 0;
      }
      /* fields 4-6 must be blank */
      if (!(dsa->f4[0] == '\0' && dsa->f5[0] == '\0' &&
            dsa->f6[0] == '\0'))
      {  print("%s:%d: invalid data card; fields 4-6 must be blank",
            dsa->fname, dsa->count);
         goto fail;
      }
      /* apply a "patch" */
      if (dsa->f1[0] == 'X')
      {  lpx_set_row_stat(dsa->lp, i, dsa->f1[1] == 'L' ? LPX_NL :
            LPX_NU);
         lpx_set_col_stat(dsa->lp, j, LPX_BS);
      }
      else
         lpx_set_col_stat(dsa->lp, j, dsa->f1[0] == 'L' ? LPX_NL :
            LPX_NU);
      goto loop;
fini: /* check ENDATA indicator card */
      if (memcmp(dsa->card, "ENDATA ", 7) != 0)
      {  print("%s:%d: ENDATA indicator card missing", dsa->fname,
            dsa->count);
         goto fail;
      }
      print("lpx_read_bas: %d cards were read", dsa->count);
      xfclose(dsa->fp);
      lpx_delete_index(dsa->lp);
      return 0;
fail: if (dsa->fp != NULL) xfclose(dsa->fp);
      lpx_delete_index(dsa->lp);
      return 1;
}

/*----------------------------------------------------------------------
-- lpx_write_bas - write LP basis in fixed MPS format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_write_bas(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_write_bas writes current LP basis in fixed MPS format
-- to an output text file whose name is the character string fname.
--
-- *Returns*
--
-- If the operation was successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

int lpx_write_bas(LPX *lp, const char *fname)
{     FILE *fp;
      int nrows, ncols, i, j, rtype, ctype, rstat, cstat;
      char rname[8+1], cname[8+1];
      print("lpx_write_bas: writing LP basis to `%s'...", fname);
      /* open the output text file */
      fp = xfopen(fname, "w");
      if (fp == NULL)
      {  print("lpx_write_bas: unable to create `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      /* determine number of rows and number of columns */
      nrows = lpx_get_num_rows(lp);
      ncols = lpx_get_num_cols(lp);
      /* the problem should contain at least one row and one column */
      if (!(nrows > 0 && ncols > 0))
         fault("lpx_write_bas: problem has no rows/columns");
      /* write comment cards (if required) */
      if (lpx_get_int_parm(lp, LPX_K_MPSINFO))
      {  int dir, status;
         double obj;
         const char *name;
         /* problem name and statistics */
         name = lpx_get_prob_name(lp);
         if (name == NULL) name = "UNKNOWN";
         fprintf(fp, "* Problem:    %.31s\n", name);
         fprintf(fp, "* Rows:       %d\n", nrows);
         fprintf(fp, "* Columns:    %d\n", ncols);
         fprintf(fp, "* Non-zeros:  %d\n", lpx_get_num_nz(lp));
         /* solution status */
         status = lpx_get_status(lp);
         fprintf(fp, "* Status:     %s\n",
            status == LPX_OPT    ? "OPTIMAL" :
            status == LPX_FEAS   ? "FEASIBLE" :
            status == LPX_INFEAS ? "INFEASIBLE (INTERMEDIATE)" :
            status == LPX_NOFEAS ? "INFEASIBLE (FINAL)" :
            status == LPX_UNBND  ? "UNBOUNDED" :
            status == LPX_UNDEF  ? "UNDEFINED" : "???");
         /* objective function */
         name = lpx_get_obj_name(lp);
         dir = lpx_get_obj_dir(lp);
         obj = lpx_get_obj_val(lp);
         fprintf(fp, "* Objective:  %s%s%.10g %s\n",
            name == NULL ? "" : name,
            name == NULL ? "" : " = ", obj,
            dir == LPX_MIN ? "(MINimum)" :
            dir == LPX_MAX ? "(MAXimum)" : "(" "???" ")");
         fprintf(fp, "* Format:     Fixed MPS\n");
         fprintf(fp, "*\n");
      }
      /* write NAME indicator card */
      {  const char *name = lpx_get_prob_name(lp);
         if (name == NULL)
            fprintf(fp, "NAME\n");
         else
            fprintf(fp, "NAME          %.8s\n", name);
      }
      /* write information about which columns should be made basic
         and which rows should be made non-basic */
      i = j = 0;
      for (;;)
      {  /* find a next non-basic row */
         for (i++; i <= nrows; i++)
         {  lpx_get_row_info(lp, i, &rstat, NULL, NULL);
            if (rstat != LPX_BS) break;
         }
         /* find a next basic column */
         for (j++; j <= ncols; j++)
         {  lpx_get_col_info(lp, j, &cstat, NULL, NULL);
            if (cstat == LPX_BS) break;
         }
         /* if all non-basic rows and basic columns have been written,
            break the loop */
         if (i > nrows && j > ncols) break;
         /* since the basis is valid, there must be nor extra non-basic
            rows nor extra basic columns */
         xassert(i <= nrows && j <= ncols);
         /* write the pair (basic column, non-basic row) */
         lpx_get_row_bnds(lp, i, &rtype, NULL, NULL);
         fprintf(fp, " %s %-8s  %s\n",
            (rtype == LPX_DB && rstat == LPX_NU) ? "XU" : "XL",
            col_name(lp, j, cname), row_name(lp, i, rname));
      }
      /* write information about statuses of double-bounded non-basic
         columns */
      for (j = 1; j <= ncols; j++)
      {  lpx_get_col_bnds(lp, j, &ctype, NULL, NULL);
         lpx_get_col_info(lp, j, &cstat, NULL, NULL);
         if (ctype == LPX_DB && cstat != LPX_BS)
            fprintf(fp, " %s %s\n",
               cstat == LPX_NU ? "UL" : "LL", col_name(lp, j, cname));
      }
      /* write ENDATA indcator card */
      fprintf(fp, "ENDATA\n");
      /* close the output text file */
      fflush(fp);
      if (ferror(fp))
      {  print("lpx_write_bas: write error on `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      xfclose(fp);
      /* return to the calling program */
      return 0;
fail: /* the operation failed */
      if (fp != NULL) xfclose(fp);
      return 1;
}

/*----------------------------------------------------------------------
-- lpx_read_freemps - read problem data in free MPS format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- LPX *lpx_read_freemps(char *fname);
--
-- *Description*
--
-- The routine lpx_read_freemps reads LP/MIP problem data in free MPS
-- format from an input text file whose name is the character string
-- fname.
--
-- *Returns*
--
-- If no error occurred, the routine returns a pointer to the created
-- problem object. Otherwise the routine returns NULL. */

#define dsa dsa1
#define read_rows read_rows1
#define put_column put_column1
#define read_columns read_columns1
#define set_rhs set_rhs1
#define read_rhs read_rhs1
#define set_range set_range1
#define read_ranges read_ranges1
#define read_bounds read_bounds1

struct dsa
{     /* working area used by MPS routines */
      LPX *lp;
      /* LP/MIP problem object */
      const char *fname;
      /* name of input text file */
      FILE *fp;
      /* stream assigned to input text file */
      int count;
      /* line count */
      int c;
      /* current character */
      char item[255+1];
      /* current item */
      int obj;
      /* ordinal number of free (unbounded) row used as the objective
         function */
};

static int read_c(struct dsa *dsa)
{     /* read character from input file */
      int c;
      if (dsa->c == '\n') dsa->count++;
      c = fgetc(dsa->fp);
      if (ferror(dsa->fp))
      {  print("%s:%d: read error - %s", dsa->fname, dsa->count,
            strerror(errno));
         goto fail;
      }
      if (feof(dsa->fp))
      {  if (dsa->c == '\n')
         {  print("%s:%d: unexpected EOF", dsa->fname, dsa->count);
            goto fail;
         }
         c = '\n';
      }
      if (c == '\n')
         ;
      else if (isspace(c))
         c = ' ';
      else if (iscntrl(c))
      {  print("%s:%d: invalid control character 0x%02X", dsa->fname,
            dsa->count, c);
         goto fail;
      }
      dsa->c = c;
      return 0;
fail: return 1;
}

static int read_char(struct dsa *dsa)
{     /* read character from input file and skip comment lines */
      int nl;
loop: nl = (dsa->c == '\n');
      if (read_c(dsa)) goto fail;
      if (nl && dsa->c == '*')
      {  while (dsa->c != '\n')
            if (read_c(dsa)) goto fail;
         goto loop;
      }
      return 0;
fail: return 1;
}

static int read_item(struct dsa *dsa)
{     /* read item from input file */
      /* skip uninteresting characters */
      while (dsa->c == ' ')
         if (read_char(dsa)) goto fail;
      /* scan item */
      if (dsa->c == '$')
      {  /* dollar sign begins a comment */
         while (dsa->c != '\n')
            if (read_char(dsa)) goto fail;
      }
      if (dsa->c == '\n')
      {  /* empty item indicates end of line */
         dsa->item[0] = '\0';
         if (read_char(dsa)) goto fail;
      }
      else
      {  int len = 0;
         while (!(dsa->c == ' ' || dsa->c == '\n'))
         {  if (len == sizeof(dsa->item) - 1)
            {  print("%s:%d: item `%.15s...' too long", dsa->fname,
                  dsa->count, dsa->item);
               goto fail;
            }
            dsa->item[len++] = (char)dsa->c;
            if (read_char(dsa)) goto fail;
         }
         dsa->item[len] = '\0';
      }
      return 0;
fail: return 1;
}

static int read_rows(struct dsa *dsa)
{     /* process data lines in ROWS section */
      int i, type;
loop: /* check if there is more data lines */
      if (dsa->c != ' ') goto done;
      /* create new row */
      i = lpx_add_rows(dsa->lp, 1);
      /* scan row type */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing row type", dsa->fname, dsa->count);
         goto fail;
      }
      else if (strcmp(dsa->item, "N") == 0)
      {  type = LPX_FR;
         /* the first free (unbounded) row is used as the objective
            function */
         if (dsa->obj == 0) dsa->obj = i;
      }
      else if (strcmp(dsa->item, "L") == 0)
         type = LPX_UP;
      else if (strcmp(dsa->item, "G") == 0)
         type = LPX_LO;
      else if (strcmp(dsa->item, "E") == 0)
         type = LPX_FX;
      else
      {  print("%s:%d: invalid row type", dsa->fname, dsa->count);
         goto fail;
      }
      lpx_set_row_bnds(dsa->lp, i, type, 0.0, 0.0);
      /* scan row name */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing row name", dsa->fname, dsa->count);
         goto fail;
      }
      lpx_set_row_name(dsa->lp, i, dsa->item);
      if (dsa->obj == i) lpx_set_obj_name(dsa->lp, dsa->item);
      /* skip remaining characters */
      while (dsa->c != '\n')
         if (read_char(dsa)) goto fail;
      if (read_item(dsa)) goto fail;
      xassert(dsa->item[0] == '\0');
      goto loop;
done: return 0;
fail: return 1;
}

static void put_column(struct dsa *dsa, int j, int len, int ind[],
      double aij[])
{     /* store j-th column to the constraint matrix */
      int k, newlen = 0;
      for (k = 1; k <= len; k++)
      {  if (aij[k] != 0.0)
         {  newlen++;
            ind[newlen] = ind[k];
            aij[newlen] = aij[k];
            if (dsa->obj == ind[newlen])
               lpx_set_obj_coef(dsa->lp, j, aij[newlen]);
         }
      }
      lpx_set_mat_col(dsa->lp, j, newlen, ind, aij);
      return;
}

static int read_columns(struct dsa *dsa)
{     /* process data lines in COLUMNS section */
      int i, j = 0, k, len = 0, m, marker = 0, *map, *ind;
      double *aij, temp;
      char cname[sizeof(dsa->item)], rname[sizeof(dsa->item)];
      /* allocate working arrays */
      m = lpx_get_num_rows(dsa->lp);
      map = xcalloc(1+m, sizeof(int));
      for (i = 1; i <= m; i++) map[i] = 0;
      ind = xcalloc(1+m, sizeof(int));
      aij = xcalloc(1+m, sizeof(double));
loop: /* check if there is more data lines */
      if (dsa->c != ' ') goto done;
      /* scan column name */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing column name", dsa->fname, dsa->count);
         goto fail;
      }
      strcpy(cname, dsa->item);
      /* scan row name */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing row name", dsa->fname, dsa->count);
         goto fail;
      }
      strcpy(rname, dsa->item);
      /* scan optional INTORG/INTEND marker */
      if (strcmp(rname, "'MARKER'") == 0)
      {  if (read_item(dsa)) goto fail;
         if (dsa->item[0] == '\0')
         {  print("%s:%d: missing marker type", dsa->fname, dsa->count);
            goto fail;
         }
         else if (strcmp(dsa->item, "'INTORG'") == 0)
            marker = 1;
         else if (strcmp(dsa->item, "'INTEND'") == 0)
            marker = 0;
         else
         {  print("%s:%d: invalid marker type", dsa->fname, dsa->count);
            goto fail;
         }
         goto skip;
      }
      if (j == 0 || strcmp(lpx_get_col_name(dsa->lp, j), cname) != 0)
      {  /* new column begins, so finish the current column */
         if (j != 0)
         {  for (k = 1; k <= len; k++) map[ind[k]] = 0;
            put_column(dsa, j, len, ind, aij), len = 0;
         }
         /* create new column */
         if (lpx_find_col(dsa->lp, cname) != 0)
         {  print("%s:%d: column %s multiply specified", dsa->fname,
               dsa->count, cname);
            goto fail;
         }
         j = lpx_add_cols(dsa->lp, 1);
         lpx_set_col_name(dsa->lp, j, cname);
         if (marker)
         {  /* lpx_set_class(dsa->lp, LPX_MIP); */
            lpx_set_col_kind(dsa->lp, j, LPX_IV);
         }
         lpx_set_col_bnds(dsa->lp, j, LPX_LO, 0.0, 0.0);
      }
      /* find row */
      i = lpx_find_row(dsa->lp, rname);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            rname);
         goto fail;
      }
      /* scan coefficient value */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing coefficient value", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (str2num(dsa->item, &temp))
      {  print("%s:%d: invalid coefficient value", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: coefficient in row %s multiply specified",
            dsa->fname, dsa->count, rname);
         goto fail;
      }
      map[i] = 1, len++, ind[len] = i, aij[len] = temp;
      /* there may be another row name and coefficient value */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0') goto loop;
      strcpy(rname, dsa->item);
      /* find row */
      i = lpx_find_row(dsa->lp, rname);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            rname);
         goto fail;
      }
      /* scan coefficient value */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing coefficient value", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (str2num(dsa->item, &temp))
      {  print("%s:%d: invalid coefficient value", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: coefficient in row %s multiply specified",
            dsa->fname, dsa->count, rname);
         goto fail;
      }
      map[i] = 1, len++, ind[len] = i, aij[len] = temp;
skip: /* skip remaining characters */
      while (dsa->c != '\n')
         if (read_char(dsa)) goto fail;
      if (read_item(dsa)) goto fail;
      xassert(dsa->item[0] == '\0');
      goto loop;
done: /* finish the last column */
      if (j != 0) put_column(dsa, j, len, ind, aij);
      xfree(map), xfree(ind), xfree(aij);
      return 0;
fail: xfree(map), xfree(ind), xfree(aij);
      return 1;
}

static void set_rhs(struct dsa *dsa, int i, double b)
{     /* set right-hand side for i-th row */
      switch (lpx_get_row_type(dsa->lp, i))
      {  case LPX_FR:
            if (dsa->obj == i)
               lpx_set_obj_coef(dsa->lp, 0, b);
            else if (b != 0.0)
               print("%s:%d: warning: non-zero rhs for free row %s igno"
                  "red", dsa->fname, dsa->count, lpx_get_row_name(dsa->
                  lp, i));
            break;
         case LPX_LO:
            lpx_set_row_bnds(dsa->lp, i, LPX_LO, b, 0.0);
            break;
         case LPX_UP:
            lpx_set_row_bnds(dsa->lp, i, LPX_UP, 0.0, b);
            break;
         case LPX_FX:
            lpx_set_row_bnds(dsa->lp, i, LPX_FX, b, b);
            break;
         default:
            xassert(dsa != dsa);
      }
      return;
}

static int read_rhs(struct dsa *dsa)
{     /* process data lines in RHS section */
      int i, m, *map;
      double temp;
      char name[sizeof(dsa->item)] = "";
      /* allocate working array */
      m = lpx_get_num_rows(dsa->lp);
      map = xcalloc(1+m, sizeof(int));
      for (i = 1; i <= m; i++) map[i] = 0;
loop: /* check if there is more data lines */
      if (dsa->c != ' ') goto done;
      /* scan rhs vector name */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing rhs vector name", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (name[0] == '\0') strcpy(name, dsa->item);
      if (strcmp(dsa->item, name) != 0)
      {  print("%s:%d: at most one rhs vector allowed", dsa->fname,
            dsa->count);
         goto fail;
      }
      /* scan row name */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing row name", dsa->fname, dsa->count);
         goto fail;
      }
      /* find row */
      i = lpx_find_row(dsa->lp, dsa->item);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            dsa->item);
         goto fail;
      }
      /* scan rhs value */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing rhs value", dsa->fname, dsa->count);
         goto fail;
      }
      if (str2num(dsa->item, &temp))
      {  print("%s:%d: invalid rhs value", dsa->fname, dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: rhs for row %s multiply specified", dsa->fname,
            dsa->count, lpx_get_row_name(dsa->lp, i));
         goto fail;
      }
      set_rhs(dsa, i, temp), map[i] = 1;
      /* there may be another row name and rhs value */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0') goto loop;
      /* find row */
      i = lpx_find_row(dsa->lp, dsa->item);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            dsa->item);
         goto fail;
      }
      /* scan rhs value */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing rhs value", dsa->fname, dsa->count);
         goto fail;
      }
      if (str2num(dsa->item, &temp))
      {  print("%s:%d: invalid rhs value", dsa->fname, dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: rhs for row %s multiply specified", dsa->fname,
            dsa->count, lpx_get_row_name(dsa->lp, i));
         goto fail;
      }
      set_rhs(dsa, i, temp), map[i] = 1;
      /* skip remaining characters */
      while (dsa->c != '\n')
         if (read_char(dsa)) goto fail;
      if (read_item(dsa)) goto fail;
      xassert(dsa->item[0] == '\0');
      goto loop;
done: xfree(map);
      return 0;
fail: xfree(map);
      return 1;
}

static void set_range(struct dsa *dsa, int i, double r)
{     /* set range value for i-th row */
      double b;
      switch (lpx_get_row_type(dsa->lp, i))
      {  case LPX_FR:
            print("%s:%d: warning: range value for free row %s ignored",
               dsa->fname, dsa->count, lpx_get_row_name(dsa->lp, i));
            break;
         case LPX_LO:
            b = lpx_get_row_lb(dsa->lp, i);
            if (r == 0.0)
               lpx_set_row_bnds(dsa->lp, i, LPX_FX, b, b);
            else
               lpx_set_row_bnds(dsa->lp, i, LPX_DB, b, b + fabs(r));
            break;
         case LPX_UP:
            b = lpx_get_row_ub(dsa->lp, i);
            if (r == 0.0)
               lpx_set_row_bnds(dsa->lp, i, LPX_FX, b, b);
            else
               lpx_set_row_bnds(dsa->lp, i, LPX_DB, b - fabs(r), b);
            break;
         case LPX_FX:
            b = lpx_get_row_lb(dsa->lp, i);
            if (r > 0.0)
               lpx_set_row_bnds(dsa->lp, i, LPX_DB, b, b + fabs(r));
            else if (r < 0.0)
               lpx_set_row_bnds(dsa->lp, i, LPX_DB, b - fabs(r), b);
            break;
         default:
            xassert(dsa != dsa);
      }
      return;
}

static int read_ranges(struct dsa *dsa)
{     /* process data lines in RANGES section */
      int i, m, *map;
      double temp;
      char name[sizeof(dsa->item)] = "";
      /* allocate working array */
      m = lpx_get_num_rows(dsa->lp);
      map = xcalloc(1+m, sizeof(int));
      for (i = 1; i <= m; i++) map[i] = 0;
loop: /* check if there is more data lines */
      if (dsa->c != ' ') goto done;
      /* scan range vector name */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing range vector name", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (name[0] == '\0') strcpy(name, dsa->item);
      if (strcmp(dsa->item, name) != 0)
      {  print("%s:%d: at most one range vector allowed", dsa->fname,
            dsa->count);
         goto fail;
      }
      /* scan row name */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing row name", dsa->fname, dsa->count);
         goto fail;
      }
      /* find row */
      i = lpx_find_row(dsa->lp, dsa->item);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            dsa->item);
         goto fail;
      }
      /* scan range value */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing range value", dsa->fname, dsa->count);
         goto fail;
      }
      if (str2num(dsa->item, &temp))
      {  print("%s:%d: invalid range value", dsa->fname, dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: range for row %s multiply specified", dsa->fname,
            dsa->count, lpx_get_row_name(dsa->lp, i));
         goto fail;
      }
      set_range(dsa, i, temp), map[i] = 1;
      /* there may be another row name and range value */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0') goto loop;
      /* find row */
      i = lpx_find_row(dsa->lp, dsa->item);
      if (i == 0)
      {  print("%s:%d: row %s not found", dsa->fname, dsa->count,
            dsa->item);
         goto fail;
      }
      /* scan range value */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing range value", dsa->fname, dsa->count);
         goto fail;
      }
      if (str2num(dsa->item, &temp))
      {  print("%s:%d: invalid range value", dsa->fname, dsa->count);
         goto fail;
      }
      if (map[i])
      {  print("%s:%d: range for row %s multiply specified", dsa->fname,
            dsa->count, lpx_get_row_name(dsa->lp, i));
         goto fail;
      }
      set_range(dsa, i, temp), map[i] = 1;
      /* skip remaining characters */
      while (dsa->c != '\n')
         if (read_char(dsa)) goto fail;
      if (read_item(dsa)) goto fail;
      xassert(dsa->item[0] == '\0');
      goto loop;
done: xfree(map);
      return 0;
fail: xfree(map);
      return 1;
}

static int read_bounds(struct dsa *dsa)
{     /* process data lines in BOUNDS section */
      int j, type;
      double lb, ub, temp;
      char btyp[2+1];
      char name[sizeof(dsa->item)] = "";
loop: /* check if there is more data lines */
      if (dsa->c != ' ') goto done;
      /* scan bound type */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing bound type", dsa->fname, dsa->count);
         goto fail;
      }
      if (strlen(dsa->item) != 2)
      {  print("%s:%d: invalid bound type", dsa->fname, dsa->count);
         goto fail;
      }
      strcpy(btyp, dsa->item);
      /* scan bound vector name */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing bound vector name", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (name[0] == '\0') strcpy(name, dsa->item);
      if (strcmp(dsa->item, name) != 0)
      {  print("%s:%d: at most one bound vector allowed", dsa->fname,
            dsa->count);
         goto fail;
      }
      /* scan column name */
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing column name", dsa->fname, dsa->count);
         goto fail;
      }
      /* find column */
      j = lpx_find_col(dsa->lp, dsa->item);
      if (j == 0)
      {  print("%s:%d: column %s not found", dsa->fname, dsa->count,
            dsa->item);
         goto fail;
      }
      /* scan optional bound value */
      if (!(strcmp(btyp, "LO") == 0 || strcmp(btyp, "UP") == 0 ||
            strcmp(btyp, "FX") == 0 || strcmp(btyp, "UI") == 0))
         goto skip;
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
      {  print("%s:%d: missing bound value", dsa->fname, dsa->count);
         goto fail;
      }
      if (str2num(dsa->item, &temp))
      {  print("%s:%d: invalid bound value", dsa->fname, dsa->count);
         goto fail;
      }
skip: /* change column bounds */
      type = lpx_get_col_type(dsa->lp, j);
      if (type == LPX_FR || type == LPX_UP)
         lb = -DBL_MAX;
      else
         lb = lpx_get_col_lb(dsa->lp, j);
      if (type == LPX_FR || type == LPX_LO)
         ub = +DBL_MAX;
      else
         ub = lpx_get_col_ub(dsa->lp, j);
      if (strcmp(btyp, "LO") == 0)
         lb = temp;
      else if (strcmp(btyp, "UP") == 0)
         ub = temp;
      else if (strcmp(btyp, "FX") == 0)
         lb = ub = temp;
      else if (strcmp(btyp, "FR") == 0)
         lb = -DBL_MAX, ub = +DBL_MAX;
      else if (strcmp(btyp, "MI") == 0)
         lb = -DBL_MAX;
      else if (strcmp(btyp, "PL") == 0)
         ub = +DBL_MAX;
#if 1 /* 11/V-2006 */
      else if (strcmp(btyp, "LI") == 0)
      {  /* lpx_set_class(dsa->lp, LPX_MIP); */
         lpx_set_col_kind(dsa->lp, j, LPX_IV);
         lb = temp;
      }
#endif
      else if (strcmp(btyp, "UI") == 0)
      {  /* lpx_set_class(dsa->lp, LPX_MIP); */
         lpx_set_col_kind(dsa->lp, j, LPX_IV);
         ub = temp;
      }
      else if (strcmp(btyp, "BV") == 0)
      {  /* lpx_set_class(dsa->lp, LPX_MIP); */
         lpx_set_col_kind(dsa->lp, j, LPX_IV);
         lb = 0.0, ub = 1.0;
      }
      else
      {  print("%s:%d: invalid bound type", dsa->fname, dsa->count);
         goto fail;
      }
      if (lb == -DBL_MAX && ub == +DBL_MAX)
         type = LPX_FR;
      else if (ub == +DBL_MAX)
         type = LPX_LO;
      else if (lb == -DBL_MAX)
         type = LPX_UP;
      else if (lb != ub)
         type = LPX_DB;
      else
         type = LPX_FX;
      lpx_set_col_bnds(dsa->lp, j, type, lb, ub);
      /* skip remaining characters */
      while (dsa->c != '\n')
         if (read_char(dsa)) goto fail;
      if (read_item(dsa)) goto fail;
      xassert(dsa->item[0] == '\0');
      goto loop;
done: return 0;
fail: return 1;
}

LPX *lpx_read_freemps(const char *fname)
{     /* read problem data in free MPS format */
      struct dsa _dsa, *dsa = &_dsa;
      dsa->lp = NULL;
      dsa->fname = fname;
      dsa->fp = NULL;
      dsa->count = 0;
      dsa->c = '\n';
      dsa->obj = 0;
      print("lpx_read_freemps: reading problem data from `%s'...",
         dsa->fname);
      dsa->fp = xfopen(dsa->fname, "r");
      if (dsa->fp == NULL)
      {  print("lpx_read_freemps: unable to open `%s' - %s", dsa->fname,
            strerror(errno));
         goto fail;
      }
      dsa->lp = lpx_create_prob();
      lpx_create_index(dsa->lp);
      /* read very first character */
      if (read_char(dsa)) goto fail;
      /* read NAME indicator record */
      if (dsa->c == ' ')
err1: {  print("%s:%d: NAME indicator record missing", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (read_item(dsa)) goto fail;
      if (strcmp(dsa->item, "NAME") != 0) goto err1;
      if (read_item(dsa)) goto fail;
      if (dsa->item[0] == '\0')
         print("lpx_read_freemps: problem name not specified");
      else
      {  print("lpx_read_freemps: problem %s", dsa->item);
         lpx_set_prob_name(dsa->lp, dsa->item);
         while (dsa->c != '\n')
            if (read_char(dsa)) goto fail;
         if (read_item(dsa)) goto fail;
      }
      xassert(dsa->item[0] == '\0');
      /* read ROWS section */
      if (dsa->c == ' ')
err2: {  print("%s:%d: ROWS indicator record missing", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (read_item(dsa)) goto fail;
      if (strcmp(dsa->item, "ROWS") != 0) goto err2;
      while (dsa->c != '\n')
         if (read_char(dsa)) goto fail;
      if (read_item(dsa)) goto fail;
      xassert(dsa->item[0] == '\0');
      if (read_rows(dsa)) goto fail;
      /* read COLUMNS section */
      xassert(dsa->c != ' ');
      if (read_item(dsa)) goto fail;
      if (strcmp(dsa->item, "COLUMNS") != 0)
      {  print("%s:%d: COLUMNS indicator record missing", dsa->fname,
            dsa->count);
         goto fail;
      }
      while (dsa->c != '\n')
         if (read_char(dsa)) goto fail;
      if (read_item(dsa)) goto fail;
      xassert(dsa->item[0] == '\0');
      if (read_columns(dsa)) goto fail;
      {  int m = lpx_get_num_rows(dsa->lp);
         int n = lpx_get_num_cols(dsa->lp);
         int nnz = lpx_get_num_nz(dsa->lp);
         print("lpx_read_freemps: %d row%s, %d column%s, %d non-zero%s",
            m, m == 1 ? "" : "s", n, n == 1 ? "" : "s", nnz, nnz == 1 ?
            "" : "s");
      }
      /* read RHS section (optional) */
      xassert(dsa->c != ' ');
      if (read_item(dsa)) goto fail;
      if (strcmp(dsa->item, "RHS") == 0)
      {  while (dsa->c != '\n')
            if (read_char(dsa)) goto fail;
         if (read_item(dsa)) goto fail;
         xassert(dsa->item[0] == '\0');
         if (read_rhs(dsa)) goto fail;
         xassert(dsa->c != ' ');
         if (read_item(dsa)) goto fail;
      }
      /* read RANGES section (optional) */
      if (strcmp(dsa->item, "RANGES") == 0)
      {  while (dsa->c != '\n')
            if (read_char(dsa)) goto fail;
         if (read_item(dsa)) goto fail;
         xassert(dsa->item[0] == '\0');
         if (read_ranges(dsa)) goto fail;
         xassert(dsa->c != ' ');
         if (read_item(dsa)) goto fail;
      }
      /* read BOUNDS section (optional) */
      if (strcmp(dsa->item, "BOUNDS") == 0)
      {  while (dsa->c != '\n')
            if (read_char(dsa)) goto fail;
         if (read_item(dsa)) goto fail;
         xassert(dsa->item[0] == '\0');
         if (read_bounds(dsa)) goto fail;
         xassert(dsa->c != ' ');
         if (read_item(dsa)) goto fail;
      }
      /* check ENDATA indicator line */
      if (strcmp(dsa->item, "ENDATA") != 0)
      {  print("%s:%d: ENDATA indicator record missing", dsa->fname,
            dsa->count);
         goto fail;
      }
      if (lpx_get_class(dsa->lp) == LPX_MIP)
      {  int ni = lpx_get_num_int(dsa->lp);
         int nb = lpx_get_num_bin(dsa->lp);
         char s[50];
         if (nb == 0)
            strcpy(s, "none of");
         else if (ni == 1 && nb == 1)
            strcpy(s, "");
         else if (nb == 1)
            strcpy(s, "one of");
         else if (nb == ni)
            strcpy(s, "all of");
         else
            sprintf(s, "%d of", nb);
         print("lpx_read_freemps: %d integer column%s, %s which %s bina"
            "ry", ni, ni == 1 ? "" : "s", s, nb == 1 ? "is" : "are");
      }
      print("lpx_read_freemps: %d records were read", dsa->count);
      xfclose(dsa->fp);
      lpx_delete_index(dsa->lp);
      lpx_order_matrix(dsa->lp);
      return dsa->lp;
fail: if (dsa->lp != NULL) lpx_delete_prob(dsa->lp);
      if (dsa->fp != NULL) xfclose(dsa->fp);
      return NULL;
}

#undef dsa
#undef read_rows
#undef put_column
#undef read_columns
#undef set_rhs
#undef read_rhs
#undef set_range
#undef read_ranges
#undef read_bounds

/*----------------------------------------------------------------------
-- lpx_write_freemps - write problem data in free MPS format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_write_freemps(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_write_freemps writes problem data in free MPS format
-- to an output text file whose name is the character string fname.
--
-- *Returns*
--
-- If the operation was successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

#define row_name row_name1
#define col_name col_name1
#define mps_numb mps_numb1

static char *mps_name(char name[255+1])
{     int k;
      for (k = 0; name[k] != '\0'; k++)
         if (name[k] == ' ') name[k] = '_';
      return name;
}

static char *row_name(LPX *lp, int i, char rname[255+1])
{     const char *str;
      str = (i == 0 ? lpx_get_obj_name(lp) : lpx_get_row_name(lp, i));
      if (str == NULL)
         sprintf(rname, "R%07d", i);
      else
         strcpy(rname, mps_name((void *)str));
      return rname;
}

static char *col_name(LPX *lp, int j, char cname[8+1])
{     const char *str;
      str = lpx_get_col_name(lp, j);
      if (str == NULL)
         sprintf(cname, "C%07d", j);
      else
         strcpy(cname, mps_name((void *)str));
      return cname;
}

static char *mps_numb(double val, char numb[12+1])
{     int n;
      char str[255+1], *e;
      for (n = 12; n >= 6; n--)
      {  if (val != 0.0 && fabs(val) < 0.002)
#if 0
            sprintf(str, "%.*E", n, val);
#else
            /* n is number of desired decimal places, but in case of E
               format precision means number of digits that follow the
               decimal point */
            sprintf(str, "%.*E", n-1, val);
#endif
         else
            sprintf(str, "%.*G", n, val);
         xassert(strlen(str) <= 255);
         e = strchr(str, 'E');
         if (e != NULL) sprintf(e+1, "%d", atoi(e+1));
         if (strlen(str) <= 12) return strcpy(numb, str);
      }
      fault("lpx_write_freemps: unable to convert floating point number"
         " %g to character string", val);
      return NULL; /* make the compiler happy */
}

int lpx_write_freemps(LPX *lp, const char *fname)
{     FILE *fp;
      int wide = lpx_get_int_parm(lp, LPX_K_MPSWIDE);
      int skip = lpx_get_int_parm(lp, LPX_K_MPSSKIP);
      int marker = 0; /* intorg/intend marker count */
      int mip, make_obj, nrows, ncols, i, j, flag, *ndx;
      double *obj, *val;
      char rname[255+1], cname[255+1], numb[12+1];
      print("lpx_write_freemps: writing problem data to `%s'...", fname)
         ;
      /* open the output text file */
      fp = xfopen(fname, "w");
      if (fp == NULL)
      {  print("lpx_write_freemps: unable to create `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      /* determine whether the problem is LP or MIP */
      mip = (lpx_get_class(lp) == LPX_MIP);
      /* determine number of rows and number of columns */
      nrows = lpx_get_num_rows(lp);
      ncols = lpx_get_num_cols(lp);
      /* the problem should contain at least one row and one column */
      if (!(nrows > 0 && ncols > 0))
         fault("lpx_write_freemps: problem has no rows/columns");
      /* determine if the routine should output the objective row */
      make_obj = lpx_get_int_parm(lp, LPX_K_MPSOBJ);
      if (make_obj == 2)
      {  for (i = 1; i <= nrows; i++)
         {  int typx;
            lpx_get_row_bnds(lp, i, &typx, NULL, NULL);
            if (typx == LPX_FR)
            {  make_obj = 0;
               break;
            }
         }
      }
      /* write comment records (if required) */
      if (lpx_get_int_parm(lp, LPX_K_MPSINFO))
      {  const char *name = lpx_get_prob_name(lp);
         if (name == NULL) name = "UNKNOWN";
         fprintf(fp, "* Problem:    %s\n", name);
         fprintf(fp, "* Class:      %s\n", !mip ? "LP" : "MIP");
         fprintf(fp, "* Rows:       %d\n", nrows);
         if (!mip)
            fprintf(fp, "* Columns:    %d\n", ncols);
         else
            fprintf(fp, "* Columns:    %d (%d integer, %d binary)\n",
               ncols, lpx_get_num_int(lp), lpx_get_num_bin(lp));
         fprintf(fp, "* Non-zeros:  %d\n", lpx_get_num_nz(lp));
         fprintf(fp, "* Format:     Free MPS\n");
         fprintf(fp, "*\n");
      }
      /* write NAME indicator record */
      {  const char *name = lpx_get_prob_name(lp);
         if (name == NULL)
            fprintf(fp, "NAME\n");
         else
            fprintf(fp, "NAME %s\n", mps_name((void *)name));
      }
      /* write ROWS section */
      fprintf(fp, "ROWS\n");
      if (make_obj)
         fprintf(fp, " %c %s\n", 'N', row_name(lp, 0, rname));
      for (i = 1; i <= nrows; i++)
      {  int typx;
         lpx_get_row_bnds(lp, i, &typx, NULL, NULL);
         switch (typx)
         {  case LPX_FR: typx = 'N'; break;
            case LPX_LO: typx = 'G'; break;
            case LPX_UP: typx = 'L'; break;
            case LPX_DB: typx = 'E'; break;
            case LPX_FX: typx = 'E'; break;
            default: xassert(typx != typx);
         }
         fprintf(fp, " %c %s\n", typx, row_name(lp, i, rname));
      }
      /* prepare coefficients of the objective function */
      obj = xcalloc(1+ncols, sizeof(double));
      obj[0] = lpx_get_obj_coef(lp, 0);
      for (j = 1; j <= ncols; j++)
         obj[j] = lpx_get_obj_coef(lp, j);
      ndx = xcalloc(1+ncols, sizeof(int));
      val = xcalloc(1+ncols, sizeof(double));
      for (i = 1; i <= nrows; i++)
#if 0
      {  double ci = lpx_get_row_coef(lp, i);
#else
      {  double ci = 0.0;
#endif
         if (ci != 0.0)
         {  int len, t;
            len = lpx_get_mat_row(lp, i, ndx, val);
            for (t = 1; t <= len; t++)
               obj[ndx[t]] += ci * val[t];
         }
      }
      xfree(ndx);
      xfree(val);
      for (j = 0; j <= ncols; j++)
         if (fabs(obj[j]) < 1e-12) obj[j] = 0.0;
      /* write COLUMNS section */
      fprintf(fp, "COLUMNS\n");
      ndx = xcalloc(1+nrows, sizeof(int));
      val = xcalloc(1+nrows, sizeof(double));
      for (j = 1; j <= ncols; j++)
      {  int nl = 1, iv, len, t;
         col_name(lp, j, cname);
         iv = (mip && lpx_get_col_kind(lp, j) == LPX_IV);
         if (iv && marker % 2 == 0)
         {  /* open new intorg/intend group */
            marker++;
            fprintf(fp, " M%07d 'MARKER' 'INTORG'\n", marker);
         }
         else if (!iv && marker % 2 == 1)
         {  /* close the current intorg/intend group */
            marker++;
            fprintf(fp, " M%07d 'MARKER' 'INTEND'\n", marker);
         }
         /* obtain j-th column of the constraint matrix */
         len = lpx_get_mat_col(lp, j, ndx, val);
         ndx[0] = 0;
         val[0] = (make_obj ? obj[j] : 0.0);
         if (len == 0 && val[0] == 0.0 && !skip)
            fprintf(fp, " %s %s %s $ empty column\n", cname,
               row_name(lp, 1, rname), mps_numb(0.0, numb));
         for (t = val[0] != 0.0 ? 0 : 1; t <= len; t++)
         {  if (nl) fprintf(fp, " %s", cname);
            fprintf(fp, " %s %s", row_name(lp, ndx[t], rname),
               mps_numb(val[t], numb));
            if (wide) nl = 1 - nl;
            if (nl) fprintf(fp, "\n");
         }
         if (!nl) fprintf(fp, "\n");
      }
      if (marker % 2 == 1)
      {  /* close the last intorg/intend group (if not closed) */
         marker++;
         fprintf(fp, " M%07d 'MARKER' 'INTEND'\n", marker);
      }
      xfree(ndx);
      xfree(val);
      /* write RHS section */
      flag = 0;
      {  int nl = 1;
         for (i = make_obj ? 0 : 1; i <= nrows; i++)
         {  int typx;
            double lb, ub, rhs;
            if (i == 0)
               typx = LPX_FR, lb = ub = 0.0;
            else
               lpx_get_row_bnds(lp, i, &typx, &lb, &ub);
            switch (typx)
            {  case LPX_FR:
                  rhs = (make_obj && i == 0 ? obj[0] : 0.0); break;
               case LPX_LO:
                  rhs = lb; break;
               case LPX_UP:
                  rhs = ub; break;
               case LPX_DB:
                  rhs = (ub > 0.0 ? lb : ub); break;
               case LPX_FX:
                  rhs = lb; break;
               default:
                  xassert(typx != typx);
            }
            if (rhs == 0.0) continue;
            if (!flag) fprintf(fp, "RHS\n"), flag = 1;
            if (nl) fprintf(fp, " RHS1");
            fprintf(fp, " %s %s", row_name(lp, i, rname), mps_numb(rhs,
               numb));
            if (wide) nl = 1 - nl;
            if (nl) fprintf(fp, "\n");
         }
         if (!nl) fprintf(fp, "\n");
      }
      xfree(obj);
      /* write RANGES section */
      flag = 0;
      {  int nl = 1;
         for (i = 1; i <= nrows; i++)
         {  int typx;
            double lb, ub, rng;
            lpx_get_row_bnds(lp, i, &typx, &lb, &ub);
            if (typx != LPX_DB) continue;
            if (!flag) fprintf(fp, "RANGES\n"), flag = 1;
            if (nl) fprintf(fp, " RNG1");
            rng = (ub > 0.0 ? ub - lb : lb - ub);
            fprintf(fp, " %s %s", row_name(lp, i, rname), mps_numb(rng,
               numb));
            if (wide) nl = 1 - nl;
            if (nl) fprintf(fp, "\n");
         }
         if (!nl) fprintf(fp, "\n");
      }
      /* write BOUNDS section */
      flag = 0;
      {  for (j = 1; j <= ncols; j++)
         {  int typx;
            double lb, ub;
            lpx_get_col_bnds(lp, j, &typx, &lb, &ub);
            if (typx == LPX_LO && lb == 0.0) continue;
            if (!flag) fprintf(fp, "BOUNDS\n"), flag = 1;
            switch (typx)
            {  case LPX_FR:
                  fprintf(fp, " FR BND1 %s\n", col_name(lp, j, cname));
                  break;
               case LPX_LO:
                  fprintf(fp, " LO BND1 %s %s\n", col_name(lp, j,
                     cname), mps_numb(lb, numb));
                  break;
               case LPX_UP:
                  fprintf(fp, " MI BND1 %s\n", col_name(lp, j, cname));
                  fprintf(fp, " UP BND1 %s %s\n", col_name(lp, j,
                     cname), mps_numb(ub, numb));
                  break;
               case LPX_DB:
                  if (lb != 0.0)
                  fprintf(fp, " LO BND1 %s %s\n", col_name(lp, j,
                     cname), mps_numb(lb, numb));
                  fprintf(fp, " UP BND1 %s %s\n", col_name(lp, j,
                     cname), mps_numb(ub, numb));
                  break;
               case LPX_FX:
                  fprintf(fp, " FX BND1 %s %s\n", col_name(lp, j,
                     cname), mps_numb(lb, numb));
                  break;
               default:
                  xassert(typx != typx);
            }
         }
      }
      /* write ENDATA indicator record */
      fprintf(fp, "ENDATA\n");
      /* close the output text file */
      fflush(fp);
      if (ferror(fp))
      {  print("lpx_write_freemps: write error on `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      xfclose(fp);
      /* return to the calling program */
      return 0;
fail: /* the operation failed */
      if (fp != NULL) xfclose(fp);
      return 1;
}

#undef row_name
#undef col_name
#undef mps_numb

/*----------------------------------------------------------------------
-- lpx_print_prob - write problem data in plain text format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_print_prob(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_print_prob writes data from a problem object, which
-- the parameter lp points to, to an output text file, whose name is the
-- character string fname, in plain text format.
--
-- *Returns*
--
-- If the operation is successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

#define row_name row_name2
#define col_name col_name2

static char *row_name(LPX *lp, int i, char name[255+1])
{     const char *str;
      str = lpx_get_row_name(lp, i);
      if (str == NULL)
         sprintf(name, "R.%d", i);
      else
         strcpy(name, str);
      return name;
}

static char *col_name(LPX *lp, int j, char name[255+1])
{     const char *str;
      str = lpx_get_col_name(lp, j);
      if (str == NULL)
         sprintf(name, "C.%d", j);
      else
         strcpy(name, str);
      return name;
}

int lpx_print_prob(LPX *lp, const char *fname)
{     FILE *fp;
      int m, n, mip, i, j, len, t, type, *ndx;
      double coef, lb, ub, *val;
      char *str, name[255+1];
      print("lpx_write_prob: writing problem data to `%s'...", fname);
      fp = xfopen(fname, "w");
      if (fp == NULL)
      {  print("lpx_write_prob: unable to create `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      m = lpx_get_num_rows(lp);
      n = lpx_get_num_cols(lp);
      mip = (lpx_get_class(lp) == LPX_MIP);
      str = (void *)lpx_get_prob_name(lp);
      fprintf(fp, "Problem:    %s\n", str == NULL ? "(unnamed)" : str);
      fprintf(fp, "Class:      %s\n", !mip ? "LP" : "MIP");
      fprintf(fp, "Rows:       %d\n", m);
      if (!mip)
         fprintf(fp, "Columns:    %d\n", n);
      else
         fprintf(fp, "Columns:    %d (%d integer, %d binary)\n",
            n, lpx_get_num_int(lp), lpx_get_num_bin(lp));
      fprintf(fp, "Non-zeros:  %d\n", lpx_get_num_nz(lp));
      fprintf(fp, "\n");
      fprintf(fp, "*** OBJECTIVE FUNCTION ***\n");
      fprintf(fp, "\n");
      switch (lpx_get_obj_dir(lp))
      {  case LPX_MIN:
            fprintf(fp, "Minimize:");
            break;
         case LPX_MAX:
            fprintf(fp, "Maximize:");
            break;
         default:
            xassert(lp != lp);
      }
      str = (void *)lpx_get_obj_name(lp);
      fprintf(fp, " %s\n", str == NULL ? "(unnamed)" : str);
      coef = lpx_get_obj_coef(lp, 0);
      if (coef != 0.0)
         fprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, coef,
            "(constant term)");
      for (i = 1; i <= m; i++)
#if 0
      {  coef = lpx_get_row_coef(lp, i);
#else
      {  coef = 0.0;
#endif
         if (coef != 0.0)
            fprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, coef,
               row_name(lp, i, name));
      }
      for (j = 1; j <= n; j++)
      {  coef = lpx_get_obj_coef(lp, j);
         if (coef != 0.0)
            fprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, coef,
               col_name(lp, j, name));
      }
      fprintf(fp, "\n");
      fprintf(fp, "*** ROWS (CONSTRAINTS) ***\n");
      ndx = xcalloc(1+n, sizeof(int));
      val = xcalloc(1+n, sizeof(double));
      for (i = 1; i <= m; i++)
      {  fprintf(fp, "\n");
         fprintf(fp, "Row %d: %s", i, row_name(lp, i, name));
         lpx_get_row_bnds(lp, i, &type, &lb, &ub);
         switch (type)
         {  case LPX_FR:
               fprintf(fp, " free");
               break;
            case LPX_LO:
               fprintf(fp, " >= %.*g", DBL_DIG, lb);
               break;
            case LPX_UP:
               fprintf(fp, " <= %.*g", DBL_DIG, ub);
               break;
            case LPX_DB:
               fprintf(fp, " >= %.*g <= %.*g", DBL_DIG, lb, DBL_DIG,
                  ub);
               break;
            case LPX_FX:
               fprintf(fp, " = %.*g", DBL_DIG, lb);
               break;
            default:
               xassert(type != type);
         }
         fprintf(fp, "\n");
#if 0
         coef = lpx_get_row_coef(lp, i);
#else
         coef = 0.0;
#endif
         if (coef != 0.0)
            fprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, coef,
               "(objective)");
         len = lpx_get_mat_row(lp, i, ndx, val);
         for (t = 1; t <= len; t++)
            fprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, val[t],
               col_name(lp, ndx[t], name));
      }
      xfree(ndx);
      xfree(val);
      fprintf(fp, "\n");
      fprintf(fp, "*** COLUMNS (VARIABLES) ***\n");
      ndx = xcalloc(1+m, sizeof(int));
      val = xcalloc(1+m, sizeof(double));
      for (j = 1; j <= n; j++)
      {  fprintf(fp, "\n");
         fprintf(fp, "Col %d: %s", j, col_name(lp, j, name));
         if (mip)
         {  switch (lpx_get_col_kind(lp, j))
            {  case LPX_CV:
                  break;
               case LPX_IV:
                  fprintf(fp, " integer");
                  break;
               default:
                  xassert(lp != lp);
            }
         }
         lpx_get_col_bnds(lp, j, &type, &lb, &ub);
         switch (type)
         {  case LPX_FR:
               fprintf(fp, " free");
               break;
            case LPX_LO:
               fprintf(fp, " >= %.*g", DBL_DIG, lb);
               break;
            case LPX_UP:
               fprintf(fp, " <= %.*g", DBL_DIG, ub);
               break;
            case LPX_DB:
               fprintf(fp, " >= %.*g <= %.*g", DBL_DIG, lb, DBL_DIG,
                  ub);
               break;
            case LPX_FX:
               fprintf(fp, " = %.*g", DBL_DIG, lb);
               break;
            default:
               xassert(type != type);
         }
         fprintf(fp, "\n");
         coef = lpx_get_obj_coef(lp, j);
         if (coef != 0.0)
            fprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, coef,
               "(objective)");
         len = lpx_get_mat_col(lp, j, ndx, val);
         for (t = 1; t <= len; t++)
            fprintf(fp, "%*.*g %s\n", DBL_DIG+7, DBL_DIG, val[t],
               row_name(lp, ndx[t], name));
      }
      xfree(ndx);
      xfree(val);
      fprintf(fp, "\n");
      fprintf(fp, "End of output\n");
      fflush(fp);
      if (ferror(fp))
      {  print("lpx_write_prob: write error on `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      xfclose(fp);
      return 0;
fail: if (fp != NULL) xfclose(fp);
      return 1;
}

#undef row_name
#undef col_name

/*----------------------------------------------------------------------
-- lpx_print_sol - write LP problem solution in printable format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_print_sol(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_print_sol writes the current basic solution of an LP
-- problem, which is specified by the pointer lp, to a text file, whose
-- name is the character string fname, in printable format.
--
-- Information reported by the routine lpx_print_sol is intended mainly
-- for visual analysis.
--
-- *Returns*
--
-- If the operation was successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

int lpx_print_sol(LPX *lp, const char *fname)
{     FILE *fp;
      int what, round;
      print("lpx_print_sol: writing LP problem solution to `%s'...",
         fname);
      fp = xfopen(fname, "w");
      if (fp == NULL)
      {  print("lpx_print_sol: can't create `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      /* problem name */
      {  const char *name;
         name = lpx_get_prob_name(lp);
         if (name == NULL) name = "";
         fprintf(fp, "%-12s%s\n", "Problem:", name);
      }
      /* number of rows (auxiliary variables) */
      {  int nr;
         nr = lpx_get_num_rows(lp);
         fprintf(fp, "%-12s%d\n", "Rows:", nr);
      }
      /* number of columns (structural variables) */
      {  int nc;
         nc = lpx_get_num_cols(lp);
         fprintf(fp, "%-12s%d\n", "Columns:", nc);
      }
      /* number of non-zeros (constraint coefficients) */
      {  int nz;
         nz = lpx_get_num_nz(lp);
         fprintf(fp, "%-12s%d\n", "Non-zeros:", nz);
      }
      /* solution status */
      {  int status;
         status = lpx_get_status(lp);
         fprintf(fp, "%-12s%s\n", "Status:",
            status == LPX_OPT    ? "OPTIMAL" :
            status == LPX_FEAS   ? "FEASIBLE" :
            status == LPX_INFEAS ? "INFEASIBLE (INTERMEDIATE)" :
            status == LPX_NOFEAS ? "INFEASIBLE (FINAL)" :
            status == LPX_UNBND  ? "UNBOUNDED" :
            status == LPX_UNDEF  ? "UNDEFINED" : "???");
      }
      /* objective function */
      {  char *name;
         int dir;
         double obj;
         name = (void *)lpx_get_obj_name(lp);
         dir = lpx_get_obj_dir(lp);
         obj = lpx_get_obj_val(lp);
         fprintf(fp, "%-12s%s%s%.10g %s\n", "Objective:",
            name == NULL ? "" : name,
            name == NULL ? "" : " = ", obj,
            dir == LPX_MIN ? "(MINimum)" :
            dir == LPX_MAX ? "(MAXimum)" : "(" "???" ")");
      }
      /* main sheet */
      for (what = 1; what <= 2; what++)
      {  int mn, ij;
         fprintf(fp, "\n");
         fprintf(fp, "   No. %-12s St   Activity     Lower bound   Uppe"
            "r bound    Marginal\n",
            what == 1 ? "  Row name" : "Column name");
         fprintf(fp, "------ ------------ -- ------------- ------------"
            "- ------------- -------------\n");
         mn = (what == 1 ? lpx_get_num_rows(lp) : lpx_get_num_cols(lp));
         for (ij = 1; ij <= mn; ij++)
         {  const char *name;
            int typx, tagx;
            double lb, ub, vx, dx;
            if (what == 1)
            {  name = lpx_get_row_name(lp, ij);
               if (name == NULL) name = "";
               lpx_get_row_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               lpx_get_row_info(lp, ij, &tagx, &vx, &dx);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            else
            {  name = lpx_get_col_name(lp, ij);
               if (name == NULL) name = "";
               lpx_get_col_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               lpx_get_col_info(lp, ij, &tagx, &vx, &dx);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            /* row/column ordinal number */
            fprintf(fp, "%6d ", ij);
            /* row column/name */
            if (strlen(name) <= 12)
               fprintf(fp, "%-12s ", name);
            else
               fprintf(fp, "%s\n%20s", name, "");
            /* row/column status */
            fprintf(fp, "%s ",
               tagx == LPX_BS ? "B " :
               tagx == LPX_NL ? "NL" :
               tagx == LPX_NU ? "NU" :
               tagx == LPX_NF ? "NF" :
               tagx == LPX_NS ? "NS" : "??");
            /* row/column primal activity */
            fprintf(fp, "%13.6g ", vx);
            /* row/column lower bound */
            if (typx == LPX_LO || typx == LPX_DB || typx == LPX_FX)
               fprintf(fp, "%13.6g ", lb);
            else
               fprintf(fp, "%13s ", "");
            /* row/column upper bound */
            if (typx == LPX_UP || typx == LPX_DB)
               fprintf(fp, "%13.6g ", ub);
            else if (typx == LPX_FX)
               fprintf(fp, "%13s ", "=");
            else
               fprintf(fp, "%13s ", "");
            /* row/column dual activity */
            if (tagx != LPX_BS)
            {  if (dx == 0.0)
                  fprintf(fp, "%13s", "< eps");
               else
                  fprintf(fp, "%13.6g", dx);
            }
            /* end of line */
            fprintf(fp, "\n");
         }
      }
      fprintf(fp, "\n");
#if 1
      if (lpx_get_prim_stat(lp) != LPX_P_UNDEF &&
          lpx_get_dual_stat(lp) != LPX_D_UNDEF)
      {  int m = lpx_get_num_rows(lp);
         LPXKKT kkt;
         fprintf(fp, "Karush-Kuhn-Tucker optimality conditions:\n\n");
         lpx_check_kkt(lp, 1, &kkt);
         fprintf(fp, "KKT.PE: max.abs.err. = %.2e on row %d\n",
            kkt.pe_ae_max, kkt.pe_ae_row);
         fprintf(fp, "        max.rel.err. = %.2e on row %d\n",
            kkt.pe_re_max, kkt.pe_re_row);
         switch (kkt.pe_quality)
         {  case 'H':
               fprintf(fp, "        High quality\n");
               break;
            case 'M':
               fprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               fprintf(fp, "        Low quality\n");
               break;
            default:
               fprintf(fp, "        PRIMAL SOLUTION IS WRONG\n");
               break;
         }
         fprintf(fp, "\n");
         fprintf(fp, "KKT.PB: max.abs.err. = %.2e on %s %d\n",
            kkt.pb_ae_max, kkt.pb_ae_ind <= m ? "row" : "column",
            kkt.pb_ae_ind <= m ? kkt.pb_ae_ind : kkt.pb_ae_ind - m);
         fprintf(fp, "        max.rel.err. = %.2e on %s %d\n",
            kkt.pb_re_max, kkt.pb_re_ind <= m ? "row" : "column",
            kkt.pb_re_ind <= m ? kkt.pb_re_ind : kkt.pb_re_ind - m);
         switch (kkt.pb_quality)
         {  case 'H':
               fprintf(fp, "        High quality\n");
               break;
            case 'M':
               fprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               fprintf(fp, "        Low quality\n");
               break;
            default:
               fprintf(fp, "        PRIMAL SOLUTION IS INFEASIBLE\n");
               break;
         }
         fprintf(fp, "\n");
         fprintf(fp, "KKT.DE: max.abs.err. = %.2e on column %d\n",
            kkt.de_ae_max, kkt.de_ae_col);
         fprintf(fp, "        max.rel.err. = %.2e on column %d\n",
            kkt.de_re_max, kkt.de_re_col);
         switch (kkt.de_quality)
         {  case 'H':
               fprintf(fp, "        High quality\n");
               break;
            case 'M':
               fprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               fprintf(fp, "        Low quality\n");
               break;
            default:
               fprintf(fp, "        DUAL SOLUTION IS WRONG\n");
               break;
         }
         fprintf(fp, "\n");
         fprintf(fp, "KKT.DB: max.abs.err. = %.2e on %s %d\n",
            kkt.db_ae_max, kkt.db_ae_ind <= m ? "row" : "column",
            kkt.db_ae_ind <= m ? kkt.db_ae_ind : kkt.db_ae_ind - m);
         fprintf(fp, "        max.rel.err. = %.2e on %s %d\n",
            kkt.db_re_max, kkt.db_re_ind <= m ? "row" : "column",
            kkt.db_re_ind <= m ? kkt.db_re_ind : kkt.db_re_ind - m);
         switch (kkt.db_quality)
         {  case 'H':
               fprintf(fp, "        High quality\n");
               break;
            case 'M':
               fprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               fprintf(fp, "        Low quality\n");
               break;
            default:
               fprintf(fp, "        DUAL SOLUTION IS INFEASIBLE\n");
               break;
         }
         fprintf(fp, "\n");
      }
#endif
#if 1
      if (lpx_get_status(lp) == LPX_UNBND)
      {  int m = lpx_get_num_rows(lp);
         int k = lpx_get_ray_info(lp);
         fprintf(fp, "Unbounded ray: %s %d\n",
            k <= m ? "row" : "column", k <= m ? k : k - m);
         fprintf(fp, "\n");
      }
#endif
      fprintf(fp, "End of output\n");
      fflush(fp);
      if (ferror(fp))
      {  print("lpx_print_sol: can't write to `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      xfclose(fp);
      return 0;
fail: if (fp != NULL) xfclose(fp);
      return 1;
}

/*----------------------------------------------------------------------
-- lpx_print_ips - write interior point solution in printable format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_print_ips(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_print_ips writes the current interior point solution
-- of an LP problem, which the parameter lp points to, to a text file,
-- whose name is the character string fname, in printable format.
--
-- Information reported by the routine lpx_print_ips is intended mainly
-- for visual analysis.
--
-- *Returns*
--
-- If the operation was successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

int lpx_print_ips(LPX *lp, const char *fname)
{     FILE *fp;
      int what, round;
      print("lpx_print_ips: writing LP problem solution to `%s'...",
         fname);
      fp = xfopen(fname, "w");
      if (fp == NULL)
      {  print("lpx_print_ips: can't create `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      /* problem name */
      {  const char *name;
         name = lpx_get_prob_name(lp);
         if (name == NULL) name = "";
         fprintf(fp, "%-12s%s\n", "Problem:", name);
      }
      /* number of rows (auxiliary variables) */
      {  int nr;
         nr = lpx_get_num_rows(lp);
         fprintf(fp, "%-12s%d\n", "Rows:", nr);
      }
      /* number of columns (structural variables) */
      {  int nc;
         nc = lpx_get_num_cols(lp);
         fprintf(fp, "%-12s%d\n", "Columns:", nc);
      }
      /* number of non-zeros (constraint coefficients) */
      {  int nz;
         nz = lpx_get_num_nz(lp);
         fprintf(fp, "%-12s%d\n", "Non-zeros:", nz);
      }
      /* solution status */
      {  int status;
         status = lpx_ipt_status(lp);
         fprintf(fp, "%-12s%s\n", "Status:",
            status == LPX_T_UNDEF  ? "INTERIOR UNDEFINED" :
            status == LPX_T_OPT    ? "INTERIOR OPTIMAL" : "???");
      }
      /* objective function */
      {  char *name;
         int dir;
         double obj;
         name = (void *)lpx_get_obj_name(lp);
         dir = lpx_get_obj_dir(lp);
         obj = lpx_ipt_obj_val(lp);
         fprintf(fp, "%-12s%s%s%.10g %s\n", "Objective:",
            name == NULL ? "" : name,
            name == NULL ? "" : " = ", obj,
            dir == LPX_MIN ? "(MINimum)" :
            dir == LPX_MAX ? "(MAXimum)" : "(" "???" ")");
      }
      /* main sheet */
      for (what = 1; what <= 2; what++)
      {  int mn, ij;
         fprintf(fp, "\n");
         fprintf(fp, "   No. %-12s      Activity     Lower bound   Uppe"
            "r bound    Marginal\n",
            what == 1 ? "  Row name" : "Column name");
         fprintf(fp, "------ ------------    ------------- ------------"
            "- ------------- -------------\n");
         mn = (what == 1 ? lpx_get_num_rows(lp) : lpx_get_num_cols(lp));
         for (ij = 1; ij <= mn; ij++)
         {  const char *name;
            int typx /*, tagx */;
            double lb, ub, vx, dx;
            if (what == 1)
            {  name = lpx_get_row_name(lp, ij);
               if (name == NULL) name = "";
               lpx_get_row_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               vx = lpx_ipt_row_prim(lp, ij);
               dx = lpx_ipt_row_dual(lp, ij);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            else
            {  name = lpx_get_col_name(lp, ij);
               if (name == NULL) name = "";
               lpx_get_col_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               vx = lpx_ipt_col_prim(lp, ij);
               dx = lpx_ipt_col_dual(lp, ij);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            /* row/column ordinal number */
            fprintf(fp, "%6d ", ij);
            /* row column/name */
            if (strlen(name) <= 12)
               fprintf(fp, "%-12s ", name);
            else
               fprintf(fp, "%s\n%20s", name, "");
            /* two positions are currently not used */
            fprintf(fp, "   ");
            /* row/column primal activity */
            fprintf(fp, "%13.6g ", vx);
            /* row/column lower bound */
            if (typx == LPX_LO || typx == LPX_DB || typx == LPX_FX)
               fprintf(fp, "%13.6g ", lb);
            else
               fprintf(fp, "%13s ", "");
            /* row/column upper bound */
            if (typx == LPX_UP || typx == LPX_DB)
               fprintf(fp, "%13.6g ", ub);
            else if (typx == LPX_FX)
               fprintf(fp, "%13s ", "=");
            else
               fprintf(fp, "%13s ", "");
            /* row/column dual activity */
            fprintf(fp, "%13.6g", dx);
            /* end of line */
            fprintf(fp, "\n");
         }
      }
      fprintf(fp, "\n");
      fprintf(fp, "End of output\n");
      fflush(fp);
      if (ferror(fp))
      {  print("lpx_print_ips: can't write to `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      xfclose(fp);
      return 0;
fail: if (fp != NULL) xfclose(fp);
      return 1;
}

/*----------------------------------------------------------------------
-- lpx_print_mip - write MIP problem solution in printable format.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_print_mip(LPX *lp, char *fname);
--
-- *Description*
--
-- The routine lpx_print_mip writes a best known integer solution of
-- a MIP problem, which is specified by the pointer lp, to a text file,
-- whose name is the character string fname, in printable format.
--
-- Information reported by the routine lpx_print_mip is intended mainly
-- for visual analysis.
--
-- *Returns*
--
-- If the operation was successful, the routine returns zero. Otherwise
-- the routine prints an error message and returns non-zero. */

int lpx_print_mip(LPX *lp, const char *fname)
{     FILE *fp;
      int what, round;
      if (lpx_get_class(lp) != LPX_MIP)
         fault("lpx_print_mip: error -- not a MIP problem");
      print("lpx_print_mip: writing MIP problem solution to `%s'...",
         fname);
      fp = xfopen(fname, "w");
      if (fp == NULL)
      {  print("lpx_print_mip: can't create `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      /* problem name */
      {  const char *name;
         name = lpx_get_prob_name(lp);
         if (name == NULL) name = "";
         fprintf(fp, "%-12s%s\n", "Problem:", name);
      }
      /* number of rows (auxiliary variables) */
      {  int nr;
         nr = lpx_get_num_rows(lp);
         fprintf(fp, "%-12s%d\n", "Rows:", nr);
      }
      /* number of columns (structural variables) */
      {  int nc, nc_int, nc_bin;
         nc = lpx_get_num_cols(lp);
         nc_int = lpx_get_num_int(lp);
         nc_bin = lpx_get_num_bin(lp);
         fprintf(fp, "%-12s%d (%d integer, %d binary)\n", "Columns:",
            nc, nc_int, nc_bin);
      }
      /* number of non-zeros (constraint coefficients) */
      {  int nz;
         nz = lpx_get_num_nz(lp);
         fprintf(fp, "%-12s%d\n", "Non-zeros:", nz);
      }
      /* solution status */
      {  int status;
         status = lpx_mip_status(lp);
         fprintf(fp, "%-12s%s\n", "Status:",
            status == LPX_I_UNDEF  ? "INTEGER UNDEFINED" :
            status == LPX_I_OPT    ? "INTEGER OPTIMAL" :
            status == LPX_I_FEAS   ? "INTEGER NON-OPTIMAL" :
            status == LPX_I_NOFEAS ? "INTEGER EMPTY" : "???");
      }
      /* objective function */
      {  char *name;
         int dir;
         double mip_obj, lp_obj;
         name = (void *)lpx_get_obj_name(lp);
         dir = lpx_get_obj_dir(lp);
         mip_obj = lpx_mip_obj_val(lp);
         lp_obj = lpx_get_obj_val(lp);
#if 0 /* 27/XII-2005 */
         fprintf(fp, "%-12s%s%s%.10g %s %.10g (LP)\n", "Objective:",
            name == NULL ? "" : name,
            name == NULL ? "" : " = ", mip_obj,
            dir == LPX_MIN ? "(MINimum)" :
            dir == LPX_MAX ? "(MAXimum)" : "(" "???" ")", lp_obj);
#else
         fprintf(fp, "%-12s%s%s%.10g %s", "Objective:",
            name == NULL ? "" : name,
            name == NULL ? "" : " = ", mip_obj,
            dir == LPX_MIN ? "(MINimum)" :
            dir == LPX_MAX ? "(MAXimum)" : "(" "???" ")");
         if (lpx_get_status(lp) == LPX_OPT)
            fprintf(fp, " %.10g (LP)\n", lp_obj);
         else
            fprintf(fp, "\n");
#endif
      }
      /* main sheet */
      for (what = 1; what <= 2; what++)
      {  int mn, ij;
         fprintf(fp, "\n");
         fprintf(fp, "   No. %-12s      Activity     Lower bound   Uppe"
            "r bound\n",
            what == 1 ? "  Row name" : "Column name");
         fprintf(fp, "------ ------------    ------------- ------------"
            "- -------------\n");
         mn = (what == 1 ? lpx_get_num_rows(lp) : lpx_get_num_cols(lp));
         for (ij = 1; ij <= mn; ij++)
         {  const char *name;
            int kind, typx;
            double lb, ub, vx;
            if (what == 1)
            {  name = lpx_get_row_name(lp, ij);
               if (name == NULL) name = "";
               kind = LPX_CV;
               lpx_get_row_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               vx = lpx_mip_row_val(lp, ij);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            else
            {  name = lpx_get_col_name(lp, ij);
               if (name == NULL) name = "";
               kind = lpx_get_col_kind(lp, ij);
               lpx_get_col_bnds(lp, ij, &typx, &lb, &ub);
               round = lpx_get_int_parm(lp, LPX_K_ROUND);
               lpx_set_int_parm(lp, LPX_K_ROUND, 1);
               vx = lpx_mip_col_val(lp, ij);
               lpx_set_int_parm(lp, LPX_K_ROUND, round);
            }
            /* row/column ordinal number */
            fprintf(fp, "%6d ", ij);
            /* row column/name */
            if (strlen(name) <= 12)
               fprintf(fp, "%-12s ", name);
            else
               fprintf(fp, "%s\n%20s", name, "");
            /* row/column kind */
            fprintf(fp, "%s  ",
               kind == LPX_CV ? " " : kind == LPX_IV ? "*" : "?");
            /* row/column primal activity */
            fprintf(fp, "%13.6g", vx);
            /* row/column lower and upper bounds */
            switch (typx)
            {  case LPX_FR:
                  break;
               case LPX_LO:
                  fprintf(fp, " %13.6g", lb);
                  break;
               case LPX_UP:
                  fprintf(fp, " %13s %13.6g", "", ub);
                  break;
               case LPX_DB:
                  fprintf(fp, " %13.6g %13.6g", lb, ub);
                  break;
               case LPX_FX:
                  fprintf(fp, " %13.6g %13s", lb, "=");
                  break;
               default:
                  xassert(typx != typx);
            }
            /* end of line */
            fprintf(fp, "\n");
         }
      }
      fprintf(fp, "\n");
#if 1
      if (lpx_mip_status(lp) != LPX_I_UNDEF)
      {  int m = lpx_get_num_rows(lp);
         LPXKKT kkt;
         fprintf(fp, "Integer feasibility conditions:\n\n");
         lpx_check_int(lp, &kkt);
         fprintf(fp, "INT.PE: max.abs.err. = %.2e on row %d\n",
            kkt.pe_ae_max, kkt.pe_ae_row);
         fprintf(fp, "        max.rel.err. = %.2e on row %d\n",
            kkt.pe_re_max, kkt.pe_re_row);
         switch (kkt.pe_quality)
         {  case 'H':
               fprintf(fp, "        High quality\n");
               break;
            case 'M':
               fprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               fprintf(fp, "        Low quality\n");
               break;
            default:
               fprintf(fp, "        SOLUTION IS WRONG\n");
               break;
         }
         fprintf(fp, "\n");
         fprintf(fp, "INT.PB: max.abs.err. = %.2e on %s %d\n",
            kkt.pb_ae_max, kkt.pb_ae_ind <= m ? "row" : "column",
            kkt.pb_ae_ind <= m ? kkt.pb_ae_ind : kkt.pb_ae_ind - m);
         fprintf(fp, "        max.rel.err. = %.2e on %s %d\n",
            kkt.pb_re_max, kkt.pb_re_ind <= m ? "row" : "column",
            kkt.pb_re_ind <= m ? kkt.pb_re_ind : kkt.pb_re_ind - m);
         switch (kkt.pb_quality)
         {  case 'H':
               fprintf(fp, "        High quality\n");
               break;
            case 'M':
               fprintf(fp, "        Medium quality\n");
               break;
            case 'L':
               fprintf(fp, "        Low quality\n");
               break;
            default:
               fprintf(fp, "        SOLUTION IS INFEASIBLE\n");
               break;
         }
         fprintf(fp, "\n");
      }
#endif
      fprintf(fp, "End of output\n");
      fflush(fp);
      if (ferror(fp))
      {  print("lpx_print_mip: can't write to `%s' - %s", fname,
            strerror(errno));
         goto fail;
      }
      xfclose(fp);
      return 0;
fail: if (fp != NULL) xfclose(fp);
      return 1;
}

/* eof */
