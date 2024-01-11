//#include "M5AtomS3.h"
#include <At_Vsido_Connect_Library.h>
#include <Dynamixel2Arduino.h>

#define USB_SERIAL Serial
#define VSD_SERIAL USB_SERIAL
#define DXL_SERIAL Serial1
const int DXL_DIR_PIN = G38;
const float DXL_PROTOCOL_VERSION = 2.0;
uint32_t BAUDRATE = 2000000;
Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
using namespace ControlTableItem;

// V-SIdo Connectライブラリ
At_Vsido_Connect_Library atvsdcon = At_Vsido_Connect_Library();

void VSD_isrRx() {

  
  // 受信確認
  while (VSD_SERIAL.available()) {
    // 1文字だけ読み込む
    unsigned char ch = VSD_SERIAL.read();

    if (atvsdcon.read1byte(ch) == false) continue;
    //解析を行う
    if (atvsdcon.unpackPacket() == false) continue;

    VSD_SERIAL.write(atvsdcon.r_str, atvsdcon.r_ln);
  }
}

void checkALL() {  //受信割込み用の関数

  //ショートパケットを全サーボに投げているので、遅い。また、現座角度しか読んでいない  
  //TODO:最適化(ロングパケットにする。エラーフラグなども読む。)
  for (int sid = 1; sid < atvsdcon.MAXSERVO; sid++) {
    if (atvsdcon.servo_connected[sid]) {
      float robotis_angle = dxl.getPresentPosition(sid,UNIT_DEGREE);
      int vsido_angle=(int) ((robotis_angle-180.0)*10);
      atvsdcon.servo_present_angles[sid]=vsido_angle;
      atvsdcon.servo_present_torques[sid]=(int)(dxl.getPresentCurrent(sid,UNIT_MILLI_AMPERE) * 10);
    }
  }

  //シリアル割込み
  while (VSD_SERIAL.available())
    VSD_isrRx();
  // UDP割り込み処理を入れるならここ
  /****/
}

void setup() {
  pinMode(DXL_DIR_PIN, OUTPUT);     // TXENピンを出力モードに設定
  digitalWrite(DXL_DIR_PIN, LOW);   // 初期状態ではLOWに設定
  USB_SERIAL.begin(2000000);
  dxl.begin(BAUDRATE);
  Serial1.begin(BAUDRATE, SERIAL_8N1, G1, G2);
  //M5.begin(false, true, false, true);  // LCD, USBSerial, I2C, LED
  //USBSerial.println("Pls Press Btn change color");
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  for (int sid = 0; sid < atvsdcon.MAXSERVO; sid++) {
    atvsdcon.servo_connected[sid] = dxl.ping(sid);
    dxl.torqueOff(sid);
    dxl.setOperatingMode(sid, OP_POSITION);
    dxl.torqueOn(sid);
  }

  atvsdcon.servo_connected[1] = true; 

  delay(1000);

}

void loop() {

  for (int sid = 1; sid < atvsdcon.MAXSERVO; sid++) {
    if (atvsdcon.servo_connected[sid]) {
      //ショートパケットを全サーボに投げているので、遅い
      //TODO:最適化(ロングパケットにする)
      float robotis_angle = atvsdcon.servo_angles[sid] * 0.1 + 180.0;
      bool result = dxl.setGoalPosition(sid, robotis_angle, UNIT_DEGREE);
    }
  }
  // V-Sidoプロトコルの受信割込み
  checkALL();

}
