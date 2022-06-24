# 概要
[Atom Motion](https://docs.m5stack.com/en/atom/atom_motion) を使用したAtom Lite用V-sidoプロトコル受信サンプル.  
このサンプル群ではAtom Motionの上限である4個のPWMサーボの動作を確認した.
Atom Motionライブラリは公式ライブラリのものを使用
 
# 構成

- bt_receiver  
  
	Atom Lite の Bluetooth 機能を使用したサンプル.  
    Atom Motion に Atom Lite を接続した構成で動作確認を行った.
- ethudp_receiver  
  
	[Atom PoE](https://docs.m5stack.com/en/atom/atom_poe) を用いてEthernet接続を行い、UDPを使用したサンプル.  
    Atom　PoE に Atom Liteを接続し,　Atom Liteのピンの一部を引き出して Atom Motion に配線した構成で動作確認を行った.  
	引き出したピンを以下に示す  
	- G21 (SCL)
	- G25 (SDA)
	- GND  
	- 3V3  
  
	Atom Motion は内部マイコンの電源に 3V3 を使用しているが内部にレギュレータを持っておらず、バッテリからの給電ができないため、別途引いてやる必要がある.  
	Atom Motion のバッテリ電源と PoE の電源の衝突を避けるため、5Vのラインは配線していない.  
  
- serial2_receiver  

	Atom Lite の Grove 端子を使用し、シリアル通信をするサンプル.  
	[Tail485](https://docs.m5stack.com/en/atom/tail485) を使用し動作確認を行った.  
	Tail485 を使用する場合、Atom Motion側のバッテリ電源とTail485 の電源の衝突に気を配る必要がある.

- wifiudp_receiver  
	Atom Lite の Wi-Fi 機能を使用したサンプル.  
    Atom Motion に Atom Lite を接続した構成で動作確認を行った.  
	本サンプルでは、STA/AP モードを任意で切り替えて使用できる.


