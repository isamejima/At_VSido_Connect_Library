#include <Wire.h>

#include "AT_TCA9548A.h"

AT_TCA9548A::AT_TCA9548A() {
  _pWire=&Wire;
}

AT_TCA9548A::AT_TCA9548A(uint8_t address):_address(address) {    
  _pWire=&Wire;
}

void AT_TCA9548A::address(uint8_t address) {    
	_address = address;
}

void AT_TCA9548A::setWire(TwoWire* pWire)
{
  _pWire=pWire;  
}
    
uint8_t AT_TCA9548A::getChannel() {
	return _currentChannel;
}

uint8_t AT_TCA9548A::selectChannel(uint8_t channel) {
	uint8_t result = 0xff;
	if (channel >= 0 && channel < TCA9548A_MAX_CHANNELS) {
		_pWire->beginTransmission(_address);
		_pWire->write( ((uint8_t)1) << (channel));	
		_currentChannel = channel;
		result = _pWire->endTransmission();
	} 
	return result;
}

uint8_t AT_TCA9548A::nextChannel() {
	uint8_t nextChannel = _currentChannel + 1;
	if (nextChannel > (TCA9548A_MAX_CHANNELS-1)) {
		nextChannel = 0;	
	}

	return selectChannel(nextChannel);
}
 
