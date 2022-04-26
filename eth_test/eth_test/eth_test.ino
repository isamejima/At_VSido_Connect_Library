#include <M5Atom.h>
#include <SPI.h>
#include <EthernetUdp.h>
#include "Network_Setting.h"

#define SCK 22
#define MISO 23
#define MOSI 33
#define CS 19

EthernetUDP udp;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//MACはATOM LITE POEサンプルと同じものを仮設定

#define BUFSIZE 1024

void setup() {
  M5.begin(true, false, true);//引数はマイコンで変化　Atom liteの場合はUART,I2C,LEDの初期化の有無

	
  delay(100);	//M5.beginのあとは他の処理の前に少し待機

  Serial.begin(115200);

  SPI.begin(SCK, MISO, MOSI, -1);
  Ethernet.init(CS);
  Ethernet.begin(mac,ip);
  

  udp.begin(localPort);
}

void loop() {
  char packetBuffer[BUFSIZE]={0};

  M5.update();
  
  delay(100);

  int packetSize = udp.parsePacket();
 
  if (packetSize){

    udp.read(packetBuffer, packetSize);

    Serial.print("received: ");
    Serial.println(packetBuffer);

    delay(100);

  }

}