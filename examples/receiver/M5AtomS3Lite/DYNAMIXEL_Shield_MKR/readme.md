# 概要
[DYNAMIXEL Shield for Arduino MKR](https://emanual.robotis.com/docs/en/parts/interface/mkr_shield/) を使用したAtomS3 Lite用V-sidoプロトコル受信サンプル.

# 構成

DYNAMIXEL Shield for Arduino MKR(以下MKR基板) に配線した構成で動作確認を行った.


	- G1   <->	RX
	- G2   <->	TX
	- G38 	<-> TX_EN
	- GND	<-> GND
	- 5V	<-> 5V



- ethudp_receiver

	[Atom PoE](https://docs.m5stack.com/en/atom/atom_poe) を用いてEthernet接続を行い、UDPを使用したサンプル.

- serial1_receiver

	Atom Lite S3 の USB 端子を使用し、シリアル通信をするサンプル.




