#include "Arduino_Communication.h"

Arduino_Commnunication::Arduino_Commnunication() {
}

//初期化
int Arduino_Commnunication::init(void)
{
	switch(COMMUNICATION_MODE){
		case ETH:
		
			SPI.begin(SCK, MISO, MOSI, -1);
			Ethernet.init(CS);
			Ethernet.begin(mac,ip);


			udp.begin(localPort);// UDP通信の開始(引数はポート番号)
			break;
		case AP:
			WiFi.softAP(ssid, pass);           // SSIDとパスの設定
			delay(100);                        // 追記：このdelayを入れないと失敗する場合がある
			WiFi.softAPConfig(ip, gateway, subnet); // IPアドレス、ゲートウェイ、サブネットマスクの設定
		
			udp.begin(localPort);  // UDP通信の開始(引数はポート番号)
			break;
		case STA:

			WiFi.begin(ssid, pass);
			
			while (WiFi.status() != WL_CONNECTED) {
				//Serial.print(".");
				delay(500);
			}

			//Serial.println(" connected");
			
			WiFi.config(ip, gateway, subnet);
			
			udp.begin(localPort);  // UDP通信の開始(引数はポート番号)
		
			break;

		default:	//未定義
			break;
		
	}
	return 0;
}