#include <At_Vsido_Connect_Library.h>
#include <Dynamixel2Arduino.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h>
#include <SPI.h>
#include <M5Stack.h>

#include "Adafruit_BNO08x_RVC.h"

#define USB_SERIAL Serial

#define IMU_SERIAL Serial1
#define IMU_TX 21
#define IMU_RX 22

#define DXL_SERIAL Serial2
#define DXL_TX 16
#define DXL_RX 17
#define DXL_DIR -1

#define SCK 18
#define CS 26
#define MISO 19
#define MOSI 23
#define RST 13

// V-SIdo Connectライブラリ
At_Vsido_Connect_Library atvsdcon = At_Vsido_Connect_Library();

// imu関係
Adafruit_BNO08x_RVC rvc = Adafruit_BNO08x_RVC();

//dynamixel関係
const float     DXL_PROTOCOL_VERSION = 2.0;
Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR);
using namespace ControlTableItem;

//PoE モジュール関係
// ethernetudp関係
EthernetUDP udp;
byte mac[] = { 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed };//macアドレス(仮)
const int localPort = 8888;  // ポート番号
const IPAddress ip(192, 168, 4, 40);       // IPアドレス(ゲートウェイも兼ねる)
const IPAddress gateway(192, 168, 4, 0);   // (ゲートウェイ)
const IPAddress subnet(255, 255, 255, 0);  // サブネットマスク

void VSD_isrRx() {
  // 利用可能なパケット長を確認
  int packetSize = udp.parsePacket();

  if (packetSize) {
    // 送信元(remote)のIPアドレス・ポートを確認
    IPAddress r_ip = udp.remoteIP();
    int r_port = udp.remotePort();

    // パケット読出し
    char packetBuffer[255];
    udp.read(packetBuffer, packetSize);

    for (int i = 0; i < packetSize; i++) {
      char ch = packetBuffer[i];
      if (atvsdcon.read1byte(ch) == false) continue;

      // 解析を行う
      if (atvsdcon.unpackPacket() == false) continue;

      // 返信
      udp.beginPacket(r_ip, r_port);
      udp.write(atvsdcon.r_str, atvsdcon.r_ln);
      udp.endPacket();
    }
  }
}

void checkALL() {  // 受信割込み用の関数

  // ショートパケットを全サーボに投げているので、遅い。また、現座角度しか読んでいない
  // TODO:最適化(ロングパケットにする。エラーフラグなども読む。)
  for (int sid = 1; sid < atvsdcon.MAXSERVO; sid++) {
    if (atvsdcon.servo_connected[sid]) {
      float robotis_angle = dxl.getPresentPosition(sid, UNIT_DEGREE);
      int vsido_angle = (int)((robotis_angle - 180.0) * 10);
      atvsdcon.servo_present_angles[sid] = vsido_angle;
      atvsdcon.servo_present_torques[sid] =
        (int)(dxl.getPresentCurrent(sid, UNIT_MILLI_AMPERE) * 10);
    }
  }
  VSD_isrRx();
}

void setup_ethudp() {

  pinMode(CS,OUTPUT);
  SPI.end();
  delay(10);
  SPI.begin(SCK, MISO, MOSI, CS);
  delay(20);

  pinMode(RST,OUTPUT);
  digitalWrite(RST,0);
  delay(5);
  digitalWrite(RST,1);
  delay(50);

  // poEモジュール用にSPIを定義
  // Ether通信開始
  Ethernet.init(CS);
  Ethernet.begin(mac, ip);

  // UDP通信の開始
  udp.begin(localPort);
}

void setup() {

M5.begin(true,false,true);
USB_SERIAL.begin(115200);

dacWrite(25,0);

delay(10);

USB_SERIAL.begin(115200);

IMU_SERIAL.begin(115200,SERIAL_8N1, IMU_TX, IMU_RX);//Aポート
rvc.begin(&IMU_SERIAL);//IMU取得

DXL_SERIAL.begin(1000000, SERIAL_8N1, DXL_TX, DXL_RX);
dxl.begin(1000000);
dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

setup_ethudp();

  for (int sid = 0; sid < atvsdcon.MAXSERVO; sid++) {
    atvsdcon.servo_connected[sid] = dxl.ping(sid);
    dxl.torqueOff(sid);
    dxl.setOperatingMode(sid, OP_POSITION);
    dxl.torqueOn(sid);
  }

  delay(1000);
}

void loop() {
  BNO08x_RVC_Data heading;

 if (rvc.read(&heading)) {
  Serial.print(heading.yaw);Serial.print(F(","));
  Serial.print(heading.pitch);Serial.print(F(","));
  Serial.print(heading.roll);Serial.print(F(","));
  Serial.print(heading.x_accel);Serial.print(F(","));
  Serial.print(heading.y_accel);Serial.print(F(","));
  Serial.print(heading.z_accel);
  Serial.println("");
 }  

  for (int sid = 1; sid < atvsdcon.MAXSERVO; sid++) {
    if (atvsdcon.servo_connected[sid]) {
      // ショートパケットを全サーボに投げるパターン
      float robotis_angle = atvsdcon.servo_angles[sid] * 0.1 + 180.0;
      bool result = dxl.setGoalPosition(sid, robotis_angle, UNIT_DEGREE);
    }
  }
  // V-Sidoプロトコルの受信割込み
  checkALL();
}
