各フォルダのサンプルは以下の通りである。

・ether_Example - Ethernet　UDPを用いて通信を行い、PWMサーボを動作するサンプル。
・wifi_Example - Atom Lite本体のWi-Fiを用いてPWMサーボを動作するサンプル。AT/STAはどちらも対応。
・rs485_Example - RS485を用いて通信を行い、PWMサーボを動作するサンプル。




なお、このフォルダのサンプルプログラムは以下の構成で動作したものである。
・ATOM Lite
・ATOM Motion ※1
・PWMサーボモータ(ATOM MotionのS1に接続)

通信方法により、以下のモジュールを追加した。

・Atom　PoE　(ether_Example)　※2
・ATOM Tail485 (rs485_example)

※１ ATOM Motionのサンプルプログラムより、モータ動作用のコードを使用
※２ ATOM Motion とATOM PoEを作成した回路にて接続。





備考
---ATOM MotionとAtom PoEの接続について---

ATOM PoE内基板よりG21,G25,GND,3.3Vを引き出し、ATOM Motironに配線。
信号用のI2C線２本とGND、ATOM　Motion側の制御用STMマイコンの電源に3.3Vの配線を行い、動作を確認した。

PWMサーボの駆動はATOM Motion側のバッテリより供給される電力にて行う。






