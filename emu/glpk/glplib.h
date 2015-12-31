/* glplib.h (low-level library routines) */

/***********************************************************************
*  This code is part of GNU Linear Programming Kit (GLPK).
*
*  Copyright (C) 2000, 01, 02, 03, 04, 05, 06, 07 Andrew Makhorin,
*  Department for Applied Informatics, Moscow Aviation Institute,
*  Moscow, Russia. All rights reserved. E-mail: <mao@mai2.rcnet.ru>.
*
*  GLPK is free software; you can redistribute it and/or modify it
*  under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  GLPK is distributed in the hope that it will be useful, but WITHOUT
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
*  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
*  License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with GLPK; see the file COPYING. If not, write to the Free
*  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
*  02110-1301, USA.
***********************************************************************/

#ifndef _GLPLIB_H
#define _GLPLIB_H

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bigmul _glp_lib_bigmul
void bigmul(int n, int m, unsigned short x[], unsigned short y[]);
/* multiply unsigned integer numbers of arbitrary precision */

#define bigdiv _glp_lib_bigdiv
void bigdiv(int n, int m, unsigned short x[], unsigned short y[]);
/* divide unsigned integer numbers of arbitrary precision */

#define ulong_t _glp_lib_ulong_t
typedef struct { unsigned int lo, hi; } ulong_t;
/* 64-bit unsigned integer data type */

#define uldiv_t _glp_lib_uldiv_t
typedef struct { ulong_t quot, rem; } uldiv_t;
/* result of 64-bit unsigned integer division */

#define ulset _glp_lib_ulset
ulong_t ulset(unsigned int hi, unsigned int lo);
/* construct an unsigned long integer */

#define uladd _glp_lib_uladd
ulong_t uladd(ulong_t x, ulong_t y);
/* add unsigned long integers */

#define ulsub _glp_lib_ulsub
ulong_t ulsub(ulong_t x, ulong_t y);
/* subtract unsigned long integers */

#define ulcmp _glp_lib_ulcmp
int ulcmp(ulong_t x, ulong_t y);
/* compare unsigned long integers */

#define ulmul _glp_lib_ulmul
ulong_t ulmul(ulong_t x, ulong_t y);
/* multiply unsigned long integers */

#define uldiv _glp_lib_uldiv
uldiv_t uldiv(ulong_t x, ulong_t y);
/* divide unsigned long integers */

#define lib_set_ptr _glp_lib_set_ptr
void lib_set_ptr(void *ptr);
/* store global pointer in TLS */

#define lib_get_ptr _glp_lib_get_ptr
void *lib_get_ptr(void);
/* retrieve global pointer from TLS */

typedef struct LIBENV LIBENV;
typedef struct LIBMEM LIBMEM;

#define LIB_MAX_OPEN 20
/* maximal number of simultaneously open streams */

struct LIBENV
{     /* library environmental block */
      char version[7+1];
      /* version string returned by the routine glp_version */
      /*--------------------------------------------------------------*/
      /* memory allocation */
      ulong_t mem_limit;
      /* maximal amount of memory (in bytes) available for dynamic
         allocation */
      LIBMEM *mem_ptr;
      /* pointer to the linked list of allocated memory blocks */
      int mem_count;
      /* total number of currently allocated memory blocks */
      int mem_cpeak;
      /* peak value of mem_count */
      ulong_t mem_total;
      /* total amount of currently allocated memory (in bytes; is the
         sum of the size field over all memory block descriptors) */
      ulong_t mem_tpeak;
      /* peak value of mem_total */
      /*--------------------------------------------------------------*/
      /* terminal output */
      int (*term_hook)(void *info, const char *s);
      /* user-defined routine to intercept terminal output */
      void *term_info;
      /* transit pointer passed to the routine term_hook */
      /*--------------------------------------------------------------*/
      /* input/output streams */
      FILE *file_slot[LIB_MAX_OPEN];
      /* file_slot[k], 0 <= k <= LIB_MAX_OPEN-1, is a pointer to k-th
         stream; file_slot[k] = NULL means that k-th slot is free */
      FILE *log_file;
      /* output stream used to hardcopy all terminal output; NULL means
         no hardcopying */
};

#define LIB_MEM_FLAG 0x20101960
/* memory block descriptor flag */

struct LIBMEM
{     /* memory block descriptor */
      int flag;
      /* descriptor flag */
      int size;
      /* size of block (in bytes, including descriptor) */
      LIBMEM *prev;
      /* pointer to previous memory block descriptor */
      LIBMEM *next;
      /* pointer to next memory block descriptor */
};

#define lib_init_env _glp_lib_init_env
int lib_init_env(void);
/* initialize library environment */

#define lib_link_env _glp_lib_link_env
LIBENV *lib_link_env(void);
/* retrieve pointer to library environmental block */

#define lib_free_env _glp_lib_free_env
int lib_free_env(void);
/* free library environment */

#define xputs _glp_lib_xputs
void xputs(const char *s);
/* write character string to the terminal */

#define xprintf _glp_lib_xprintf
void xprintf(const char *fmt, ...);
/* write formatted output to the terminal */

#define xprint1 _glp_lib_xprint1 

void xprint1(const char *fmt, ...); 
/* (obsolete) */

#define lib_print_hook _glp_lib_print_hook
void lib_print_hook(int (*func)(void *info, char *buf), void *info);
/* (obsolete) */

#define xfault _glp_lib_xfault
void xfault(const char *fmt, ...);
/* display error message and terminate execution */

#define xfault1 _glp_lib_xfault1
void xfault1(const char *fmt, ...);
/* (obsolete) */

#define lib_fault_hook _glp_lib_fault_hook
void lib_fault_hook(int (*func)(void *info, char *buf), void *info);
/* (obsolete) */

#define xassert(expr) \
      ((void)((expr) || (_xassert(#expr, __FILE__, __LINE__), 1)))
#define _xassert _glp_lib_xassert
void _xassert(const char *expr, const char *file, int line);
/* check for logical condition */

/* some processors need data to be properly aligned; the macro
   align_boundary defines the boundary that fits for all data types;
   the macro align_datasize enlarges the specified size of a data item
   to provide a proper alignment of immediately following data */

#define align_boundary sizeof(double)

#define align_datasize(size) ((((size) + (align_boundary - 1)) / \
      align_boundary) * align_boundary)

#define xmalloc _glp_lib_xmalloc
void *xmalloc(int size);
/* allocate memory block */

#define xcalloc _glp_lib_xcalloc
void *xcalloc(int n, int size);
/* allocate memory block */

#define xfree _glp_lib_xfree
void xfree(void *ptr);
/* free memory block */

#define xfopen _glp_lib_xfopen
FILE *xfopen(const char *fname, const char *mode);
/* open file */

#define xfclose _glp_lib_xfclose
void xfclose(FILE *fp);
/* close file */

#define lib_open_log _glp_lib_open_log
int lib_open_log(const char *fname);
/* open hardcopy file */

#define lib_close_log _glp_lib_close_log
int lib_close_log(void);
/* close hardcopy file */

#define xtime _glp_lib_xtime
ulong_t xtime(void);
/* determine the current universal time */

#define xdifftime _glp_lib_xdifftime
double xdifftime(ulong_t t1, ulong_t t0);
/* compute the difference between two time values */

#define str2int _glp_lib_str2int
int str2int(const char *str, int *val);
/* convert character string to value of int type */

#define str2num _glp_lib_str2num
int str2num(const char *str, double *val);
/* convert character string to value of double type */

#define strspx _glp_lib_strspx
char *strspx(char *str);
/* remove all spaces from character string */

#define strtrim _glp_lib_strtrim
char *strtrim(char *str);
/* remove trailing spaces from character string */

#define fp2rat _glp_lib_fp2rat
int fp2rat(double x, double eps, double *p, double *q);
/* convert floating-point number to rational number */

#endif

/* eof */
