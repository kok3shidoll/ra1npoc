# ra1npoc  
checkra1n dump and poc for iOS  

## note  
This poc uses the payload dumped from checkra1n 0.12.2 beta, but s8000 device used pongoOS from checkra1n 0.12.4 beta.

This tool is for testing purposes. Do not use it on a normal device.  


## support  
### target devices  
- iPhone 5s (s5l8960x): for iOS 12(.5.1)  
- iPhone 6s (s8000): for iOS 14(.5.1)  
- iPhone 7 (t8010): for iOS 14(.3)  
- iPhone 8 (t8015): for iOS 13(.5)  

*KPF has been modified to give xargs "rootdev=md0 -v"*  

### host side devices  
- iPhone 5s (iOS 12.5.1)  
    - Works (via lightning to USB camera adapter)  

- iPhone 8 (iOS 13.5)
    - Works (via lightning to USB camera adapter)  
    
- iPhone 5 (iOS 10.2.1)  
    - Works  
    *successful checkm8 and loaded stage2, but unable to send pongoOS (via lightning to USB camera adapter). However, this device is able to send pongoOS by switching to the lightning to USB 3 camera adapter with power supply.*  

- iPhone 5 (iOS 9.1)  
    - Not Works  
    *successful checkm8 and loaded stage2, but unable to send pongoOS (via lightning to USB camera adapter).*  


## build  
```
cd src/  
./mk_macosx.sh  
./mk_iphoneos_arm64.sh  
./mk_iphoneos.sh  
```
*In order to build for iphoneos, you need to copy some headers such as IOKit from the macOS SDK.*  


## definition lists  
`-DHAVE_DEBUG`  
- Enables the display of some messages.  
    
`-DIPHONEOS_ARM`  
- Allows iOS to connect to iOS devices. For lightning device, require the "lightning to usb camera adapter".  
    
`-DIPHONEOS_LOWSPEC`  
- Prevents some unnecessary functions from working so that they will work even when run on low spec devices (such as Apple A7).  


## run  
### A7 device  
```
ra1npoc --a7 s5l8960x_overwrite1 s5l8960x_overwrite2 s5l8960x_stage2 t8015_pongoOS  
```

### A9 device (s8000)  
```
ra1npoc --a9 /dev/null 8000_overwrite2 s8000_stage2 s8000_pongoOS_251  
```

### A10 device  
```
ra1npoc --a10 t8010_overwrite1 t8010_overwrite2 t8010_stage2 t8010_pongoOS  
```

### A11 device  
```
ra1npoc --a11 t8015_overwrite1 t8015_overwrite2 t8015_stage2 t8015_pongoOS  
```


## thanks  
checkra1n team: checkra1n  
axi0mX: checkm8 exploit  


license: MIT  
