#include <M5Atom.h>
#include <At_Vsido_Connect_Sender.h>
#include <BluetoothSerial.h>

#include "AT_TCA9548A.h"
#include "AT_PbHub.h"

#define PaHub_I2C_ADDRESS 0x70
#define PbHub_I2C_ADDRESS IIC_ADDR1

unsigned char sendbuf[255] = {};
int           send_len     = 0;

// V-Sido Connect Senderライブラリ
At_Vsido_Connect_Sender atvsidosender = At_Vsido_Connect_Sender();

unsigned long last_update_ms;
unsigned long elapsed_ms;
unsigned long loop_interval_ms = 20;

BluetoothSerial SerialBT;
String          target_name = "VSidoConnect_Atom";

AT_TCA9548A tca9548a;
AT_PbHub pbhub;
uint8_t  PBHUB_CH_REG[6] = {
    CH0_ADDR, CH1_ADDR, CH2_ADDR, CH3_ADDR, CH4_ADDR, CH5_ADDR};

bool is_volume_connected[6] = {false, false, false, false, false, false};

#define VSD_SERIAL   SerialBT
#define DEBUG_SERIAL Serial
bool debug_flag=true;

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

void checkALL() {  // 受信割込み用の関数
  // シリアル割込み
  while (VSD_SERIAL.available())
    VSD_isrRx();
  // UDP割り込み処理を入れるならここ
  /****/
}

void setup_bt() {
  if (debug_flag)DEBUG_SERIAL.println(String("Connecting to ") + target_name + " ...");

  // bt初期化
  SerialBT.begin("VSidoConnect_AtomBTSender", true);

  bool connected = SerialBT.connect(target_name);

  if (connected) {
    if (debug_flag)DEBUG_SERIAL.println("Connected Succesfully!");
  } else {
    while (!SerialBT.connected(10000)) {
      if (debug_flag)DEBUG_SERIAL.println("Failed to connect. retry ...");
    }
  }
}

void setup_PaPbHub() {

  Wire1.begin(26, 32);

  tca9548a.address(PaHub_I2C_ADDRESS);
  tca9548a.setWire(&Wire1);

  pbhub.setWire(&Wire1);

  uint8_t returnCode = 0;

  //接続済みのポートを調べる
  for (uint8_t a_ch = 0; a_ch < 6; a_ch++) {
    returnCode = tca9548a.selectChannel(a_ch);
    if (returnCode == 0) {
      Wire1.beginTransmission(PbHub_I2C_ADDRESS);
      returnCode = Wire1.endTransmission();
      if (returnCode == 0) {
        Serial.printf("CH%d PbHub detected\n", a_ch);
        is_volume_connected[a_ch] = true;

        for (int8_t b_ch = 0; b_ch < 6; b_ch++)
        {
          uint8_t target_id = a_ch * 6 + b_ch;
          atvsidosender.servo_connected[target_id] = true;
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

  last_update_ms = millis();

}

void loop() {
  uint8_t returnCode = 0;

  for (uint8_t a_ch = 0; a_ch < 8; a_ch++) {
    if (is_volume_connected[a_ch] == false) {
      continue;
    }

    returnCode = tca9548a.selectChannel(a_ch);
    if (returnCode == 0) {

      for (uint8_t b_ch = 0; b_ch < 6; b_ch++) {
        uint16_t analog_value = pbhub.hub_a_read_value(PBHUB_CH_REG[b_ch]);
        int target_angle = map(analog_value, 0, 4095, -1800, 1800);
        uint8_t target_id = a_ch * 6 + b_ch + 1;

        if (atvsidosender.servo_connected[target_id])
        {
          atvsidosender.servo_angles[target_id] = target_angle;
        }
      }
    }
  }
  
  elapsed_ms = millis() - last_update_ms;
  if (elapsed_ms >= loop_interval_ms) {
    atvsidosender.genObjectPacket(2, sendbuf, &send_len);
    VSD_SERIAL.write(sendbuf, send_len);
    last_update_ms = millis();
  }

  checkALL();
}
