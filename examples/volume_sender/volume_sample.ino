#include <MCP_ADC.h>
#include <At_Vsido_Connect_Sender.h>

#define SCK 22
#define MISO 23
#define MOSI 33
#define CS 19

//V-Sido Connect Senderライブラリ
At_Vsido_Connect_Sender atvsidosender=At_Vsido_Connect_Sender();

//ad converter library
MCP3208 mcp(MISO,MOSI,SCK);

#define VSD_SERIAL Serial;
#define DEBUG_SERIAL Serial;

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

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);

  mcp.selectVSPI();
  mcp.begin(CS);
  
  Serial.println();
  Serial.println("ADC\tCHAN\tMAXVALUE");
  Serial.print("mcp\t");
  Serial.print(mcp.channels());
  Serial.print("\t");
  Serial.println(mcp.maxValue());
}


void loop()
{
  Serial.print("mcp:\t");
  for (int channel = 0 ; channel < mcp.channels(); channel++)
  {
    uint16_t val = mcp.analogRead(channel);
    Serial.print(val);
    Serial.print("\t");
  }
  Serial.println();

  delay(1000);
}