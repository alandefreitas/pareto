/*
 * resource.h
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Based on:
 * http://www.opengroup.org/onlinepubs/000095399/basedefs/sys/resource.h.html
 */
#ifndef	_RESOURCE_H_
#define	_RESOURCE_H_

#include <sys/time.h>			/* for struct timeval */

#define RUSAGE_SELF     (1<<0)
#define RUSAGE_CHILDREN (1<<1)

struct rusage
{
    struct timeval ru_utime;	/* user time used */
    struct timeval ru_stime;	/* system time used */
};

int getrusage(int who, struct rusage * usage);

#endif /* Not _RESOURCE_H_ */
