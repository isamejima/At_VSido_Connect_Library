#include <M5Atom.h>
#include <MCP_ADC.h>
#include <At_Vsido_Connect_Sender.h>
#include <BluetoothSerial.h>

#define SCK 22
#define MISO 23
#define MOSI 33
#define CS 19

#define RX_PIN 32
#define TX_PIN 26

unsigned char sendbuf[255] = {};
int send_len = 0;

BluetoothSerial SerialBT;
String target_name = "VSidoConnect_StickC";

// V-Sido Connect Senderライブラリ
At_Vsido_Connect_Sender atvsidosender = At_Vsido_Connect_Sender();

// ad converter library
MCP3208 mcp(MISO, MOSI, SCK);

#define VSD_SERIAL SerialBT
//#define VSD_SERIAL Serial2
#define DEBUG_SERIAL Serial

unsigned long debug_loop_time_us;
unsigned long debug_last_time_us;
int debug_countup = 0;
int debug_send_count = 0;
int debug_receive_count = 0;

unsigned long last_update_ms;
unsigned long elapsed_ms;
unsigned long loop_interval_ms=4;

void VSD_isrRx()
{
  while (VSD_SERIAL.available())
  {                                       // 受信確認
    unsigned char ch = VSD_SERIAL.read(); // 1文字だけ読み込む
    if (atvsidosender.read1byte(ch) == false)
    {
      continue;
    }

    bool ret = atvsidosender.unpackPacket();
    if (ret == true)
    {
      debug_receive_count++;
    }
  }
}

void checkALL()
{ //受信割込み用の関数
  //シリアル割込み
  while (VSD_SERIAL.available())
    VSD_isrRx();
  // UDP割り込み処理を入れるならここ
  /****/
}

void setup_mcp()
{
  mcp.begin(CS);

  /*
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("ADC\tCHAN\tMAXVALUE");
    DEBUG_SERIAL.print("mcp\t");
    DEBUG_SERIAL.print(mcp.channels());
    DEBUG_SERIAL.print("\t");
    DEBUG_SERIAL.println(mcp.maxValue());
    */
}

void setup_bt()
{
  DEBUG_SERIAL.println(String("Connecting to ") + target_name + " ...");
  // bt初期化
  SerialBT.begin("VSidoConnect_AtomVolume", true);

  bool connected = SerialBT.connect(target_name);
  // bool connected = SerialBT.connect(address);

  if (connected)
  {
    DEBUG_SERIAL.println("Connected Succesfully!");
  }
  else
  {
    while (!SerialBT.connected(10000))
    {
      DEBUG_SERIAL.println("Failed to connect. retry ...");
    }
  }
}

void setup_vsidosender()
{
  /*
  atvsidosender.servo_connected[1] = true;
  atvsidosender.servo_connected[2] = true;
  atvsidosender.servo_connected[3] = true;
  atvsidosender.servo_connected[4] = true;
  */
 //30サーボ分、送信125 byte,受信124byte
  for(int id=1;id<31;id++){
    atvsidosender.servo_connected[id] = true;
  }
}

void setup()
{
  M5.begin(false, false, true);
  M5.dis.drawpix(0, 0xFF0000);

  delay(100);
  DEBUG_SERIAL.begin(115200);
  setup_mcp();

  setup_vsidosender();

  setup_bt();
  // Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  M5.dis.drawpix(0, 0x0000FF);

  debug_last_time = micros();
  debug_countup = 0;
}

void loop()
{
  elapsed_ms = mills() - last_update_ms;

  if (elapsed_ms > loop_interval_ms)
  {   
    debug_countup += 2;
    if (debug_countup >= 200)
    {
      debug_countup = 0;
    }
    //  dacWrite(26,countup);//dac出力更新

    for (int channel = 0; channel < mcp.channels(); channel++)
    {
      uint16_t val = mcp.analogRead(channel);
      if (atvsidosender.servo_connected[channel + 1])
      {
        int max_value = mcp.maxValue();
        int center_value = max_value / 2;
        atvsidosender.servo_angles[channel + 1] = (center_value - val) * 1350 / center_value;
      }
    }
    atvsidosender.genObjectPacket(2, sendbuf, &send_len);
    VSD_SERIAL.write(sendbuf, send_len);
    send_count++;

    if (countup == 0)
    {
      debug_loop_time_us = (micros() - debug_last_time_us) / 100;

      DEBUG_SERIAL.print("send_count:");
      DEBUG_SERIAL.print(debug_send_count);
      DEBUG_SERIAL.print(",");
      DEBUG_SERIAL.print("receive_count:");
      DEBUG_SERIAL.print(debug_receive_count);
      DEBUG_SERIAL.print(",");
      DEBUG_SERIAL.print("loop_time:");
      DEBUG_SERIAL.print(debug_loop_time_us);
      DEBUG_SERIAL.println("");
      debug_last_time_us = micros();
      debug_send_count = 0;
      debug_receive_count = 0;
    }
    int offset=mills() - last_update_ms;
    last_update_ms = mills();
  }

  checkALL();
}