#ifndef _HV_IO_H_
#define _HV_IO_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define point_printf_format "%-16.15g"

/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#  define  __attribute__(x)  /* NOTHING */
#endif

void 
errprintf(const char * template,...) 
/* enables the compiler to check the format string against the
   parameters */  __attribute__ ((format(printf, 1, 2)));

void warnprintf(const char *template,...)
/* enables the compiler to check the format string against the
   parameters */  __attribute__ ((format(printf, 1, 2)));

/* Error codes for read_data.  */
#define READ_INPUT_FILE_EMPTY -1
#define READ_INPUT_WRONG_INITIAL_DIM -2

int
read_data (const char *filename, double **data_p, 
           int *nobjs_p, int **cumsizes_p, int *nsets_p);

#endif
