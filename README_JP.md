# ra1npoc  
checkra1n dump and poc for iOS  

## 注意  
これはcheckra1n 0.12.2 betaからダンプしたPayloadに基づいて作成された、iOS上でcheckra1nを動かすための実証コードです。  
しかし、s8000デバイスはcheckra1n 0.12.4 betaのpongoOSを使用した。
このツールは検証用です。通常のデバイスでは実行しないでください。  


## サポート  
### ターゲットデバイス (脱獄される側)  
- iPhone 5s (s5l8960x): iOS 12(.5.1)  
- iPhone 6s (s8000): iOS 14(.5.1)  
- iPhone 7 (t8010): iOS 14(.3)  
- iPhone 8 (t8015): iOS 13(.5)  

*KPF has been modified to give xargs `rootdev=md0 -v`*  
*KPFはxargsに `rootdev=md0 -v`を渡すように変更されています。*  


### ホスト側のデバイス (脱獄する側)  
- iPhone 5s (iOS 12.5.1)  
    - 動作確認済 (lightning to USB camera adapter経由)  

- iPhone 8 (iOS 13.5)
    - 動作確認済 (lightning to USB camera adapter経由)  

- iPhone 5 (iOS 10.2.1)  
    - 動作確認済*  
    *lightning to USB camera adapterを使用した場合、checkm8は成功しますが、stage2上からpongoOSを送信することができません。*  
    *しかし、電源供給のあるlightning to USB 3 camera adapterに接続し直すことで、stage2上からpongoOSを送信することが可能になります。*  

- iPhone 5 (iOS 9.1)  
    - 動作不可  
    *lightning to USB camera adapterを使用した場合、checkm8は成功しますが、stage2上からpongoOSを送信することができません。*  
    *lightning to USB 3 camera adapterはiOS 9.3未満では使用できません。*  


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

`-DIPHONEOS_LOWSPEC`  
- スペックの低いiOSデバイス上でも動作するように、一部の関数や動作を無効にします。  


## 実行  
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


## 注意点
- iPhoneからlightning to USB camera adapter経由でDFU modeにする際にはLCDが点灯した状態のDFU modeに入れないと電源供給が足りず、DFU Modeに出来ない可能性があります。  
    - 対象デバイス: 残充電の少ない`iPhone 7` (脱獄される側)  
    - 解決策: パソコンやモバイルバッテリーなどの十分な電源供給か可能な機器を使用してDFU Modeにした上でiOSデバイスに接続し直す。  

- iPhoneからlightning to USB camera adapter経由でDFU modeにする際に電源供給が足りず、DFU Modeに出来ない可能性があります。  
    - 対象デバイス: 残充電の少ない`iPhone 7`, `iPhone 8` (脱獄される側)  
    - 解決策: パソコンやモバイルバッテリーなどの十分な電源供給か可能な機器を使用してDFU Modeにした上でiOSデバイスに接続し直す。  

- stage2からpongoOSを送信する際に電源供給が足りず、再接続が出来ない可能性があります。  
    - 対象デバイス: `iPhone 5`, `iOS 10以下のiPhone` (脱獄する側)  
    - 解決策: 電源供給のあるlightning to USB 3 camera adapterに接続し直す。(iOS 9.3以降に限る)  


## thanks  
checkra1n team: checkra1n  
axi0mX: checkm8 exploit  


license: MIT  
