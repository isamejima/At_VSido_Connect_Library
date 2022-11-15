#include <M5Atom.h>
#include <BluetoothSerial.h>
#include <At_Vsido_Connect_Library.h>

#include "AT_TCA9548A.h"
#include "AT_PbHub.h"

#define PaHub_I2C_ADDRESS 0x70
#define PbHub_I2C_ADDRESS IIC_ADDR1

unsigned char sendbuf[255] = {};
int           send_len     = 0;

unsigned long last_update_ms;
unsigned long elapsed_ms;
unsigned long loop_interval_ms = 20;

BluetoothSerial SerialBT;
String          target_name = "VSidoConnect_Atom";

AT_TCA9548A tca9548a;
AT_PbHub pbhub;
uint8_t  PBHUB_CH_REG[6] = {
  CH0_ADDR, CH1_ADDR, CH2_ADDR, CH3_ADDR, CH4_ADDR, CH5_ADDR
};

bool is_hub_connected[6] = {false, false, false, false, false, false};

#define VSD_SERIAL   SerialBT
#define DEBUG_SERIAL Serial
bool debug_flag = false;

//V-Sido Connect Library
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

void updateServoMotor() {
  // TODO　目標角度に応じてモーターを回す

  //現在ステータス更新
  for (int id = 1; id < atvsdcon.MAXSERVO; id++) {
    atvsdcon.servo_present_angles[id] =
      atvsdcon.servo_angles[id];  //現在角度を目標角度に
    atvsdcon.servo_present_torques[id] =
      atvsdcon.servo_torques[id];  //現在トルクを目標トルクに

    //サーボON状態を更新
    if (atvsdcon.servo_torques[id] == 0) {
      atvsdcon.servo_status_servoon[id] = false;  // servoeoff状態
    } else {
      atvsdcon.servo_status_servoon[id] = true;  // servoon状態
    }

    //エラー情報を更新
    atvsdcon.servo_status_error[id] = false;  // torqueoff状態
  }
}

void checkALL() {  // 受信割込み用の関数
  // シリアル割込み
  while (VSD_SERIAL.available())
    VSD_isrRx();
  // UDP割り込み処理を入れるならここ
  /****/
}

void setup_bt() {
  SerialBT.begin(target_name);

  if (debug_flag)DEBUG_SERIAL.println(String("BT Name is") + target_name);
  if (debug_flag)DEBUG_SERIAL.println("Waiting for Pairing ...");
}

void setup_PaPbHub() {

  Wire1.begin(26, 32);

  tca9548a.address(PaHub_I2C_ADDRESS);
  tca9548a.setWire(&Wire1);

  pbhub.setWire(&Wire1);

if (debug_flag)DEBUG_SERIAL.println("setup PaPbhub...");

  uint8_t returnCode = 0;

  //接続済みのポートを調べる
  for (uint8_t a_ch = 0; a_ch < 6; a_ch++) {
    returnCode = tca9548a.selectChannel(a_ch);
    if (returnCode == 0) {
      Wire1.beginTransmission(PbHub_I2C_ADDRESS);
      returnCode = Wire1.endTransmission();
      if (returnCode == 0) {
        if(debug_flag)DEBUG_SERIAL.printf("CH%d PbHub detected\n", a_ch);
        is_hub_connected[a_ch] = true;

        for (int8_t b_ch = 0; b_ch < 6; b_ch++)
        {
          uint8_t sid = a_ch * 6 + b_ch+1;
          atvsdcon.servo_connected[sid] = true;
        }
      }
    }
  }
}

void setup() {
  M5.begin(false, false, true);
  M5.dis.drawpix(0, 0xFF0000);

  delay(100);
  if (debug_flag)DEBUG_SERIAL.begin(115200);

  setup_PaPbHub();

  setup_bt();

  M5.dis.drawpix(0, 0x0000FF);


if(debug_flag)DEBUG_SERIAL.println("setup finished");
  last_update_ms = millis();

}

void loop() {
  uint8_t returnCode = 0;

  for (uint8_t a_ch = 0; a_ch < 6; a_ch++) {
    if (is_hub_connected[a_ch] == false) {
      continue;
    }

    returnCode = tca9548a.selectChannel(a_ch);
    if (returnCode == 0) {

      for (uint8_t b_ch = 0; b_ch < 6; b_ch++) {
        uint8_t sid = a_ch * 6 + b_ch + 1;
        //受信角度を読み込み
        int int_position = atvsdcon.servo_angles[sid];

        //受信角度をサーボに送信
        if (atvsdcon.servo_connected[sid]) {
          uint16_t pulse = map(int_position, -1800, 1800, 500, 2500);
          pbhub.hub_wire_setServoPulse_B(PBHUB_CH_REG[b_ch], pulse);
          if(debug_flag)DEBUG_SERIAL.printf("sid=%d angle=%d pulse=%d\r\n",sid,int_position,pulse);
          
        }
      }
    }
  }
  delay(40);
  checkALL();
}
