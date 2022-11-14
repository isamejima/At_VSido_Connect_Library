#ifndef AT_TCA9548A_H
#define AT_TCA9548A_H

#include <Arduino.h>

//IC MAX is 8. but Pahub can uuse only six.
#define TCA9548A_MAX_CHANNELS 6

class AT_TCA9548A
{
  public:
    AT_TCA9548A();
    AT_TCA9548A(uint8_t address);
    void address(uint8_t address);

    void setWire(TwoWire *pWire);

    uint8_t getChannel();

    uint8_t selectChannel(uint8_t channel);
    uint8_t nextChannel();

  private:
    TwoWire *_pWire;
    uint8_t _address;
    uint8_t _currentChannel;

};
#endif //AT_TCA9548A_H
