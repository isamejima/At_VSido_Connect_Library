#include <M5Atom.h>
#include <BluetoothSerial.h>

#define VSIDO_MAXSERVO 32
#include <At_Vsido_Connect_Library.h>

BluetoothSerial SerialBT;
String target_name = "VSidoConnect_Atom";

// V-SIdo Connectライブラリ
At_Vsido_Connect_Library atvsdcon = At_Vsido_Connect_Library();

#define VSD_SERIAL SerialBT
#define DEBUG_SERIAL Serial

void VSD_isrRx()
{
  // 受信確認
  while (VSD_SERIAL.available())
  {

    // 1文字だけ読み込む
    unsigned char ch = VSD_SERIAL.read();

    if (atvsdcon.read1byte(ch) == false)
      continue;
    //解析を行う
    if (atvsdcon.unpackPacket() == false)
      continue;

    DEBUG_SERIAL.println("vsido packet received");
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
  DEBUG_SERIAL.println(target_name + " started. You can pair it with bluetooth");
}

void setup()
{
  //シリアルとled利用で初期化
  //シリアルは115200bps
  M5.begin(true, false, true);

  //起動したら赤色LEDに
  M5.dis.drawpix(0, 0xff0000);
  delay(50);

  setup_bt();
  delay(50);
  // setup最後に緑色LEDに
  M5.dis.drawpix(0, 0x00ff00);

  //サーボidを有効化
  atvsdcon.servo_connected[1] = true;
  atvsdcon.servo_connected[2] = true;
  atvsdcon.servo_connected[3] = true;
  atvsdcon.servo_connected[4] = true;
}

void checkALL()
{ //受信割込み用の関数
  //シリアル割込み
  while (VSD_SERIAL.available())
    VSD_isrRx();
}

void loop()
{

  M5.update();

  //現在角度、ステータスバイト更新処理
  updateServoMotor();

  // vsidoパケット受信処理
  checkALL();

  delay(10);
}
