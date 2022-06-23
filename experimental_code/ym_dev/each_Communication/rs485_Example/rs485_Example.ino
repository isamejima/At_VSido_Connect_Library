#include <M5Atom.h>
#include <At_Vsido_Connect_Library.h>

#include "AtomMotion.h"

#define VSD_SERIAL Serial2

#define RX_PIN      32
#define TX_PIN      26
 
AtomMotion Atom;
At_Vsido_Connect_Library atvsdcon=At_Vsido_Connect_Library();

void VSD_isrRx()
{
	// 受信確認
    while(VSD_SERIAL.available()) {    
	
		// 1文字だけ読み込む
        unsigned char ch = VSD_SERIAL.read();            
		
        if(atvsdcon.read1byte(ch)==false)
			continue;
		//解析を行う
		if ( atvsdcon.unpackPacket()== false)
			continue;
		
		VSD_SERIAL.write(atvsdcon.r_str, atvsdcon.r_ln);
    }
}

void updateServoMotor()
{
  // TODO　目標角度に応じてモーターを回す

  //現在ステータス更新
  for (int id = 1; id < atvsdcon.MAXSERVO; id++)
  {
    atvsdcon.servo_present_angles[id] = atvsdcon.servo_angles[id];   //現在角度を目標角度に
    atvsdcon.servo_present_torques[id] = atvsdcon.servo_torques[id]; //現在トルクを目標トルクに

    //サーボON状態を更新
    if (atvsdcon.servo_torques[id] == 0)
    {
      atvsdcon.servo_status_servoon[id] = false; // servoeoff状態
    }
    else
    {
      atvsdcon.servo_status_servoon[id] = true; // servoon状態
    }

    //エラー情報を更新
    atvsdcon.servo_status_error[id] = false; // torqueoff状態
  }
}

void setup() {
  
  //引数はマイコンで変化　Atom liteの場合はUART,I2C,LEDの初期化の有無
  M5.begin(true, false, true);
  
  //Atom Motion初期化
  Atom.Init();      

  //M5.beginのあとは他の処理の前に少し待機
  delay(100);	
  
  //電源確認にLED点灯
  M5.dis.drawpix(0, 0xffffff);	

  //シリアル関係初期化
  VSD_SERIAL.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  
  //サーボid1を有効化
  atvsdcon.servo_connected[1]=true;	
  atvsdcon.servo_connected[2]=true;	
  atvsdcon.servo_connected[3]=true;	
  atvsdcon.servo_connected[4]=true;	

}


void checkALL(){//受信割込み用の関数

	  
	//現在角度、ステータスバイト更新処理
	updateServoMotor();

	//シリアル割込み
	while ( VSD_SERIAL.available() )VSD_isrRx();

}

void loop() {

  M5.update();
  
  
    for(int sid=1;sid<atvsdcon.MAXSERVO;sid++){
		//受信角度を読み込み
		int int_position = atvsdcon.servo_angles[sid];
		
		//受信角度をサーボに送信
		if(atvsdcon.servo_connected[sid]){
			//角度情報をサーボパルス幅へ変換
			int tar_position=(int_position+1800)*20/36+500;

			//サーボへ書き込み
			Atom.SetServoPulse(sid, tar_position);	

		}
		//V-Sidoプロトコルの受信割込み
		checkALL();
	}
}
