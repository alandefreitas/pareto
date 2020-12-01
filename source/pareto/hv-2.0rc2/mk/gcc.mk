# -*- Makefile-gmake -*-
MISSING_MARCH_BEGIN=Cannot guess cpu type for gcc
MISSING_MARCH_END=. Please specify your cpu type, \
'make march=i686' typically works fine in most computers. \
For more fine tuning, consult compiler manual.


ifneq ($(uname_S),mingw)
GCC_MAJOR := $(shell $(CC) -E -dM - < /dev/null  | grep "__GNUC__" | cut -d\  -f3)
GCC_MINOR := $(shell $(CC) -E -dM - < /dev/null  | grep "__GNUC_MINOR__" | cut -d\  -f3)

gcc-guess-march = $(strip $(shell ${CC} -march=$(MARCH) -x c -S -\#\#\# - < /dev/null 2>&1 | \
                grep -o -e "march=[^'\"]*" | head -n 1 | \
                sed 's,march=,,'))
else
ifndef MARCH
MARCH=native
gcc-guess-march=native
endif
endif

WARN_CFLAGS = -pedantic -Wall -Wextra -Wvla
CFLAGS += -std=gnu99 $(WARN_CFLAGS)

ifeq ($(DEBUG), 0)
  OPT_CFLAGS := -O3 -funroll-loops -ffast-math -DNDEBUG -mfpmath=sse
# Options -msse -mfpmath=sse improve performance but are not portable.
# Options -fstandard-precision=fast -ftree-vectorize are not well
# supported in some versions/architectures.
else
  CFLAGS += -g3 -mfpmath=sse
endif

ifndef MARCH
  ifeq ($(GCC_MAJOR),2)
    $(error Please upgrade to a recent version of GCC (>= 4.0))
  endif
  ifeq ($(GCC_MAJOR),3)
    $(error $(MISSING_MARCH_BEGIN) 3$(MISSING_MARCH_END))
  endif
  ifeq ($(GCC_MINOR),0)	
    $(error $(MISSING_MARCH_BEGIN) 4.0$(MISSING_MARCH_END))
  endif
  ifeq ($(GCC_MINOR),1)
    $(error $(MISSING_MARCH_BEGIN) 4.1$(MISSING_MARCH_END))
  endif
  ifeq ($(GCC_MINOR),2)
    $(error $(MISSING_MARCH_BEGIN) 4.2$(MISSING_MARCH_END))
  else
    MARCH := native
    ARCH := $(gcc-guess-march)
    OPT_CFLAGS += -march=$(MARCH)
  endif
else
  ARCH := $(gcc-guess-march)
  OPT_CFLAGS += -march=$(MARCH)
endif
