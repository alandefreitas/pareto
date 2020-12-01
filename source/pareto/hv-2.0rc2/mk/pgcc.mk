ifeq ($(DEBUG), 0)
  OPT_CFLAGS := -fast -O3 -Msmartalloc -c99
else
  OPT_CFLAGS := -g -c99
endif
