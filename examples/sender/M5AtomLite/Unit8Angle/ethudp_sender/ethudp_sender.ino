#include <At_Vsido_Connect_Sender.h>
#include <BluetoothSerial.h>
#include <M5Atom.h>
#include <Wire.h>
#include <SPI.h>

#include <Ethernet.h>
#include <EthernetUDP.h>

#include "M5_ANGLE8.h"

#define VSD_SERIAL SerialBT

BluetoothSerial SerialBT;
String          target_name = "VSidoConnect_Atom";

// M5Stack用 W5500搭載 LANモジュール
#define W5500_MOSI 33
#define W5500_MISO 23
#define W5500_SCK 22
#define W5500_CS 19
//#define W5500_RST 
//#define W5500_INTn 36

M5_ANGLE8 angle8=M5_ANGLE8();
#define GROVE_I2C_SDA 26
#define GROVE_I2C_SCL 32

At_Vsido_Connect_Sender atvsidosender = At_Vsido_Connect_Sender();

EthernetUDP udp;
IPAddress my_ip(192, 168, 4, 65);
int local_port = 8888;
IPAddress target_ip(192, 168, 4, 255);
int target_port = 8888;

char packetBuffer[255];
unsigned char sendbuf[255];

unsigned long loop_interval_ms=20;
unsigned long last_update_ms=0;
unsigned long elapsed_ms=0;

void VSD_isrRx() {
	while (VSD_SERIAL.available()) {  // 受信確認
		unsigned char ch = VSD_SERIAL.read();  // 1文字だけ読み込む
		if (atvsidosender.read1byte(ch) == false) {
			continue;
		}
		bool ret = atvsidosender.unpackPacket();
		if (ret == true) {}
	}
}


void VSDUDPisrRx() {
// 利用可能なパケット長を確認
    int packetSize = udp.parsePacket();
    //Serial.printf("packetSize is %d\r\n",packetSize);

    if (packetSize>0)
    {
        // 送信元(remote)のIPアドレス・ポートを確認
        IPAddress r_ip = udp.remoteIP();
        int r_port = udp.remotePort();
        
        // パケット読出し
        udp.read(packetBuffer, packetSize);

        for (int i = 0; i < packetSize; i++)
        {
            char ch = packetBuffer[i];
            if (atvsidosender.read1byte(ch) == false)
                continue;

            // 解析を行う
            if (atvsidosender.unpackPacket() == false)
                continue;

            //追加でなにかやる

        }
    }
}

void setup_bt() {
		Serial.println(String("Connecting to ") + target_name + " ...");

	// bt初期化
	SerialBT.begin("VSidoConnect_AtomBTSender", true);

	bool connected = SerialBT.connect(target_name);

	if (connected) {
		Serial.println("Connected Succesfully!");
	} else {
		while (!SerialBT.connected(10000)) {
			Serial.println("Failed to connect. retry ...");
		}
	}
}

void setup_udp() {
    Serial.println("udp setup");
    SPI.begin(W5500_SCK, W5500_MISO, W5500_MOSI, W5500_CS);

    Ethernet.init(W5500_CS);

/*    byte mac[] =
    {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
        */
    //受信側と同一だとうまく通信できないので、変更
    byte mac[] =
    {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE};        

    Serial.println("start udp server:");
    Serial.println(my_ip);
    Ethernet.begin(mac, my_ip);
    udp.begin(local_port);
}


void setup() {
	//引数はマイコンで変化　Atom liteの場合はUART,I2C,LEDの初期化の有無
	M5.begin(true, false, true);

	// M5.beginのあとは他の処理の前に念のため少し待機
	delay(100);

  //起動中は赤LED
	M5.dis.drawpix(0, 0xff0000);
	delay(50);

  //8angle unitと通信
  Wire1.begin(GROVE_I2C_SDA,GROVE_I2C_SCL);
  while (!angle8.begin()) {
        Serial.println("angle8 Connect Error");
        delay(100);
    }

	for (int i = 0; i <= ANGLE8_TOTAL_LED; i++) {
    angle8.setLEDColor(i,0x000000,0);  
        delay(10);
  }


//	setup_bt();
	setup_udp();  
	delay(50);

	//サーボid有効化
	for (int i = 1; i <= 8; i++) {
		atvsidosender.servo_connected[i] = true;
    angle8.setLEDColor(i-1,0x00ff00,30); //unitのLEDを点灯
    delay(10);
	}

	// setup最後に緑色LEDに
	M5.dis.drawpix(0, 0x00ff00);
}


void checkALL() {  //受信割込み用の関数

	//シリアル割込み
	while (VSD_SERIAL.available())
		VSD_isrRx();

  VSDUDPisrRx();
}

void loop() {
	M5.update();


  elapsed_ms = millis() - last_update_ms;

	if (elapsed_ms >= loop_interval_ms) {
    for (uint8_t i = 0; i < ANGLE8_TOTAL_ADC; i++) {
        uint16_t adc_v=angle8.getAnalogInput(i, _12bit);//0~4095
        uint16_t r_adc_v=4095-adc_v;
//        int16_t vsido_val=map(r_adc_v,0,4095,0,1000);
        int16_t vsido_val=map(r_adc_v,0+10,4095-10,0,1000);
        atvsidosender.servo_angles[i+1]=constrain(vsido_val,0,1000);
  }
  
   int send_len=0;
		atvsidosender.genObjectPacket(2, sendbuf, &send_len);
    if(send_len>0){

/*
    for(int i=0;i<send_len;i++){
    Serial.printf("%02x ",sendbuf[i]);
    }
    Serial.println("");
*/
      udp.beginPacket(target_ip, target_port);
			udp.write(sendbuf, send_len);
			udp.endPacket();

    }

		last_update_ms = millis();
	}

	checkALL();
}
