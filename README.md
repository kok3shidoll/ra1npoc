# ra1npoc  
checkra1n dump and poc for iOS  

## 注意  
これはcheckra1n 0.12.4 betaからダンプしたPayloadに基づいて作成された、iOS上でcheckra1nを動かすための実証コードです。  
このツールは検証用です。通常のデバイスでは実行しないでください。  


## サポート  
### JailbreakしたいiOSデバイス  
| chip | name |   |
|---------|----------|----------|
| S5L8960 | Apple A7 | ✅ |
| T7000 | Apple A8 | ✅ |
| T7001 | Apple A8X | ❌ |
| S8000 | Apple A9 | ✅ |
| S8003 | Apple A9 | ❓ |
| S8001 | Apple A9X | ❌ |
| T8010 | Apple A10 | ✅ |
| T8011 | Apple A10X | ✅ |
| T8015 | Apple A11 | ✅ |


### ホスト側のデバイス (このソフトウェアを実行する側)  
- iPhone 5s (iOS 12.5.1)  
    - 動作確認済 (lightning to USB camera adapter 経由)  

- iPhone 8 (iOS 13.5)  
    - 動作確認済 (lightning to USB camera adapter 経由)  

- iPhone 5 (iOS 9.x, 10.x)  
    - 動作確認済 (lightning to USB camera adapter + 電源供給)  

- iPhone 5s (iOS 9.1, 9.3.3)  
    - 動作確認済 (lightning to USB camera adapter + 電源供給)  


## ビルド  
### 通常  
```
make all
```

### Built-In ビルド  
```
./builtin.sh
make all "CFLAGS+=-DBUILTIN_PAYLOAD"
```

*iPhoneOS向けにビルドする場合、IOKitなどの一部ヘッダーはmacOSのものからコピーする必要があります。*  


## 定義  
`DEBUG`  
- いくつかのデバッグ用メッセージの表示を有効にします。  

`IPHONEOS_ARM`  
- iOSデバイスからiOSデバイスに接続できるようにします。lightningデバイスの場合、接続にはlightning to USB camera adapterが必要となります。  

`BUILTIN_PAYLOAD`   
- Payloadをbuilt-inします。

## 実行  
- iOS 14環境で実行する場合、バイナリは`/usr/local/bin`以下に配置する必要があります。  

### A7/A10/A10X/A11  
```
ra1npoc [--a7 | --a10 | --a10x | --a11] [{Soc}_overwrite1 {Soc}_overwrite2 {Soc}_stage2 pongoOS]  
```

### A8/A9  
```
ra1npoc [--a8 | --a9] [/dev/null {Soc}_overwrite2 {Soc}_stage2 pongoOS]  
```

### built-in
```
ra1npoc [--a7 |--a8 | --a9 | --a10 | --a10x | --a11]  
```


## iOSで利用する際の注意点   
- 対象デバイスをlightning to USB camera adapter経由で脱獄するデバイス (このソフトウェアを実行する側) に接続する際、電源供給が足りず、対象デバイスをDFU Modeに出来ない可能性があります。  
    - 解決策1: パソコンやモバイルバッテリーなどの十分な電源供給か可能な機器を使用してDFU Modeにした上でiOSデバイスに接続し直す。  
    - 解決策2: USBハブに電源供給を行い、バスパワーではなくセルフパワーで接続する。  

- stage2からpongoOSを送信する際に電源供給が足りず、再接続が出来ない可能性があります。  
    - 影響のあるデバイス (このソフトウェアを実行する側): `iOS 10以下のiOSデバイス`  
    - 解決策: USBハブに電源供給を行い、バスパワーではなくセルフパワーで接続する。  

## Credit  
checkra1n team: checkra1n  
axi0mX: checkm8 exploit  

license: MIT  
