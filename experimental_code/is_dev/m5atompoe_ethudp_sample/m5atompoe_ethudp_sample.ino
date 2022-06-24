#include <M5Atom.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUDP.h>

#include "At_Vsido_Connect_Library.h"

#include "Network_Setting.h"

#define BUFSIZE 1024

#define SCK 22
#define MISO 23
#define MOSI 33
#define CS 19

EthernetUDP udp;
// WiFiUDP udp;
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
// MACはATOM LITE POEサンプルと同じものを仮設定

// V-SIdo Connectライブラリ
At_Vsido_Connect_Library atvsdcon = At_Vsido_Connect_Library();

void VSD_isrRx()
{
  //利用可能なパケット長を確認
  int packetSize = udp.parsePacket();

  if (packetSize)
  {

    //送信元(remote)のIPアドレス・ポートを確認
    IPAddress r_ip = udp.remoteIP();
    int r_port = udp.remotePort();

    //パケット読出し
    char packetBuffer[255];
    udp.read(packetBuffer, packetSize);

    for (int i = 0; i < packetSize; i++)
    {
      char ch = packetBuffer[i];
      if (atvsdcon.read1byte(ch) == false)
        continue;

      //解析を行う
      if (atvsdcon.unpackPacket() == false)
        continue;

      //解析内容が正しく、受信成功した場合の処理を書く
      Serial.println("VSido Packet Received.");

      //返信
      udp.beginPacket(r_ip, r_port);
      udp.write(atvsdcon.r_str, atvsdcon.r_ln);
      udp.endPacket();
    }
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

void setup_ethudp()
{
  SPI.begin(SCK, MISO, MOSI, -1);
  Ethernet.init(CS);
  Ethernet.begin(mac, ip);

  udp.begin(localPort);
  Serial.printf("udp server start. Port: %d\n", localPort);
}

void setup()
{
  //シリアルとled利用で初期化
  //シリアルは115200bps
  M5.begin(true, false, true);

  //起動したら赤色LEDに
  M5.dis.drawpix(0, 0xff0000);
  delay(50);

  setup_ethudp();
  // setup最後に緑色LEDに
  M5.dis.drawpix(0, 0x00ff00);
}

void loop()
{

  M5.update();

  //現在角度、ステータスバイト更新処理
  updateServoMotor();

  // vsidoパケット受信処理
  VSD_isrRx();

  delay(10);
}
