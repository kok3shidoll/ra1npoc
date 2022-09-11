# ra1npoc  
checkra1n dump and poc for iOS  
A tool for re-jailbreak devices jailbroken by checkra1n/odysseyra1n on iOS/iPadOS/macOS platforms.  

## 注意  
これはcheckra1n 0.12.4 betaからダンプしたPayloadに基づいて作成された、iOS上でcheckra1nを動かすための実証コードです。  
このツールは検証用です。通常のデバイスでは実行しないでください。  


## サポート  
### JailbreakしたいiOSデバイス  
| chip | name |
|---------|----------|
| S5L8960 | Apple A7 |
| T7000 | Apple A8 |
| T7001 | Apple A8X |
| S8000 | Apple A9 |
| S8003 | Apple A9 |
| S8001 | Apple A9X |
| T8010 | Apple A10 |
| T8011 | Apple A10X |
| T8012 | Apple T2 |
| T8015 | Apple A11 |


### ホスト側のデバイス (このソフトウェアを実行する側)  
- iOS 12+  
    - 動作確認済 (lightning to USB camera adapter 経由)  

- iOS 9+  
    - 動作確認済 (lightning to USB camera adapter + 電源供給)  


## ビルド  
### Built-Inモード  
```
cd src/
make all
cd ../pongoOS/scripts
make ra1npoc
```

### 旧モード  
```
cd src/
make old
```

*iPhoneOS向けにビルドするために、IOKitなどの一部ヘッダーをmacOSからiPhoneOSのSDKにコピーする必要があります。*  


## 定義  
`IPHONEOS_ARM`  
- iOSデバイスからiOSデバイスに接続できるようにします。lightningデバイスの場合、接続にはlightning to USB camera adapterが必要となります。  

`BUILTIN_PAYLOAD`   
- Payloadをbuilt-inします。


## 実行  
### built-in  
```
ra1npoc [option]  
  -h, --help                    show usage
  -l, --list                    show list of supported devices
  -v, --verbose                 enable verbose boot
  -c, --cleandfu                use cleandfu [BETA]
  -d, --debug                   enable debug log
  -e, --extra-bootargs <args>   set extra bootargs
  -s, --special                 use special pongo_2.5.0-0cb6126f
```

### built-in (A9X)  
A9Xの場合、`-s`フラグを渡すことで、iPadOSが正常に起動するようになります。[checkra1n_issues#2074](https://github.com/checkra1n/BugTracker/issues/2074)  
```
ra1npoc -s
```


### 旧モードの場合  
### A7/A10-A11  
```
ra1npoc [--{chipname}] [{Soc}_overwrite {Soc}_stage1 {Soc}_stage2 pongoOS]  
```

### A9X 
```
ra1npoc [--a9x] [s8001_overwrite s8001_stage1 s8001_stage2 pongoOS.bin]

pongoterm
> /send /path/to/ramdisk
> ramdisk
> /send /path/to/kpf
> modload
> xargs rootdev=md0
> bootx
```

### A8/A9  
```
ra1npoc [--{chipname}] [/dev/null {Soc}_stage1 {Soc}_stage2 pongoOS]  
```


## 使い方   
[ra1npoc - How to use](https://dora2ios.github.io/info/ra1npoc/usage.html)  


## Credit  
checkra1n team: checkra1n  
axi0mX: checkm8 exploit  

license: MIT  
