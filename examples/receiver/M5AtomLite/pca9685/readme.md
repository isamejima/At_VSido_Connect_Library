# 概要
[秋月電子通商](https://akizukidenshi.com) 様の "I2C接続16チャンネル サーボ&PWM駆動キット[AE-PCA9685]"　(以下PWM駆動キット)を使用したAtom Lite用V-sidoプロトコル受信サンプル.  
PWM駆動キットは PCA9685 を用いてPWMを生成している.  PCA9685 のライブラリは秋月電子通商の該当ページにあるArduinoライブラリのものを使用した.
このサンプル群ではPWM駆動キットを2枚接続し、32個のPWMサーボの動作を確認した.
 
# 構成

Atom Liteの以下のピンをPWM駆動キットに配線した構成で動作確認を行った.  
  
Atom Lite		PWM駆動キット  
 G21 (SCL)	<-> SCL  
 G25 (SDA)	<-> SDA  
 GND  		<-> GND  
 5V  		<-> VCC  

また、PWM駆動キットは別途用意した5V電源を電源端子に接続した.
また、ジャンパは以下のように設定した.  
	V-Vジャンパ : Open  
	I2Cのプルアップ用ジャンパ : Close  
	I2Cアドレス用ジャンパ : 片方の基板をA0のみClose,それ以外すべてOpen  

- bt_receiver  
  
	Atom Lite の Bluetooth 機能を使用したサンプル.  
    
- ethudp_receiver  
  
	[Atom PoE](https://docs.m5stack.com/en/atom/atom_poe) を用いてEthernet接続を行い、UDPを使用したサンプル.  
    
- serial2_receiver  

	Atom Lite の Grove 端子を使用し、シリアル通信をするサンプル.  
	[Tail485](https://docs.m5stack.com/en/atom/tail485) を使用し動作確認を行った.  
	

- wifiudp_receiver  
	Atom Lite の Wi-Fi 機能を使用したサンプル.  
	本サンプルでは、STA/AP モードを任意で切り替えて使用できる.


