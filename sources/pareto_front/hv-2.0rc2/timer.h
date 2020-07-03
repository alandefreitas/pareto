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
*************************************************************************/
#ifndef TIMER_H_
#define TIMER_H_

#include <float.h>

#define HUGE_TIME      FLT_MAX

typedef enum type_timer {REAL_TIME, VIRTUAL_TIME} TIMER_TYPE;

void Timer_start(void);
double Timer_elapsed_virtual(void);
double Timer_elapsed_real(void);
double Timer_elapsed(TIMER_TYPE type);
void Timer_stop(void);
void Timer_continue(void);

#endif // TIMER_H_
