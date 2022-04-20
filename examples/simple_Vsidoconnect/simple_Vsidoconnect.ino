#include <At_Vsido_Connect_Library.h>
#include <DynamixelShield.h>

#if defined(ARDUINO_OpenCM904) // When using official ROBOTIS board with DXL circuit.
  #define DEBUG_SERIAL Serial2   //VSD用シリアル（基板上シリアルポートを利用）
  #define DXL_SERIAL1 Serial3  //DXLシリアル（拡張ボード用シリアルを利用）
  #define VSD_SERIAL Serial //デバッグ用シリアル（基板上のUSBポートを利用）
  const uint8_t DXL_DIR = 22; //DXLのディレクションピン（拡張ボード用は22,基板上は28)
#endif

//DXL通信関連
const float DXL_PROTOCOL_VERSION = 2.0;
DynamixelShield dxl(DXL_SERIAL1,DXL_DIR);//OpenCM904のDXL
using namespace ControlTableItem;

//V-SIdo Connectライブラリ
At_Vsido_Connect_Library atvsdcon=At_Vsido_Connect_Library();

void VSD_isrRx()
{
    while(VSD_SERIAL.available()) {    // 受信確認
        unsigned char ch = VSD_SERIAL.read();            // 1文字だけ読み込む
        if(atvsdcon.read1byte(ch)==false)continue;
    }
}

void setup() {
  //シリアル関係初期化
  DEBUG_SERIAL.begin(115200);
  VSD_SERIAL.begin(1000000);
  dxl.begin(1000000);
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  for(int sid=0;sid<atvsdcon.MAXSERVO;sid++){
    atvsdcon.servo_connected[sid]=dxl.ping(sid);
    dxl.torqueOff(sid);
    dxl.setOperatingMode(sid, OP_POSITION);
    dxl.torqueOn(sid);
  }
}

void checkALL(){//受信割込み用の関数
  //シリアル割込み
  while ( VSD_SERIAL.available() )VSD_isrRx();
  //UDP割り込み処理を入れるならここ
  /****/
}

void loop() {
  for(int sid=0;sid<atvsdcon.MAXSERVO;sid++){
    //受信角度を読み込み
    int int_position = atvsdcon.servo_angles[sid]+2048;
    //受信角度をDynamixelに送信
    if(atvsdcon.servo_connected[sid]){
      dxl.setGoalPosition(sid, int_position);    
    }
    //V-Sidoプロトコルの受信割込み
    checkALL();
  }
}
