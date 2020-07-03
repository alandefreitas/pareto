include mk/gcc.mk

ifdef MARCH
  OPT_CFLAGS += -march=$(MARCH)
  ARCH := $(MARCH)
else ifeq ($(shell which gcc-mp-4.4 > /dev/null && echo y),y)
  CC := gcc-mp-4.4
  ARCH := $(gcc-guess-march)
  OPT_CFLAGS += -march=$(ARCH)
else ifeq ($(shell which gcc-4.2 > /dev/null && echo y),y)
  CC := gcc-4.2
  ARCH := native
  OPT_CFLAGS += -mtune=$(ARCH)
else
  $(warning No processor specific optimizations set. Consider defining OPT_CFLAGS manually.)
endif
