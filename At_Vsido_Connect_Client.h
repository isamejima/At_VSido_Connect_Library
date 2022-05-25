/*
 * at_vsido_connect_client.h
 */
#ifndef AT_VSIDO_CONNECT_CLIENT
#define AT_VSIDO_CONNECT_CLIENT

#include "At_Vsido_Connect_Library.h"
#include "At_Vsido_Connect_Control_Table.h"

class At_Vsido_Connect_Client : public At_Vsido_Connect_Library
{
public:
    bool unpackPacket(); // renameした解析関数の本体
    void setDataPacketParam(int id, int dad, int dln);
    bool genDataPacket(unsigned char *packet,int* packet_ln);

protected:       
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
