#include <Arduino.h>
#include <MsTimer2.h>

#include <At_Vsido_Connect_Library.h>
#include <At_Vsido_Connect_Sender.h>

#if defined(ARDUINO_OpenCM904) // When using official ROBOTIS board with DXL circuit.
  #define DEBUG_SERIAL Serial2   //VSD用シリアル（基板上シリアルポートを利用）
  #define DXL_SERIAL1 Serial3  //DXLシリアル（拡張ボード用シリアルを利用）
  #define VSD_SERIAL Serial //デバッグ用シリアル（基板上のUSBポートを利用）
  const uint8_t DXL_DIR = 22; //DXLのディレクションピン（拡張ボード用は22,基板上は28)
#endif

#define VSD_SERIAL Serial1 //デバッグ用シリアル（基板上のUSBポートを利用）

//V-Sido Connect Senderライブラリ
At_Vsido_Connect_Sender atvsidosender=At_Vsido_Connect_Sender();

void VSD_isrRx()
{
    while(VSD_SERIAL.available()) {    // 受信確認
        unsigned char ch = VSD_SERIAL.read();            // 1文字だけ読み込む
        if(atvsidosender.read1byte(ch)==false){
            continue;
        }

        bool ret=atvsidosender.unpackPacket();
        if(ret==true){
            DEBUG_SERIAL.writelen("vsido cmd received")
        }
    }
}

void setup() {
  //シリアル関係初期化
  DEBUG_SERIAL.begin(115200);
  VSD_SERIAL.begin(1000000);


for(int id=1;id<=3;id++){
  atvsidosender.servo_connected[id]=true;
  atvsidosender.servo_torques[id]=1000;
  }

  unsigned char send_buf[VSIDO_MAXPACKETLEN]={};
  int send_len=0;
  atvsidosender.genTorquePacket(send_buf,&send_len);
  VSD_SERIAL.write(send_buf, send_len);//サーボONコマンド送信

　//timerをセット
  MsTimer2::set(2000, update_angle);
  MsTimer2::start();
}

void checkALL(){//受信割込み用の関数
  //シリアル割込み
  while ( VSD_SERIAL.available() )VSD_isrRx();
  //UDP割り込み処理を入れるならここ
  /****/
}

void update_angle()
{
    static int count=0;

    //45度ずつ動かす
    int target_anlge=450*count;

    //戻り
    if(count>4){
        target_angle=450*(8-count);
    }

    count++;
    //0degに戻ったらカウントリセット
    if(count>=8){
        count=0;
    }

  for(int id=1;id<=3;id++){
    atvsidosender.servo_angles[1]=target_anlge;
  }
}


void loop_serial_sender()
{
    unsigned char send_buf[VSIDO_MAXPACKETLEN]={};
    int send_len=0;

    //目標角度送信
    atvsidosender.genObjectPacket(send_buf,&send_len);
    VSD_SERIAL.write(send_buf, send_len);
}

void loop() {

    //V-Sidoプロトコルの送信
    loop_sender();

    //V-Sidoプロトコルの受信割込み
    checkALL();
}
