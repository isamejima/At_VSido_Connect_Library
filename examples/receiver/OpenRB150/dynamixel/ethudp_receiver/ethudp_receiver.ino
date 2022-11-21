#include <At_Vsido_Connect_Library.h>
#include <Dynamixel2Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

bool debug_flag=true;

//Ethernet関連
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 4, 40);
const IPAddress gateway(192, 168, 4, 0);  // (ゲートウェイ)
const IPAddress subnet(255, 255, 255, 0);  // サブネットマスク
unsigned int localPort = 8888;  // local port to listen on
EthernetUDP udp;

// DXL通信関連
#if defined(ARDUINO_OpenRB)  // When using OpenRB-150
//OpenRB does not require the DIR control pin.
#define DXL_SERIAL Serial1
#define DEBUG_SERIAL Serial
const int DXL_DIR_PIN = -1;
#endif

#define USB_SERIAL Serial
#define VSD_SERIAL Serial2 //使わないので、空きポートに割り当て

const float DXL_PROTOCOL_VERSION = 2.0;
uint32_t BAUDRATE = 57600;

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
//This namespace is required to use Control table item names
using namespace ControlTableItem;

// V-SIdo Connectライブラリ
At_Vsido_Connect_Library atvsdcon = At_Vsido_Connect_Library();

void VSD_isrUDP() {
	//利用可能なパケット長を確認
	int packetSize = udp.parsePacket();

	if (packetSize) {
		//送信元(remote)のIPアドレス・ポートを確認
		IPAddress r_ip   = udp.remoteIP();
		int       r_port = udp.remotePort();

		//パケット読出し
		char packetBuffer[255];
		udp.read(packetBuffer, packetSize);

		for (int i = 0; i < packetSize; i++) {
			char ch = packetBuffer[i];
			if (atvsdcon.read1byte(ch) == false) continue;
			//解析を行う
			if (atvsdcon.unpackPacket() == false) continue;
			//返信
			udp.beginPacket(r_ip, r_port);
			udp.write(atvsdcon.r_str, atvsdcon.r_ln);
			udp.endPacket();
		}
	}
}

void VSD_isrRx() {
  // 受信確認
  while (VSD_SERIAL.available()) {
    // 1文字だけ読み込む
    unsigned char ch = VSD_SERIAL.read();

    if (atvsdcon.read1byte(ch) == false) continue;
    //解析を行う
    if (atvsdcon.unpackPacket() == false) continue;

    VSD_SERIAL.write(atvsdcon.r_str, atvsdcon.r_ln);
  }
}

void setup_ethudp()
{
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  Ethernet.init(5);   // MKR ETH Shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit FeatherWing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit FeatherWing Ethernet

  // start the Ethernet
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    if(debug_flag)DEBUG_SERIAL.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    if(debug_flag)DEBUG_SERIAL.println("Ethernet cable is not connected.");
  }

  // start UDP
  udp.begin(localPort);
}

void setup() {
  //シリアル関係初期化
  USB_SERIAL.begin(115200);

  setup_ethudp();

  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(BAUDRATE);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  //OpenRB User LED(LED2)の設定
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);//点灯

  for (int sid = 0; sid < atvsdcon.MAXSERVO; sid++) {
    atvsdcon.servo_connected[sid] = dxl.ping(sid);
    dxl.torqueOff(sid);
    dxl.setOperatingMode(sid, OP_POSITION);
    dxl.torqueOn(sid);
  }
}

void updateServoStatus() {
	// ショートパケットを全サーボに投げている。また、現座角度しか読んでいない。
	// TODO:最適化(ロングパケットにする。エラーフラグなども読む。)
	for (int sid = 1; sid < atvsdcon.MAXSERVO; sid++) {
		if (atvsdcon.servo_connected[sid]) {
			float robotis_angle = dxl.getPresentPosition(sid, UNIT_DEGREE);
			int   vsido_angle   = (int)((robotis_angle - 180.0) * 10);
			atvsdcon.servo_present_angles[sid] = vsido_angle;
		}
	}
}

void checkALL() {  //受信割込み用の関数

	// dynamixelのステータス読出し
	updateServoStatus();

	//シリアル割込み
  //VSD_isrRx();

  // UDP処理
  VSD_isrUDP();
}

void loop() {

  for (int sid = 1; sid < atvsdcon.MAXSERVO; sid++) {
    if (atvsdcon.servo_connected[sid]) {
      //ショートパケットを全サーボに投げているので、遅い
      //TODO:最適化(ロングパケットにする)
      float robotis_angle = atvsdcon.servo_angles[sid] * -0.1 + 180.0;
      bool result = dxl.setGoalPosition(sid, robotis_angle, UNIT_DEGREE);
    }
  }
    // V-Sidoプロトコルの受信割込み
    checkALL();
}
