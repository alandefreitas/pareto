ifeq ($(DEBUG), 0)
  OPT_CFLAGS := -O3 -std=c99 -Wall -Wcheck
else
  OPT_CFLAGS := -g -std=c99 -Wall -Wcheck
endif

AR := xiar
