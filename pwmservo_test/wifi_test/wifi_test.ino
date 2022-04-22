#include <M5Atom.h>
#include <WiFi.h>
#include <WiFiUdp.h>
 
 
const char ssid[] = "ESP32_wifi"; // SSID
const char pass[] = "esp32pass";  // password
const int localPort = 8888;      // ポート番号

const IPAddress ip(192, 168, 4, 40);       // IPアドレス(ゲートウェイも兼ねる)
const IPAddress gateway(192, 168, 4, 0);       // (ゲートウェイ)
const IPAddress subnet(255, 255, 255, 0); // サブネットマスク

WiFiUDP udp;

void setup() {
  
  M5.begin(true, false, true);

  delay(100);

  Serial.begin(115200);

  WiFi.softAP(ssid, pass);           // SSIDとパスの設定
  delay(100);                        // 追記：このdelayを入れないと失敗する場合がある
  WiFi.softAPConfig(ip, gateway, subnet); // IPアドレス、ゲートウェイ、サブネットマスクの設定

  Serial.print("AP IP address: ");
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);

  Serial.println("Starting UDP");
  udp.begin(localPort);  // UDP通信の開始(引数はポート番号)

  Serial.print("Local port: ");
  Serial.println(localPort);

}

void loop() {
  char packetBuffer[128]={0};

  M5.update();
  
  delay(100);

  int packetSize = udp.parsePacket();
 
  if (packetSize){

    udp.read(packetBuffer, packetSize);

    Serial.println("received");
    Serial.println(packetBuffer);

    delay(100);

  }
}
