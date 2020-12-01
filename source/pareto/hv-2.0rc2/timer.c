/*************************************************************************

 Simple timer functions.

 ---------------------------------------------------------------------

 Copyright (c) 2005, 2006, 2007 Manuel Lopez-Ibanez
 TeX: \copyright 2005, 2006, 2007 Manuel L{\'o}pez-Ib{\'a}{\~n}ez

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

 NOTES: based on source code from Thomas Stuetzle.

*************************************************************************/

#include <stdio.h>
#include <sys/time.h> /* for struct timeval */
#ifndef WIN32
#include <sys/resource.h> /* for getrusage */
#else
#include "resource.h"
#endif

#include "timer.h"

#define TIMER_CPUTIME(X) ( (double)X.ru_utime.tv_sec  +         \
                           (double)X.ru_stime.tv_sec  +         \
                          ((double)X.ru_utime.tv_usec +         \
                           (double)X.ru_stime.tv_usec ) * 1.0E-6)

#define TIMER_WALLTIME(X)  ( (double)X.tv_sec +         \
                             (double)X.tv_usec * 1.0E-6 )

static struct rusage res;
static struct timeval tp;
static double virtual_time, real_time;
static double stop_virtual_time, stop_real_time;

/*
 *  The virtual time of day and the real time of day are calculated and
 *  stored for future use.  The future use consists of subtracting these
 *  values from similar values obtained at a later time to allow the user
 *  to get the amount of time used by the backtracking routine.
 */

void Timer_start()
{
    gettimeofday (&tp, NULL );
    real_time =   TIMER_WALLTIME(tp);

    getrusage (RUSAGE_SELF, &res );
    virtual_time = TIMER_CPUTIME(res);
}

/*
 *  Return the time used in seconds (either
 *  REAL or VIRTUAL time, depending on ``type'').
 */
double Timer_elapsed_virtual (void)
{
    double timer_tmp_time;
    getrusage (RUSAGE_SELF, &res);
    timer_tmp_time = TIMER_CPUTIME(res) - virtual_time;

#if DEBUG >= 4
    if (timer_tmp_time  < 0.0) {
        fprintf(stderr, "%s: Timer_elapsed(): warning: "
                "negative increase in time ", __FILE__);
        fprintf(stderr, "(%.6g - %.6g = ",
                TIMER_CPUTIME(res) , virtual_time);
        fprintf(stderr, "%.6g)\n", timer_tmp_time);
    }
#endif

    return (timer_tmp_time < 0.0) ? 0 : timer_tmp_time;
}

double Timer_elapsed_real (void)
{
    double timer_tmp_time;
    
    gettimeofday (&tp, NULL);
    timer_tmp_time = TIMER_WALLTIME(tp) - real_time;

#if DEBUG >= 2
    if (timer_tmp_time  < 0.0) {
        fprintf(stderr, "%s: Timer_elapsed(): warning: "
                "negative increase in time ", __FILE__);
        fprintf(stderr, "(%.6g - %.6g = ",
                TIMER_WALLTIME(tp) , real_time);
        fprintf(stderr, "%.6g)\n", timer_tmp_time);
    }
#endif

    return (timer_tmp_time < 0.0) ? 0 : timer_tmp_time;
}

double Timer_elapsed( TIMER_TYPE type )
{
    return (type == REAL_TIME) 
        ? Timer_elapsed_real () 
        : Timer_elapsed_virtual ();
}

void Timer_stop(void)
{
    gettimeofday( &tp, NULL );
    stop_real_time =  TIMER_WALLTIME(tp);

    getrusage( RUSAGE_SELF, &res );
    stop_virtual_time = TIMER_CPUTIME(res);
}

void Timer_continue(void)
{
    double timer_tmp_time;

    gettimeofday( &tp, NULL );
    timer_tmp_time = TIMER_WALLTIME(tp) - stop_real_time;

#if DEBUG >= 2
    if (timer_tmp_time  < 0.0) {
        fprintf(stderr, "%s: Timer_continue(): warning: "
                "negative increase in time (%.6g - %.6g = %.6g)\n",
                __FILE__, TIMER_WALLTIME(tp), stop_real_time, timer_tmp_time);
    }
#endif

    if (timer_tmp_time > 0.0) real_time += timer_tmp_time;

    getrusage( RUSAGE_SELF, &res );
    timer_tmp_time =  TIMER_CPUTIME(res) - stop_virtual_time;

#if DEBUG >= 2
    if (timer_tmp_time  < 0.0) {
        fprintf(stderr, "%s: Timer_continue(): warning: "
                "negative increase in time (%.6g - %.6g = %.6g)\n",
                __FILE__, TIMER_CPUTIME(res),stop_virtual_time,timer_tmp_time);
    }
#endif

    if (timer_tmp_time > 0.0) virtual_time += timer_tmp_time;
}
