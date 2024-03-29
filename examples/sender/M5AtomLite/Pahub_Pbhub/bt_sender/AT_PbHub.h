#ifndef __AT_PBHUB_H__
#define __AT_PBHUB_H__

#include <Wire.h>

#define IIC_ADDR1 0x61
#define IIC_ADDR2 0x62
#define IIC_ADDR3 0x63
#define IIC_ADDR4 0x64
#define IIC_ADDR5 0x65
#define IIC_ADDR6 0x66
#define IIC_ADDR7 0x67
#define IIC_ADDR8 0x68

#define CH0_ADDR 0x40
#define CH1_ADDR 0x50
#define CH2_ADDR 0x60
#define CH3_ADDR 0x70
#define CH4_ADDR 0x80
#define CH5_ADDR 0xA0


class AT_PbHub {
  public:
    AT_PbHub();
    AT_PbHub(uint8_t iic_addr);

    void setWire(TwoWire *pWire);
    void setAddress(uint8_t address);

    uint16_t hub_a_read_value(uint8_t reg);

    uint8_t hub_d_read_value_A(uint8_t reg);
    uint8_t hub_d_read_value_B(uint8_t reg);

    void hub_d_wire_value_A(uint8_t reg, uint16_t level);
    void hub_d_wire_value_B(uint8_t reg, uint16_t level);

    void hub_a_wire_value_A(uint8_t reg, uint16_t duty);
    void hub_a_wire_value_B(uint8_t reg, uint16_t duty);

    void hub_wire_length(uint8_t reg, uint16_t length);
    void hub_wire_index_color(uint8_t reg, uint16_t num, uint8_t r, int8_t g, uint8_t b);
    void hub_wire_fill_color(uint8_t reg, uint16_t first, uint16_t count, uint8_t r, int8_t g, uint8_t b);
    void hub_wire_setBrightness(uint8_t reg, uint8_t brightness);

    void hub_wire_setServoAngle_A(uint8_t reg, uint8_t angle);
    void hub_wire_setServoAngle_B(uint8_t reg, uint8_t angle);

    void hub_wire_setServoPulse_A(uint8_t reg, uint16_t pulse);
    void hub_wire_setServoPulse_B(uint8_t reg, uint16_t pulse);

  public:
  private:
    TwoWire* _pWire;
    uint8_t _iic_addr = IIC_ADDR1;

  private:
};

#endif
