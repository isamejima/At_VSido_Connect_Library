#include <M5Atom.h>
#include <BluetoothSerial.h>
#include <At_Vsido_Connect_Library.h>

#include <Wire.h>
#include "PCA9685.h"            //PCA9685用ヘッダーファイル（秋月電子通商作成） I2Cのピンに関してコードを変更

#define VSD_SERIAL SerialBT

BluetoothSerial SerialBT;
String target_name = "VSidoConnect_Atom";

 
PCA9685 pwm = PCA9685(0x40);    //PCA9685のアドレス指定（アドレスジャンパA0をClose）
PCA9685 pwm2 = PCA9685(0x41);    //PCA9685のアドレス指定（アドレスジャンパすべてOpen）
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

void setup_bt()
{
  SerialBT.begin(target_name);
}

void setup() {
  
  //引数はマイコンで変化　Atom liteの場合はUART,I2C,LEDの初期化の有無
  M5.begin(true, false, true);
  
  //M5.beginのあとは他の処理の前に少し待機
  delay(100);	
  
  pwm.begin();                   //初期設定 (アドレス0x40用)
  pwm.setPWMFreq(50);            //PWM周期を50Hzに設定 (アドレス0x40用)
  delay(50);
  pwm2.begin();                   //初期設定 (アドレス0x41用)
  pwm2.setPWMFreq(50);            //PWM周期を60Hzに設定 (アドレス0x41用)   
  delay(50);	
  
  //起動したら赤色LEDに
  M5.dis.drawpix(0, 0xff0000);
  delay(50);

  setup_bt();
  delay(50);
  
  //サーボidを32個分有効化
  for(int i=0;i<=32;i++){
  
	atvsdcon.servo_connected[i]=true;	
  }
  
  // setup最後に緑色LEDに
  M5.dis.drawpix(0, 0x00ff00);
  
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
			int_position = map(int_position, -1800, 1800, 102, 512);
			
			//サーボ1-16
			if(sid>=1&&sid<=16){
				pwm.setPWM(sid-1, 0, int_position);
			}
			//サーボ17-32
			else if(sid>=17&&sid<=32){
				pwm2.setPWM(sid-17, 0, int_position);	
			}

		}
		//V-Sidoプロトコルの受信割込み
		checkALL();
	}
}
