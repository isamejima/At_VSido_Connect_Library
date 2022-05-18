#include <M5Atom.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <At_Vsido_Connect_Library.h>

#include "AtomMotion.h"

#define AP		2
#define STA		3 

#define COMMUNICATION_MODE  AP		//AP,STA


#if COMMUNICATION_MODE == AP
	const char ssid[] = "ATOMWiFi";	// SSID
	const char pass[] = "Atompass";	// password
	const int localPort = 8888;			// ポート番号

	const IPAddress ip(192, 168, 4, 40);		// IPアドレス
	const IPAddress gateway(192, 168, 4, 0);	// (ゲートウェイ)
	const IPAddress subnet(255, 255, 255, 0);	// サブネットマスク

#elif COMMUNICATION_MODE == STA	
	const char ssid[] = "Knet";	// SSID
	const char pass[] = "9e29366d213523346617f1b851d9bf8e3ef2f05a62e42bd1a207fbbcdf5fd3f5";	// password
	const int localPort = 8888;			// ポート番号

	const IPAddress ip(192, 168, 11, 40);		// IPアドレス
	const IPAddress gateway(192, 168,11, 0);	// (ゲートウェイ)
	const IPAddress subnet(255, 255, 255, 0);	// サブネットマスク

#endif

AtomMotion Atom;
At_Vsido_Connect_Library atvsdcon=At_Vsido_Connect_Library();
 
WiFiUDP udp;

void wifi_init(void)
{
	switch(COMMUNICATION_MODE){

		case AP:
			WiFi.softAP(ssid, pass);
			delay(100);
			WiFi.softAPConfig(ip, gateway, subnet); 
		
			break;
		case STA:

			WiFi.begin(ssid, pass);
			
			while (WiFi.status() != WL_CONNECTED) {
				delay(500);
			}
			
			WiFi.config(ip, gateway, subnet);
		
			break;

		
	}
}


void VSD_isrUDP()
{
	char packetBuffer[256]={0};
	int packetSize = udp.parsePacket();
 
	if (packetSize){
		
		udp.read(packetBuffer, packetSize);

		for(int i=0;i<packetSize;i++){
			if(atvsdcon.read1byte(packetBuffer[i])==false)continue;
		}

	}
}

void setup() {
  
  //引数はマイコンで変化　Atom liteの場合はUART,I2C,LEDの初期化の有無
  M5.begin(true, false, true);
  
  //Atom Motion初期化
  Atom.Init();      

  //M5.beginのあとは他の処理の前に少し待機
  delay(100);	
  
  //電源確認にLED点灯
  M5.dis.drawpix(0, 0xffffff);	


  wifi_init();

  // UDP通信の開始
  udp.begin(localPort);  
  
  //サーボid1を有効化
  atvsdcon.servo_connected[1]=true;	

}

void checkALL(){//受信割込み用の関数

  VSD_isrUDP();
}

void loop() {

  M5.update();
  
  
    for(int sid=0;sid<atvsdcon.MAXSERVO;sid++){
		//受信角度を読み込み
		int int_position = atvsdcon.servo_angles[sid];
		
		//受信角度をサーボに送信
		if(atvsdcon.servo_connected[sid]){
			//角度情報をサーボパルス幅へ変換
			int tar_position=(int_position+1800)*20/36+500;	

			//サーボへ書き込み
			Atom.SetServoPulse(1, tar_position);	

		}
		//V-Sidoプロトコルの受信割込み
		checkALL();
	}
}
