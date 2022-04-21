#include <At_Vsido_Connect_Library.h>
#include <M5Atom.h>
#include "AtomMotion.h"

#define VSD_SERIAL Serial 

//V-SIdo Connectライブラリ
At_Vsido_Connect_Library atvsdcon=At_Vsido_Connect_Library();
AtomMotion Atom;

void VSD_isrRx()
{
    while(VSD_SERIAL.available()) {    // 受信確認
        unsigned char ch = VSD_SERIAL.read();            // 1文字だけ読み込む
        if(atvsdcon.read1byte(ch)==false)continue;
    }
}

void setup() {
  M5.begin(true, false, true);
  Atom.Init();     //sda  25     scl  21 
  
  //シリアル関係初期化
  VSD_SERIAL.begin(115200);
  
  atvsdcon.servo_connected[1]=true;

}

void checkALL(){//受信割込み用の関数
  //シリアル割込み
  while ( VSD_SERIAL.available() )VSD_isrRx();
  //UDP割り込み処理を入れるならここ
  /****/
}

void loop() {
	M5.update();
	
	
  for(int sid=0;sid<atvsdcon.MAXSERVO;sid++){
    //受信角度を読み込み
    int int_position = atvsdcon.servo_angles[sid];
	
    //受信角度をサーボに送信
    if(atvsdcon.servo_connected[sid]){
		int position2=(int_position+1800)*20/36+500;

		Atom.SetServoPulse(1, position2);

	}
    //V-Sidoプロトコルの受信割込み
    checkALL();
  }

}
