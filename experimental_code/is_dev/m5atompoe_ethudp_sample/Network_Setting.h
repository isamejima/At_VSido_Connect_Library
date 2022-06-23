/*
 * Network_Setting.h
 */
#ifndef NETWORK_SETTING
#define NETWORK_SETTING
#include <WiFi.h>
#include <Ethernet.h>

#define ETH		1
#define AP		2
#define STA		3 


#define COMMUNICATION_MODE  ETH		//ETH,AP,STA


	#if COMMUNICATION_MODE == ETH

	const int localPort = 8888;			// ポート番号

	const IPAddress ip(192, 168, 4, 40);		// IPアドレス(ゲートウェイも兼ねる)
	const IPAddress gateway(192, 168, 4, 0);	// (ゲートウェイ)
	const IPAddress subnet(255, 255, 255, 0);	// サブネットマスク

	#elif COMMUNICATION_MODE == AP
    
    const int localPort = 8888;			// ポート番号

	const IPAddress ip(192, 168, 4, 40);		// IPアドレス(ゲートウェイも兼ねる)
	const IPAddress gateway(192, 168, 4, 0);	// (ゲートウェイ)
	const IPAddress subnet(255, 255, 255, 0);	// サブネットマスク


	const char ssid[] = "ESP32WiFi";	// SSID
	const char pass[] = "esp32pass";	// password
	const int localPort = 8888;			// ポート番号

	const IPAddress ip(192, 168, 4, 40);		// IPアドレス(ゲートウェイも兼ねる)
	const IPAddress gateway(192, 168, 4, 0);	// (ゲートウェイ)
	const IPAddress subnet(255, 255, 255, 0);	// サブネットマスク

	#elif COMMUNICATION_MODE == STA	
	const char ssid[] = "Knet";	// SSID
	const char pass[] = "9e29366d213523346617f1b851d9bf8e3ef2f05a62e42bd1a207fbbcdf5fd3f5";	// password
	const int localPort = 8888;			// ポート番号

	const IPAddress ip(192, 168, 11, 40);		// IPアドレス(ゲートウェイも兼ねる)
	const IPAddress gateway(192, 168,11, 0);	// (ゲートウェイ)
	const IPAddress subnet(255, 255, 255, 0);	// サブネットマスク

	#endif
#endif