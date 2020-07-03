include mk/gcc.mk

ifdef MARCH
  ARCH := $(MARCH)
  OPT_CFLAGS += -march=$(MARCH)
else
  ## gcc-4.2 does not support -march=native on OS X for some reason...
  ARCH := native
  OPT_CFLAGS += -mtune=$(ARCH)
endif

