
IPHONEOS_CC	= xcrun -sdk iphoneos gcc
MACOSX_CC	= xcrun -sdk macosx gcc

CFLAGS		= -I./include -Wall
CFLAGS		+= -Os
IPHONEOS_FLAGS	= -DIPHONEOS_ARM -miphoneos-version-min=9.0
BUILTIN_FLAGS	= -DBUILTIN_PAYLOAD

LDFLAGS		= -framework IOKit -framework CoreFoundation

IPHONEOS_ARCH	= -arch armv7 -arch arm64

MACOSX_OBJECT	= ra1npoc_macosx
IPHONEOS_OBJECT	= ra1npoc_iphoneos

CODESIGN	= ldid -S
STRIP		= strip

PD_GENERATOR	= builtin.sh

SOURCE		= \
                main.c \
                io/iousb.c \
                common/common.c \
                soc/t8010_t8015.c \
                soc/t7000_s8000.c \
                soc/s5l8960x.c


.PHONY: all builtin old clean

all: builtin

builtin:
	./$(PD_GENERATOR)
	$(MACOSX_CC) $(CFLAGS) $(BUILTIN_FLAGS) $(LDFLAGS) $(SOURCE) -o $(MACOSX_OBJECT)
	$(IPHONEOS_CC) $(CFLAGS) $(IPHONEOS_FLAGS) $(BUILTIN_FLAGS) $(LDFLAGS) $(SOURCE) $(IPHONEOS_ARCH) -o $(IPHONEOS_OBJECT)
	$(STRIP) $(IPHONEOS_OBJECT)
	$(CODESIGN) $(IPHONEOS_OBJECT)

old:
	$(MACOSX_CC) $(CFLAGS) $(LDFLAGS) $(SOURCE) -o $(MACOSX_OBJECT)
	$(IPHONEOS_CC) $(CFLAGS) $(IPHONEOS_FLAGS) $(LDFLAGS) $(SOURCE) $(IPHONEOS_ARCH) -o $(IPHONEOS_OBJECT)
	$(STRIP) $(IPHONEOS_OBJECT)
	$(CODESIGN) $(IPHONEOS_OBJECT)

clean:
	-$(RM) $(MACOSX_OBJECT) $(IPHONEOS_OBJECT)
