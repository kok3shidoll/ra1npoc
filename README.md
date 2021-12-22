# ra1npoc  
checkra1n dump and poc for iOS  

## 注意  
これはcheckra1n 0.12.4 betaからダンプしたPayloadに基づいて作成された、iOS上でcheckra1nを動かすための実証コードです。  
このツールは検証用です。通常のデバイスでは実行しないでください。  

*KPFはxargsに `rootdev=md0 -v`を渡すように変更されています。*    


## サポート  
### JailbreakしたいiOSデバイス (以下、対象デバイス): (テスト済みバージョン)  
- iPhone 7 (t8010): iOS 14(.3)  
- iPhone 8 (t8015): iOS 13(.5)  


### ホスト側のデバイス (このソフトウェアを実行する側)  
- iPhone 5s (iOS 12.5.1)  
    - 動作確認済 (lightning to USB camera adapter 経由)  

- iPhone 8 (iOS 13.5)  
    - 動作確認済 (lightning to USB camera adapter 経由)  

- iPhone 5 (iOS 10.2.1)  
    - 動作確認済 (lightning to USB camera adapter + 電源供給)  

- iPhone 5s (iOS 9.1)  
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
### A7 device  
```
ra1npoc --a7 [s5l8960x_overwrite1 s5l8960x_overwrite2 s5l8960x_stage2 t8015_pongoOS]  
```

### A9 device (s8000)  
```
ra1npoc --a9 [/dev/null 8000_overwrite2 s8000_stage2 t8010_pongoOS]  
```

### A10 device  
```
ra1npoc --a10 [t8010_overwrite1 t8010_overwrite2 t8010_stage2 t8010_pongoOS]  
```

### A11 device  
```
ra1npoc --a11 [t8015_overwrite1 t8015_overwrite2 t8015_stage2 t8015_pongoOS]  
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
