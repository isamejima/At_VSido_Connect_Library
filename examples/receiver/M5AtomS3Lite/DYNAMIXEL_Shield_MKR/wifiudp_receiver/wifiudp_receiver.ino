#include <At_Vsido_Connect_Library.h>
#include <Dynamixel2Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "M5AtomS3.h"


#define AP  2
#define STA 3

#define COMMUNICATION_MODE AP  // ETH,AP,STA

#if COMMUNICATION_MODE == AP
const char ssid[]    = "ATOMWiFi";  // SSID
const char pass[]    = "Atompass";  // password
const int  localPort = 8888;  // ポート番号

const IPAddress ip(192, 168, 4, 40);  // IPアドレス
const IPAddress gateway(192, 168, 4, 0);  // (ゲートウェイ)
const IPAddress subnet(255, 255, 255, 0);  // サブネットマスク

#elif COMMUNICATION_MODE == STA
const char ssid[]    = "xxxxxxxx";  // SSID
const char pass[]    = "xxxxxxxx";  // password
const int  localPort = 8888;  // ポート番号

const IPAddress ip(192, 168, 11, 40);  // IPアドレス
const IPAddress gateway(192, 168, 11, 0);  // (ゲートウェイ)
const IPAddress subnet(255, 255, 255, 0);  // サブネットマスク

#endif

#define DXL_SERIAL Serial1
const int   DXL_DIR_PIN          = G38;
const float DXL_PROTOCOL_VERSION = 2.0;
uint32_t    BAUDRATE             = 2000000;

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
using namespace ControlTableItem;

// V-SIdo Connectライブラリ
At_Vsido_Connect_Library atvsdcon = At_Vsido_Connect_Library();

WiFiUDP udp;

void VSD_isrRx() {
	// 利用可能なパケット長を確認
	int packetSize = udp.parsePacket();

	if (packetSize) {
		// 送信元(remote)のIPアドレス・ポートを確認
		IPAddress r_ip   = udp.remoteIP();
		int       r_port = udp.remotePort();

		// パケット読出し
		char packetBuffer[255];
		udp.read(packetBuffer, packetSize);

		for (int i = 0; i < packetSize; i++) {
			char ch = packetBuffer[i];
			if (atvsdcon.read1byte(ch) == false) continue;


			// 解析を行う
			if (atvsdcon.unpackPacket() == false) continue;


			// 返信
			udp.beginPacket(r_ip, r_port);
			udp.write(atvsdcon.r_str, atvsdcon.r_ln);
			udp.endPacket();
		}
	}
}

void checkALL() {  // 受信割込み用の関数

	// ショートパケットを全サーボに投げているので、遅い。また、現座角度しか読んでいない
	// TODO:最適化(ロングパケットにする。エラーフラグなども読む。)
	for (int sid = 1; sid < atvsdcon.MAXSERVO; sid++) {
		if (atvsdcon.servo_connected[sid]) {
			float robotis_angle = dxl.getPresentPosition(sid, UNIT_DEGREE);
			int   vsido_angle   = (int)((robotis_angle - 180.0) * 10);
			atvsdcon.servo_present_angles[sid] = vsido_angle;
			atvsdcon.servo_present_torques[sid] =
			    (int)(dxl.getPresentCurrent(sid, UNIT_MILLI_AMPERE) * 10);
		}
	}

	VSD_isrRx();
}

void setup_wifiudp(void) {
	switch (COMMUNICATION_MODE) {
		case AP:
			WiFi.softAP(ssid, pass);
			delay(100);
			WiFi.softAPConfig(ip, gateway, subnet);

			break;
		case STA:

			WiFi.begin(ssid, pass);

			while (WiFi.status() != WL_CONNECTED) {
				delay(500);
			}

			WiFi.config(ip, gateway, subnet);

			break;
	}
	// UDP通信の開始
	udp.begin(localPort);
}

void setup() {
	auto cfg = M5.config();
	AtomS3.begin(cfg);

	pinMode(DXL_DIR_PIN, OUTPUT);  // TXENピンを出力モードに設定
	digitalWrite(DXL_DIR_PIN, LOW);  // 初期状態ではLOWに設定

	dxl.begin(BAUDRATE);
	Serial1.begin(BAUDRATE, SERIAL_8N1, G1, G2);

	dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

	setup_wifiudp();

	for (int sid = 0; sid < atvsdcon.MAXSERVO; sid++) {
		atvsdcon.servo_connected[sid] = dxl.ping(sid);
		dxl.torqueOff(sid);
		dxl.setOperatingMode(sid, OP_POSITION);
		dxl.torqueOn(sid);
	}

	atvsdcon.servo_connected[1] = true;

	delay(1000);
}

void loop() {
	for (int sid = 1; sid < atvsdcon.MAXSERVO; sid++) {
		if (atvsdcon.servo_connected[sid]) {
			// ショートパケットを全サーボに投げているので、遅い
			// TODO:最適化(ロングパケットにする)
			float robotis_angle = atvsdcon.servo_angles[sid] * 0.1 + 180.0;
			bool  result = dxl.setGoalPosition(sid, robotis_angle, UNIT_DEGREE);
		}
	}
	// V-Sidoプロトコルの受信割込み
	checkALL();
}
