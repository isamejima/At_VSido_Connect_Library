#include <At_Vsido_Connect_Library.h>
#include <M5Atom.h>
#include "AtomMotion.h"

#include "Arduino_Communication.h"
 
WiFiUDP udp;
At_Vsido_Connect_Library atvsdcon=At_Vsido_Connect_Library();
Arduino_Commnunication ardcon = Arduino_Commnunication();
AtomMotion Atom;

char packetBuffer[256]={0};

void VSD_isrUDP()
{
	int packetSize = udp.parsePacket();
 
	if (packetSize){
		
		udp.read(packetBuffer, packetSize);

		for(int i=0;i<packetSize;i++){
			if(atvsdcon.read1byte(packetBuffer[i])==false)continue;
		}

	}
}

void setup() {
  
  M5.begin(true, false, true);//引数はマイコンで変化　Atom liteの場合はUART,I2C,LEDの初期化の有無
  Atom.Init();     //sda  25     scl  21 

  delay(100);	//M5.beginのあとは他の処理の前に少し待機

  Serial.begin(115200);

  ardcon.init();

  udp.begin(localPort);  // UDP通信の開始(引数はポート番号)

  
  atvsdcon.servo_connected[1]=true;	//サーボ有効化

}


void checkALL(){//受信割込み用の関数
  //シリアル割込み
  //while ( VSD_SERIAL.available() )VSD_isrRx();
  //UDP割り込み処理を入れるならここ
  VSD_isrUDP();
}

void loop() {

  M5.update();
  
  
    for(int sid=0;sid<atvsdcon.MAXSERVO;sid++){
		//受信角度を読み込み
		int int_position = atvsdcon.servo_angles[sid];
		
		//受信角度をサーボに送信
		if(atvsdcon.servo_connected[sid]){
			int position2=(int_position+1800)*20/36+500;

			Atom.SetServoPulse(1, position2);

		}
		//V-Sidoプロトコルの受信割込み
		checkALL();
	}
}
