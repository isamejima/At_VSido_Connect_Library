#include <At_Vsido_Connect_Sender.h>
#include <BluetoothSerial.h>
#include <M5Atom.h>

#include "AT_PbHub.h"

unsigned char sendbuf[255] = {};
int           send_len     = 0;

// V-Sido Connect Senderライブラリ
At_Vsido_Connect_Sender atvsidosender = At_Vsido_Connect_Sender();

unsigned long last_update_ms;
unsigned long elapsed_ms;
unsigned long loop_interval_ms = 20;

BluetoothSerial SerialBT;
String          target_name = "VSidoConnect_Atom";

AT_PbHub pbhub;
uint8_t  PBHUB_CH_REG[6] = {
    CH0_ADDR, CH1_ADDR, CH2_ADDR, CH3_ADDR, CH4_ADDR, CH5_ADDR};

#define VSD_SERIAL   SerialBT
#define DEBUG_SERIAL Serial
bool debug_flag = true;

void VSD_isrRx() {
	while (VSD_SERIAL.available()) {  // 受信確認
		unsigned char ch = VSD_SERIAL.read();  // 1文字だけ読み込む
		if (atvsidosender.read1byte(ch) == false) {
			continue;
		}
		bool ret = atvsidosender.unpackPacket();
		if (ret == true) {}
	}
}

void checkALL() {  // 受信割込み用の関数
	// シリアル割込み
	while (VSD_SERIAL.available())
		VSD_isrRx();
	// UDP割り込み処理を入れるならここ
	/****/
}

void setup_PbHub() {
	Wire1.begin(26, 32);
	pbhub.setWire(&Wire1);
	if (debug_flag) DEBUG_SERIAL.println("pbhub is detected");
}

void setup_bt() {
	if (debug_flag)
		DEBUG_SERIAL.println(String("Connecting to ") + target_name + " ...");

	// bt初期化
	SerialBT.begin("VSidoConnect_AtomBTSender", true);

	bool connected = SerialBT.connect(target_name);

	if (connected) {
		if (debug_flag) DEBUG_SERIAL.println("Connected Succesfully!");
	} else {
		while (!SerialBT.connected(10000)) {
			if (debug_flag) DEBUG_SERIAL.println("Failed to connect. retry ...");
		}
	}
}

void setup_vsidosender() {
	// 6サーボ分、送信125 byte,受信124byte
	for (int id = 1; id < 7; id++) {
		atvsidosender.servo_connected[id] = true;
	}
}

void setup() {
	M5.begin(false, false, true);
	M5.dis.drawpix(0, 0xFF0000);

	delay(100);
	if (debug_flag) DEBUG_SERIAL.begin(115200);

	setup_vsidosender();

	setup_PbHub();

	setup_bt();

	M5.dis.drawpix(0, 0x0000FF);

	last_update_ms = millis();
}

void loop() {
	elapsed_ms = millis() - last_update_ms;

	if (elapsed_ms >= loop_interval_ms) {
		for (int channel = 0; channel < 6; channel++) {
			uint16_t analog_value = pbhub.hub_a_read_value(PBHUB_CH_REG[channel]);
			if (atvsidosender.servo_connected[channel + 1]) {
				int target_angle = map(analog_value, 0, 4095, -1350, 1350);
				atvsidosender.servo_angles[channel + 1] = target_angle;
			}
		}
		atvsidosender.genObjectPacket(2, sendbuf, &send_len);
		VSD_SERIAL.write(sendbuf, send_len);


		last_update_ms = millis();
	}

	checkALL();
}