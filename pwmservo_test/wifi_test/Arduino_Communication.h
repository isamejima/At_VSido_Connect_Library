#ifndef ARDUINO_COMMUNICATION
#define ARDUINO_COMMUNICATION

#include <WiFi.h>
#include <WiFiUdp.h>

#include "Network_Setting.h"


class Arduino_Commnunication {
public:
    Arduino_Commnunication();
	
	int init(void);//初期化

private:

};

#endif