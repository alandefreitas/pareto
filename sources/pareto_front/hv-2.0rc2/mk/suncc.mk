ifeq ($(DEBUG), 0)
  OPT_CFLAGS := -fast -xO3 -xc99=all
else
  OPT_CFLAGS := -g -xc99=all
endif
