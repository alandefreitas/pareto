/*************************************************************************

 I/O functions

 ---------------------------------------------------------------------

                       Copyright (c) 2005, 2006
                  Carlos M. Fonseca <cmfonsec@dei.uc.pt>
             Manuel Lopez-Ibanez <manuel.lopez-ibanez@ulb.ac.be>
                    Luis Paquete <paquete@dei.uc.pt>

 This program is free software (software libre); you can redistribute
 it and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, you can obtain a copy of the GNU
 General Public License at:
                 http://www.gnu.org/copyleft/gpl.html
 or by writing to:
           Free Software Foundation, Inc., 59 Temple Place,
                 Suite 330, Boston, MA 02111-1307 USA

 ----------------------------------------------------------------------

*************************************************************************/

#include "io.h"
#include "string.h" /* strerror */
#include "errno.h" /* errno */

#define PAGE_SIZE 4096          /* allocate one page at a time      */
#define DATA_INC (PAGE_SIZE/sizeof(double))

int
read_data (const char *filename, double **data_p, 
           int *nobjs_p, int **cumsizes_p, int *nsets_p)
{
    FILE *instream;

    int nobjs = *nobjs_p;        /* number of objectives (and columns).  */
    int *cumsizes = *cumsizes_p; /* cumulative sizes of data sets.       */
    int nsets    = *nsets_p;     /* number of data sets.                 */
    double *data = *data_p;

    double number;

    int retval;			/* return value for fscanf */
    char newline[2];
    int ntotal;			/* the current element of (*datap) */

    int column, line;

    int datasize;
    int sizessize;

    int error = 0;

    if (filename == NULL) {
        instream = stdin;
        filename = "<stdin>"; /* used to diagnose errors.  */
    }
    else if (NULL == (instream = fopen (filename,"r"))) {
        errprintf ("%s: %s\n", filename, strerror (errno));
        exit (EXIT_FAILURE);
    }

    if (nsets == 0) {
        ntotal = 0;
        sizessize = 0;
        datasize = 0;
    } else {
        ntotal = nobjs * cumsizes[nsets - 1];
        sizessize = ((nsets - 1) / DATA_INC + 1) * DATA_INC;
        datasize  = ((ntotal - 1) / DATA_INC + 1) * DATA_INC;
    }

    /* if size is equal to zero, this is equivalent to free().  
       That is, reinitialize the data structures.  */
    cumsizes = realloc (cumsizes, sizessize * sizeof(int));
    data = realloc (data, datasize * sizeof(double));

    column = 0;
    line = 0;

    /* skip over leading whitespace, comments and empty lines.  */
    do { 
        line++;
        /* skip full lines starting with # */
        if (!fscanf (instream, "%1[#]%*[^\n\r]", newline))
            /* and whitespace */
            fscanf (instream, "%*[ \t]");
        retval = fscanf (instream, "%1[\r\n]", newline);
    } while (retval == 1);

    if (retval == EOF) { /* faster than !feof() */
        error = READ_INPUT_FILE_EMPTY;
        goto read_data_finish;
    }

    do {
        /* beginning of data set */
	if (nsets == sizessize) {
            sizessize += DATA_INC;
	    cumsizes = realloc (cumsizes, sizessize * sizeof(int));
        }

	cumsizes[nsets] = (nsets == 0) ? 0 : cumsizes[nsets - 1];
	
        do {
            /* beginning of row */
	    column = 0;		
            
	    do {
                /* new column */
                column++; 
		
                if (fscanf (instream, "%lf", &number) != 1) {
                    char buffer[64];
                    fscanf (instream, "%60[^ \t\r\n]", buffer);
                    errprintf ("%s: line %d column %d: "
                               "could not convert string `%s' to double", 
                               filename, line, column, buffer);
                    exit (EXIT_FAILURE);
                }

                if (ntotal == datasize) {
                    datasize += DATA_INC;
                    data = realloc (data, datasize * sizeof(double));
                }
                data[ntotal] = number;
                ntotal++;

#if DEBUG > 1
                fprintf (stderr, "%s:%d:%d(%d) %d (set %d) = "
                         point_printf_format "\n", 
                        filename, line, column, nobjs, 
                        cumsizes[nsets], nsets, (double)number);
#endif
                /* skip possible trailing whitespace */
                fscanf (instream, "%*[ \t]");
                retval = fscanf (instream, "%1[\r\n]", newline);
                /* We do not consider that '\r\n' starts a new set.  */
                if (retval == 1 && newline[0] == '\r')
		    fscanf (instream, "%*[\n]");
            } while (retval == 0);

	    if (!nobjs)
		nobjs = column;
            else if (column == nobjs)
                ; /* OK */
            else if (cumsizes[0] == 0) { /* just finished first row.  */
                errprintf ("%s: line %d: input has dimension %d"
                           " while reference point has dimension %d",
                           filename, line, column, nobjs);
                error = READ_INPUT_WRONG_INITIAL_DIM;
                goto read_data_finish;
            } else {
                errprintf ("%s: line %d has different number of columns (%d)"
                           " from first row (%d)\n", 
                           filename, line, column, nobjs);
                exit (EXIT_FAILURE);
 	    }
	    cumsizes[nsets]++;

            /* look for an empty line */
            line++;
            if (!fscanf (instream, "%1[#]%*[^\n\r]", newline))
                fscanf (instream, "%*[ \t]");
            retval = fscanf (instream, "%1[\r\n]", newline);

	} while (retval == 0);

	nsets++; /* new data set */

#if DEBUG > 1
	fprintf (stderr, "%s: set %d, read %d rows\n", 
                 filename, nsets, cumsizes[nsets - 1]);
#endif
        /* skip over successive empty lines */
        do { 
            line++;
            if (!fscanf (instream, "%1[#]%*[^\n\r]", newline))
                fscanf (instream, "%*[ \t]");
            retval = fscanf (instream, "%1[\r\n]", newline);
        } while (retval == 1);

    } while (retval != EOF); /* faster than !feof() */

    /* adjust to real size (saves memory but probably slower).  */
    cumsizes = realloc (cumsizes, nsets * sizeof(int));
    data = realloc (data, ntotal * sizeof(double));

read_data_finish:

    *nobjs_p = nobjs;
    *nsets_p = nsets;
    *cumsizes_p = cumsizes;
    *data_p = data;

    if (instream != stdin) 
        fclose (instream);

    return error;
}

/* From:

   Edition 0.10, last updated 2001-07-06, of `The GNU C Library
   Reference Manual', for Version 2.3.x.

   Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2001, 2002,
   2003 Free Software Foundation, Inc.
*/
void errprintf(const char *template,...)
{
    extern char *program_invocation_short_name;
    va_list ap;

    fprintf(stderr, "%s: error: ", program_invocation_short_name);
    va_start(ap,template);
    vfprintf(stderr, template, ap);
    va_end(ap);
    fprintf(stderr, "\n");

    exit(EXIT_FAILURE);
}
/* End of copyright The GNU C Library Reference Manual */

void warnprintf(const char *template,...)
{
    extern char *program_invocation_short_name;
    va_list ap;

    fprintf(stderr, "%s: warning: ", program_invocation_short_name);
    va_start(ap,template);
    vfprintf(stderr, template, ap);
    va_end(ap);
    fprintf(stderr, "\n");

}
