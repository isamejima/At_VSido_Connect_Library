/*
 * at_vsido_connect_client.h
 */
#ifndef AT_VSIDO_CONNECT_CLIENT
#define AT_VSIDO_CONNECT_CLIENT

#include "At_Vsido_Connect_Library.h"

#pragma pack(1)
typedef struct
{
    unsigned char tmp[18];
    short present_angle;  // 19-20
    short present_time;   // 21-22
    short present_speed;  // 23-24
    short present_torque; // 25-26
} AT_Vsido_Control_Table;
#pragma pack()

class At_Vsido_Connect_Client : public At_Vsido_Connect_Library
{
public:
    bool unpackPacket(); // renameした解析関数の本体

    protected:       
    int _sent_id[VSIDO_MAXSERVO];
    int _sent_dad[VSIDO_MAXSERVO];
    int _sent_dln[VSIDO_MAXSERVO];

    int calcReturnServoNum();
    int calcReturnDataLen();

    void setStatusByte(int id, unsigned char status_byte);
    //解析処理
    virtual bool unpackObjectPacket();
    virtual bool unpackTorquePacket();
    virtual bool unpackDataPacket();

    // control tableまわり
    AT_Vsido_Control_Table getControlTable(int id);
    void setControlTable(int id, AT_Vsido_Control_Table table);
    AT_Vsido_Control_Table TableConvertToProtocol(AT_Vsido_Control_Table raw_table);
    AT_Vsido_Control_Table TableConvertFromProtocol(AT_Vsido_Control_Table vsido_table);
};

// AT_VSIDO_CONNECT_CLIENT
#endif
