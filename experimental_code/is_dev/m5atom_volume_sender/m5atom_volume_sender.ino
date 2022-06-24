#include <MCP_ADC.h>
#include <At_Vsido_Connect_Sender.h>
#include <BluetoothSerial.h>

#define SCK 22
#define MISO 23
#define MOSI 33
#define CS 19

unsigned char sendbuf[255] = {};
int send_len = 0;

BluetoothSerial SerialBT;
String target_name = "VSidoConnect_StickC";

// V-Sido Connect Senderライブラリ
At_Vsido_Connect_Sender atvsidosender = At_Vsido_Connect_Sender();

// ad converter library
MCP3208 mcp(MISO, MOSI, SCK);

#define VSD_SERIAL SerialBT
#define DEBUG_SERIAL Serial

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
    }
  }
}

void setup()
{
  delay(100);

  DEBUG_SERIAL.begin(115200);

  mcp.begin(CS);

  DEBUG_SERIAL.println();
  DEBUG_SERIAL.println("ADC\tCHAN\tMAXVALUE");
  DEBUG_SERIAL.print("mcp\t");
  DEBUG_SERIAL.print(mcp.channels());
  DEBUG_SERIAL.print("\t");
  DEBUG_SERIAL.println(mcp.maxValue());

  atvsidosender.servo_connected[1] = true;
  atvsidosender.servo_connected[2] = true;
  atvsidosender.servo_connected[3] = true;
  atvsidosender.servo_connected[4] = true;

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

void loop()
{
  //  DEBUG_SERIAL.print("mcp:\t");
  for (int channel = 0; channel < mcp.channels(); channel++)
  {
    uint16_t val = mcp.analogRead(channel);
    if (atvsidosender.servo_connected[channel + 1])
    {
      int max_value = mcp.maxValue();
      int center_value = max_value / 2;
      atvsidosender.servo_angles[channel + 1] = (center_value - val) * 1350 / center_value;
      DEBUG_SERIAL.print(atvsidosender.servo_angles[channel + 1]);
      DEBUG_SERIAL.print("\t");
    }
  }
  DEBUG_SERIAL.println();
  atvsidosender.genObjectPacket(2, sendbuf, &send_len);
  VSD_SERIAL.write(sendbuf, send_len);

  for (int i = 0; i < send_len; i++)
  {
    DEBUG_SERIAL.printf("%x ", sendbuf[i]);
  }
  DEBUG_SERIAL.println();
  delay(10);
}