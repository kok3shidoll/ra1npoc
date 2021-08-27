# ra1npoc  
checkra1n dump and poc for iOS  

## 注意  
これはcheckra1n 0.12.2 betaからダンプしたPayloadに基づいて作成された、iOS上でcheckra1nを動かすための実証コードです。  
このツールは検証用です。通常のデバイスでは実行しないでください。  


## サポート  
### ターゲットデバイス (脱獄される側)  
- iPhone 5s (s5l8960x): iOS 12(.5.1)  
- iPhone 6s (s8000): iOS 14(.3)  
- iPhone 7 (t8010): iOS 14(.3)  
- iPhone 8 (t8015): iOS 13(.5)  

*KPF has been modified to give xargs `rootdev=md0 -v`*  
*KPFはxargsに `rootdev=md0 -v`を渡すように変更されています。*  


### ホスト側のデバイス (脱獄する側)  
- iPhone 5s (iOS 12.5.1)  
    - 動作確認済 (lightning to USB camera adapter)  

- iPhone 8 (iOS 13.5)
    - 動作確認済 (lightning to USB camera adapter)  

- iPhone 5 (iOS 10.2.1)  
    - 動作確認済 (lightning to USB camera adapter + 電源供給)  

- iPhone 5 (iOS 9.1)  
    - 動作確認済 (lightning to USB camera adapter + 電源供給)  


## ビルド  
```
cd src/  
./mk_macosx.sh  
./mk_iphoneos_arm64.sh  
./mk_iphoneos.sh  
```
*iPhoneOS向けにビルドする場合、IOKitなどの一部ヘッダーはmacOSのものからコピーする必要があります。*  


## 定義  
`-DHAVE_DEBUG`  
- いくつかのデバッグ用メッセージの表示を有効にします。  

`-DIPHONEOS_ARM`  
- iOSデバイスからiOSデバイスに接続できるようにします。lightningデバイスの場合、接続にはlightning to USB camera adapterが必要となります。  


## 実行  
### A7 device  
```
ra1npoc --a7 s5l8960x_overwrite1 s5l8960x_overwrite2 s5l8960x_stage2 t8015_pongoOS  
```

### A9 device (s8000)  
```
ra1npoc --a9 /dev/null 8000_overwrite2 s8000_stage2 t8010_pongoOS  
```

### A10 device  
```
ra1npoc --a10 t8010_overwrite1 t8010_overwrite2 t8010_stage2 t8010_pongoOS  
```

### A11 device  
```
ra1npoc --a11 t8015_overwrite1 t8015_overwrite2 t8015_stage2 t8015_pongoOS  
```


## 注意点
- 対象iOSデバイスを、lightning to USB camera adapter経由で接続したiOSデバイスを供給元(脱獄する側)としてDFU modeにする際にはLCDが点灯した状態のDFU modeに入れないと電源供給が足りず、DFU Modeに出来ない可能性があります。  
    - 対象デバイス: `iPhone 7` (脱獄される側)  
    - 解決策1: パソコンやモバイルバッテリーなどの十分な電源供給か可能な機器を使用してDFU Modeにした上でiOSデバイスに接続し直す。  
    - 解決策2: USBハブに電源供給を行い、バスパワーではなくセルフパワーで接続する。  

- 対象iOSデバイスを、lightning to USB camera adapter経由で接続したiOSデバイスを供給元(脱獄する側)としてDFU modeにする際に電源供給が足りず、DFU Modeに出来ない可能性があります。  
    - 対象デバイス: 残充電の少ない`iPhone 7`, `iPhone 8` (脱獄される側)  
    - 解決策1: パソコンやモバイルバッテリーなどの十分な電源供給か可能な機器を使用してDFU Modeにした上でiOSデバイスに接続し直す。  
    - 解決策2: USBハブに電源供給を行い、バスパワーではなくセルフパワーで接続する。  

- stage2からpongoOSを送信する際に電源供給が足りず、再接続が出来ない可能性があります。  
    - 対象デバイス: `iPhone 5`, `iOS 10以下のiPhone` (脱獄する側)  
    - 解決策: USBハブに電源供給を行い、バスパワーではなくセルフパワーで接続する。  

## thanks  
checkra1n team: checkra1n  
axi0mX: checkm8 exploit  

license: MIT  
