
IOS_CC		= xcrun -sdk iphoneos gcc
MACOSX_CC	= xcrun -sdk macosx gcc

CFLAGS		= -I./include -Wall
CFLAGS		+= -Os
IOS_FLAGS	= -DIPHONEOS_ARM -miphoneos-version-min=9.0
BUILTIN_FLAGS	= -DBUILTIN_PAYLOAD

LDFLAGS		= -framework IOKit -framework CoreFoundation

IOS_ARCH	= -arch armv7 -arch arm64

RA1NPOC_MACOSX_OBJECT	= ra1npoc_macosx
RA1NPOC_IOS_OBJECT	= ra1npoc_ios

CODESIGN	= ldid -S
STRIP		= strip

PD_GENERATOR	= builtin.sh

SOURCE		= \
                main.c \
                io/iousb.c \
                common/common.c \
                exploit/checkm8_arm64.c


ENTERDFU_MACOSX_OBJECT	= enterdfu_macosx
ENTERDFU_IOS_OBJECT	= enterdfu_ios

ENTERDFU_SOURCE	= \
               	enterdfu.c \
                io/iousb.c \
                common/common.c

.PHONY: all builtin enterdfu old clean

all: builtin enterdfu

builtin:
	./$(PD_GENERATOR)
	$(MACOSX_CC) $(CFLAGS) $(BUILTIN_FLAGS) $(LDFLAGS) $(SOURCE) -o $(RA1NPOC_MACOSX_OBJECT)
	$(IOS_CC) $(CFLAGS) $(IPHONEOS_FLAGS) $(BUILTIN_FLAGS) $(LDFLAGS) $(SOURCE) $(IOS_ARCH) -o $(RA1NPOC_IOS_OBJECT)
	$(STRIP) $(RA1NPOC_IOS_OBJECT)
	$(CODESIGN) $(RA1NPOC_IOS_OBJECT)

old:
	$(MACOSX_CC) $(CFLAGS) $(LDFLAGS) $(SOURCE) -o $(RA1NPOC_MACOSX_OBJECT)
	$(IOS_CC) $(CFLAGS) $(IOS_FLAGS) $(LDFLAGS) $(SOURCE) $(IOS_ARCH) -o $(RA1NPOC_IOS_OBJECT)
	$(STRIP) $(RA1NPOC_IOS_OBJECT)
	$(CODESIGN) $(RA1NPOC_IOS_OBJECT)

enterdfu:
	$(MACOSX_CC) $(CFLAGS) $(LDFLAGS) $(ENTERDFU_SOURCE) -o $(ENTERDFU_MACOSX_OBJECT)
	$(IOS_CC) $(CFLAGS) $(IPHONEOS_FLAGS) $(LDFLAGS) $(ENTERDFU_SOURCE) $(IOS_ARCH) -o $(ENTERDFU_IOS_OBJECT)
	$(STRIP) $(ENTERDFU_IOS_OBJECT)
	$(CODESIGN) $(ENTERDFU_IOS_OBJECT)

clean:
	-$(RM) $(RA1NPOC_MACOSX_OBJECT) $(RA1NPOC_IOS_OBJECT) $(ENTERDFU_MACOSX_OBJECT) $(ENTERDFU_IOS_OBJECT)
