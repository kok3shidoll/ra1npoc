#/bin/sh

clang -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS10.2.sdk -arch armv7 main.c iousb.c checkra1n_t8010_t8015.c -o ra1npoc_iphoneos -I./include -framework IOKit -framework CoreFoundation -Os -DHAVE_DEBUG -DIPHONEOS_ARM -DIPHONEOS_LOWSPEC
codesign -f -s - ra1npoc_iphoneos
