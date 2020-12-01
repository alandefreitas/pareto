include mk/pgcc.mk

ifdef MARCH
  ARCH := $(MARCH)
  OPT_CFLAGS += -tp=$(ARCH)
else
  $(warning Using generic x64 cpu target. Consider setting MARCH to a value given by 'pgcc -tp')
  ARCH := x64
  OPT_CFLAGS += -tp=$(ARCH)
endif

