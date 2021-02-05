#/bin/sh

clang main.c iousb.c checkra1n_t8010_t8015.c -o ra1npoc_macosx -I./include -framework IOKit -framework CoreFoundation -Os -DHAVE_DEBUG
