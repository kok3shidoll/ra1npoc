# ra1npoc  
checkra1n dump and poc for iOS  
A tool for re-jailbreak devices jailbroken by checkra1n/odysseyra1n on iOS/iPadOS/macOS platforms.  

## 注意  
これはcheckra1n 0.1337.xからダンプしたPayloadに基づいて作成された、iOS上でcheckra1nを動かすための実証コードです。  
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
- iOS 12 - 17  
    - 動作確認済 (lightning to USB camera adapter 経由)  

- iOS 9 - 17  
    - 動作確認済 (lightning to USB camera adapter + 電源供給)  


## ビルド  
```
git submodule update --init --recursive
make
```


## 実行  
```
Usage: ./ra1npoc15 [-r] [-hcyEsv] [-e <boot-args>] [-k <override_pongo>]
  mode:
	-r, --ra1npoc			    : start with legacy ra1npoc mode

  options:
	-c, --cleandfu			    : use clean dfu
	-y, --yolodfu			    : use download mode (yoloDFU)
	-E, --early-exit		    : exit after uploading Pongo
	-k, --override-pongo <path>	: override Pongo image
	-e, --extra-bootargs <args>	: replace bootargs
	-s, --safemode			    : enable safe mode
	-v, --verbose-boot		    : enable verbose boot

  help:
	-h, --help			: show usage
```
- `--ra1npoc`モードは旧ビルド互換です。  

### 例
- DFU modeからpongoOSを起動する  
```
./ra1npoc15 -rE
```

- DFU modeから任意のpongoOS.binを起動する  
```
./ra1npoc15 -rEk <Pongo.bin>
```

- Recovery modeからiOS 14のデバイスをverbose bootでre-jailbreakする  
```
./ra1npoc15 -rcv
```


## 使い方   
[ra1npoc - How to use](https://kok3shidoll.github.io/info/ra1npoc/usage.html)  


## Credit  
checkra1n team: checkra1n  
axi0mX: checkm8 exploit  

license: MIT  
