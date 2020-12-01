include mk/suncc.mk

ifdef XTARGET
  ARCH := $(XTARGET)
  OPT_CFLAGS += -xtarget=$(ARCH)
else
  ifdef XARCH
    ARCH := $(XARCH)
    OPT_CFLAGS += -xarch=$(ARCH)
  else
    ARCH := native
    OPT_CFLAGS += -xtarget=$(ARCH)
  endif
endif
