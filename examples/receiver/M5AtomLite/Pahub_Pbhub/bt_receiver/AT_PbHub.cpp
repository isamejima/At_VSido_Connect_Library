#include "AT_PbHub.h"

AT_PbHub::AT_PbHub() {
      _pWire=&Wire;
}

AT_PbHub::AT_PbHub(uint8_t iic_addr) {
    _pWire=&Wire;
    _iic_addr = iic_addr;
}

void AT_PbHub::setAddress(uint8_t address) {
   _iic_addr=address;
}

void AT_PbHub::setWire(TwoWire* pWire) {
  _pWire=pWire;
}

uint16_t AT_PbHub::hub_a_read_value(uint8_t reg) {
    _pWire->beginTransmission(_iic_addr);
    _pWire->write(reg | 0x06);
    _pWire->endTransmission();

    uint8_t RegValue_L, RegValue_H;

    _pWire->requestFrom(_iic_addr, 2);
    while (_pWire->available()) {
        RegValue_L = _pWire->read();
        RegValue_H = _pWire->read();
    }

    return (RegValue_H << 8) | RegValue_L;
}

uint8_t AT_PbHub::hub_d_read_value_A(uint8_t reg) {
    _pWire->beginTransmission(_iic_addr);
    _pWire->write(reg | 0x04);
    _pWire->endTransmission();

    uint8_t RegValue;

    _pWire->requestFrom(_iic_addr, 1);
    while (_pWire->available()) {
        RegValue = _pWire->read();
    }
    return RegValue;
}

uint8_t AT_PbHub::hub_d_read_value_B(uint8_t reg) {
    _pWire->beginTransmission(_iic_addr);
    _pWire->write(reg | 0x05);
    _pWire->endTransmission();

    uint8_t RegValue;

    _pWire->requestFrom(_iic_addr, 1);
    while (_pWire->available()) {
        RegValue = _pWire->read();
    }
    return RegValue;
}

void AT_PbHub::hub_d_wire_value_A(uint8_t reg, uint16_t level) {
    _pWire->beginTransmission(_iic_addr);
    _pWire->write(reg | 0x00);
    _pWire->write(level & 0xff);
    _pWire->endTransmission();
}

void AT_PbHub::hub_d_wire_value_B(uint8_t reg, uint16_t level) {
    _pWire->beginTransmission(_iic_addr);
    _pWire->write(reg | 0x01);
    _pWire->write(level & 0xff);
    _pWire->endTransmission();
}

void AT_PbHub::hub_a_wire_value_A(uint8_t reg, uint16_t duty) {
    _pWire->beginTransmission(_iic_addr);
    _pWire->write(reg | 0x02);
    _pWire->write(duty & 0xff);
    _pWire->endTransmission();
}

void AT_PbHub::hub_a_wire_value_B(uint8_t reg, uint16_t duty) {
    _pWire->beginTransmission(_iic_addr);
    _pWire->write(reg | 0x03);
    _pWire->write(duty & 0xff);
    _pWire->endTransmission();
}

void AT_PbHub::hub_wire_length(uint8_t reg, uint16_t length) {
    _pWire->beginTransmission(_iic_addr);
    _pWire->write(reg | 0x08);
    _pWire->write(length & 0xff);
    _pWire->write(length >> 8);
    _pWire->endTransmission();
}

void AT_PbHub::hub_wire_index_color(uint8_t reg, uint16_t num, uint8_t r,
                                   int8_t g, uint8_t b) {
    _pWire->beginTransmission(_iic_addr);
    _pWire->write(reg | 0x09);
    _pWire->write(num & 0xff);
    _pWire->write(num >> 8);
    _pWire->write(r);
    _pWire->write(g);
    _pWire->write(b);
    _pWire->endTransmission();
}

void AT_PbHub::hub_wire_fill_color(uint8_t reg, uint16_t first, uint16_t count,
                                  uint8_t r, int8_t g, uint8_t b) {
    _pWire->beginTransmission(_iic_addr);
    _pWire->write(reg | 0x0a);
    _pWire->write(first & 0xff);
    _pWire->write(first >> 8);

    _pWire->write(count & 0xff);
    _pWire->write(count >> 8);

    _pWire->write(r);
    _pWire->write(g);
    _pWire->write(b);
    _pWire->endTransmission();
}

void AT_PbHub::hub_wire_setBrightness(uint8_t reg, uint8_t brightness) {
    _pWire->beginTransmission(_iic_addr);
    _pWire->write(reg | 0x0b);
    _pWire->write(brightness & 0xff);
    _pWire->endTransmission();
}
