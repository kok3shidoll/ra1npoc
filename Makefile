
RPVERSION ?= 3.3.1-$(shell git rev-parse HEAD | cut -c1-8)

C_FLAGS     	?= -Iinclude -Wall $(CFLAGS) -DRPVERSION=\"$(RPVERSION)\"
LD_FLAGS    	?= -framework CoreFoundation -framework IOKit $(LDFLAGS)

IOS_C_FLAGS     ?= -DIPHONEOS_ARM -Iios_headers -Wno-availability
MAC_LD_FLAGS	?= -mmacosx-version-min=10.11
IOS_LD_FLAGS	?= -miphoneos-version-min=9.0
DEV_LD_FLAGS	?= -DDEVBUILD=1
REL_LD_FLAGS	?= -Wno-deprecated-declarations

RP_FLAGS    	?= -DRA1NPOC_MODE=1
#BR_FLAGS    	?= -DBAKERA1N_MODE=1

C_FLAGS     	+= -Ilz4/lib

.PHONY: all debug ios clean

all: debug ios
	
debug:	\
	main.c \
	helper/pongoterm.c \
	helper/recovery.c \
	io/iousb.c \
	exploit/common.c \
	exploit/old.c \
	exploit/new.c \
	lz4/lz4_main.c \
	lz4/lib/lz4.c \
	lz4/lib/lz4hc.c \
	
	rm -rf ios_headers
	mkdir ios_headers
	ln -s $(shell xcrun -sdk macosx -show-sdk-path)/usr/include/libkern ios_headers
	ln -s $(shell xcrun -sdk macosx -show-sdk-path)/System/Library/Frameworks/IOKit.framework/Headers ios_headers/IOKit
	cd exploit/shellcode/ && ./build.sh
	cd headers/ && ./build.sh
	cd lz4/lib/ && make clean
	cd lz4/lib/ && make macos
	xcrun -sdk macosx clang $(C_FLAGS) -o ra1npoc15_$@_macosx $^ $(LD_FLAGS) $(MAC_LD_FLAGS) $(DEV_LD_FLAGS) $(RP_FLAGS) -O3 -arch arm64 -arch x86_64
	xcrun -sdk iphoneos clang $(C_FLAGS) $(IOS_C_FLAGS) -o ra1npoc15_$@_ios $^ $(LD_FLAGS) $(IOS_LD_FLAGS) $(DEV_LD_FLAGS) $(RP_FLAGS) -O3 -arch arm64
	ldid -Sent.xml ra1npoc15_$@_ios
	
ios:	\
	main.c \
	helper/pongoterm.c \
	helper/recovery.c \
	io/iousb.c \
	exploit/common.c \
	exploit/old.c \
	exploit/new.c \
	lz4/lz4_main.c \
		
	rm -rf ios_headers
	mkdir ios_headers
	ln -s $(shell xcrun -sdk macosx -show-sdk-path)/usr/include/libkern ios_headers
	ln -s $(shell xcrun -sdk macosx -show-sdk-path)/System/Library/Frameworks/IOKit.framework/Headers ios_headers/IOKit
	cd exploit/shellcode/ && ./build.sh
	cd headers/ && ./build.sh
	cd lz4/lib/ && make clean
	cd lz4/lib/ && make ios
	xcrun -sdk iphoneos clang $(C_FLAGS) $(IOS_C_FLAGS) -o ra1npoc15_$@ $^ lz4/lib/lz4.o lz4/lib/lz4hc.o $(LD_FLAGS) $(IOS_LD_FLAGS) $(REL_LD_FLAGS) $(RP_FLAGS) -arch arm64
	ldid -Sent.xml ra1npoc15_$@
	
clean:
	cd lz4/lib/ && make clean
	rm -rf ios_headers
	rm -f ra1npoc15_*
	rm -f headers/legacy_kpf.h
	rm -f headers/legacy_ramdisk.h
