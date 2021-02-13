#/bin/sh

clang main.c iousb.c checkra1n_common.c checkra1n_t8010_t8015.c checkra1n_s8000.c checkra1n_s5l8960x.c -o ra1npoc_macosx -I./include -framework IOKit -framework CoreFoundation -Os -DHAVE_DEBUG
