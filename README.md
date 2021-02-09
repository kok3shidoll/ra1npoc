# ra1npoc  
checkra1n dump and poc for iOS  

## note  
This poc uses the payload dumped from checkra1n 0.12.2 beta.  
This tool is for testing purposes. Do not use it on a normal device.  


## support  
### target devices  
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
```
ra1npoc --a10 t8010_t8015_overwrite t8010_overwrite t8010_stage2 t8010_pongoOS  
ra1npoc --a11 t8010_t8015_overwrite t8015_overwrite t8015_stage2 t8015_pongoOS  
```


## thanks  
checkra1n team: checkra1n  
axi0mX: checkm8 exploit  


license: MIT  
