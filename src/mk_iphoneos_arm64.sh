#/bin/sh

xcrun -sdk iphoneos clang main.c iousb.c checkra1n_common.c checkra1n_t8010_t8015.c checkra1n_s8000.c -o ra1npoc_iphoneos_arm64 -I./include -framework IOKit -framework CoreFoundation -Os -DHAVE_DEBUG -DIPHONEOS_ARM -DIPHONEOS_LOWSPEC -arch arm64

codesign -f -s - ra1npoc_iphoneos_arm64
