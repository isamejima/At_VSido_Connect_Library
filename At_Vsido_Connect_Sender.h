/*
 * At_Vsido_Connect_Sender.h
 */
#ifndef AT_VSIDO_CONNECT_SENDER
#define AT_VSIDO_CONNECT_SENDER

#include "At_Vsido_Connect_Control_Table.h"
#include "At_Vsido_Connect_Library.h"

class At_Vsido_Connect_Sender : public At_Vsido_Connect_Library
{
public:
    bool unpackPacket() override; // renameした解析関数の本体

    void setCycle(int cyc);
    void setObjectPacketParam(int id, int angle);
    void setToruqePacketParam(int id, int torque);
    void setDataPacketParam(int id, int dad, int dln);

    bool genDataPacket(unsigned char *packet, int *packet_ln);
    bool genObjectPacket(int cyc,unsigned char *packet, int *packet_ln);
    bool genTorquePacket(int cyc,unsigned char *packet, int *packet_ln);

protected:
    bool servo_sent[VSIDO_MAXSERVO]; //送信したかどうか

    int _sent_dad[VSIDO_MAXSERVO];
    int _sent_dln[VSIDO_MAXSERVO];

    void setStatusByte(int id, unsigned char status_byte);

    //解析処理
    virtual bool unpackObjectPacket() override;
    virtual bool unpackTorquePacket() override;
    virtual bool unpackDataPacket() override;

    // control tableまわり
    int calcReturnServoNum();
    int calcReturnDataLen();

    AT_Vsido_Control_Table getControlTable(int id);
    void setControlTable(int id, AT_Vsido_Control_Table table);
    AT_Vsido_Control_Table TableConvertToProtocol(AT_Vsido_Control_Table raw_table);
    AT_Vsido_Control_Table TableConvertFromProtocol(AT_Vsido_Control_Table vsido_table);
};

//AT_VSIDO_CONNECT_SENDER
#endif
