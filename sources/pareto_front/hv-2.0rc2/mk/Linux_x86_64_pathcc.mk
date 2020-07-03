## Patchscales compiler is partially based on gccs front-end. It is therefore safe to use
## the gcc defaults. 

ifndef MARCH
  ## auto is for pathscale what native is for gcc:
  MARCH=auto
endif

include mk/gcc.mk
