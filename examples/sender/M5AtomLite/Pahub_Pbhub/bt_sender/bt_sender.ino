#include <At_Vsido_Connect_Sender.h>
#include <BluetoothSerial.h>
#include <M5Atom.h>

#include "AT_TCA9548A.h"
#include "AT_PbHub.h"

unsigned char sendbuf[255] = {};
int           send_len     = 0;

// V-Sido Connect Senderライブラリ
At_Vsido_Connect_Sender atvsidosender = At_Vsido_Connect_Sender();

unsigned long last_update_ms;
unsigned long elapsed_ms;
unsigned long loop_interval_ms = 1000;

BluetoothSerial SerialBT;
String          target_name = "VSidoConnect_Atom";


AT_PbHub pbhub;
uint8_t  PBHUB_CH_REG[6] = {
    CH0_ADDR, CH1_ADDR, CH2_ADDR, CH3_ADDR, CH4_ADDR, CH5_ADDR};

AT_TCA9548A tca9548a;
bool is_volume_connected[6] = {false, false, false, false, false, false};

#define VSD_SERIAL   SerialBT
#define DEBUG_SERIAL Serial
#define I2C_SCL 26
#define I2C_SDA 32
#define PaHub_I2C_ADDRESS 0x70
#define PbHub_I2C_ADDRESS IIC_ADDR1


bool debug_flag = false;

void VSD_isrRx() {
	while (VSD_SERIAL.available()) {  // 受信確認
		unsigned char ch = VSD_SERIAL.read();  // 1文字だけ読み込む
		if (atvsidosender.read1byte(ch) == false) {
			continue;
		}
		debug_receive_count++;
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

void setup_PaHubPbhub() {
	tca9548a.setWire(&Wire1);
	pbhub.setWire(&Wire1);

	uint8_t returnCode = 0;

	// 接続済みのポートを調べる
	for (uint8_t channel = 0; channel < 6; channel++) {
		returnCode = tca9548a.selectChannel(channel);
		if (returnCode == 0) {
			Wire1.beginTransmission(PbHub_I2C_ADDRESS);
			returnCode = Wire1.endTransmission();
			if (returnCode == 0) {
				if (debug_flag) DEBUG_SERIAL.printf("PaHub CH%d I2C device detected\n", channel);
				is_volume_connected[channel] = true;
			}
		}
	}
}


void setup_PbHub() {

	if (debug_flag) DEBUG_SERIAL.println("PaHb is detected");
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
	for (int id = 1; id < 37; id++) {
		atvsidosender.servo_connected[id] = true;
	}
}

void setup() {
	M5.begin(false, false, true);
	M5.dis.drawpix(0, 0xFF0000);

	delay(100);
	if (debug_flag) DEBUG_SERIAL.begin(115200);

	Wire1.begin(I2C_SCL, I2C_SDA);
	setup_PaHub();
	setup_PbHub();

	setup_vsidosender();
	setup_bt();

	M5.dis.drawpix(0, 0x0000FF);

	last_update_ms = millis();
}

void loop() {

	for (uint8_t a_ch = 0; a_ch < 8; a_ch++) {
		if (is_volume_connected[a_ch] == false) {
			continue;
		}

		uint8_t returnCode = tca9548a.selectChannel(a_ch);
		if (returnCode == 0) {
			for (uint8_t b_ch = 0; b_ch < 6; b_ch++) {
				uint16_t analog_value = pbhub.hub_a_read_value(PBHUB_CH_REG[b_ch]);
				if (debug_flag)
					DEBUG_SERIAL.printf(
					    "PaHub ch%d Pbhub ch%d : value=%d\r\n", a_ch, b_ch, value);

				uint8_t  servoID      = a_ch * 6 + b_ch+1;
				if (atvsidosender.servo_connected[servoID]) {
					int target_angle = map(analog_value, 0, 4095, -1350, 1350);
					atvsidosender.servo_angles[servoID] = target_angle;
				}
			}
		}
	}

	elapsed_ms = millis() - last_update_ms;

	if (elapsed_ms >= loop_interval_ms) {			
		atvsidosender.genObjectPacket(2, sendbuf, &send_len);
		VSD_SERIAL.write(sendbuf, send_len);


		last_update_ms = millis();
	}

	checkALL();
}