# -*- Makefile-gmake -*-

CCversion=$(shell $(CC) -v 2>&1)

## Is $(CC) a gcc variant?
ifneq (,$(findstring gcc,$(CCversion)))
  $(info Detected C compiler to be a GCC variant.)
  include mk/gcc.mk
endif

# Is $(CC) the Sun C compiler?
ifneq (,$(findstring Sun,$(CCversion)))
  $(info Detected C compiler to the Sun Studio C compiler.)
  include mk/suncc.mk
endif
