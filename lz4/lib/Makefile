MACOS_FLAG += -mmacosx-version-min=10.11
MACOS_FLAG += -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk

IOS_FLAG += -miphoneos-version-min=9.0
IOS_FLAG += -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk

macos:
	clang $(MACOS_FLAG) -c lz4.c -arch arm64 -arch x86_64
	clang $(MACOS_FLAG) -c lz4hc.c -arch arm64 -arch x86_64
	
ios:
	clang $(IOS_FLAG) -c lz4.c -arch arm64
	clang $(IOS_FLAG) -c lz4hc.c -arch arm64

clean:
	rm -f *.o
