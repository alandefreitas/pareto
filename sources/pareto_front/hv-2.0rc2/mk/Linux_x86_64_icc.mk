include mk/icc.mk

ifdef XARCH
  OPT_CFLAGS += -x$(XARCH)
  ARCH := $(XARCH)
else
  ARCH := native
  OPT_CFLAGS += -xHOST
endif
