#include "At_Vsido_Connect_Sender.h"

static const int VSIDO_EXCEPTION_VALUE = 0x1fff; // 13bitの最大値

// statusbyte処理用のbit演算関係
static const unsigned int BIT_FLAG_0 = (1 << 0); // 0000 0000 0000 0001
static const unsigned int BIT_FLAG_1 = (1 << 1); // 0000 0000 0000 0010
static const unsigned int BIT_FLAG_2 = (1 << 2); // 0000 0000 0000 0100
static const unsigned int BIT_FLAG_3 = (1 << 3); // 0000 0000 0000 1000
static const unsigned int BIT_FLAG_4 = (1 << 4); // 0000 0000 0001 0000
static const unsigned int BIT_FLAG_5 = (1 << 5); // 0000 0000 0010 0000
static const unsigned int BIT_FLAG_6 = (1 << 6); // 0000 0000 0100 0000
static const unsigned int BIT_FLAG_7 = (1 << 7); // 0000 0000 1000 0000

// error
static const unsigned int MASK_ERROR = BIT_FLAG_0;
// servo on
static const unsigned int MASK_SERVOON = BIT_FLAG_1;

void At_Vsido_Connect_Sender::setCycle(int cyc)
{
    for (int id = 0; id < MAXSERVO; id++)
    {
        if (servo_connected[id])
        {
            servo_cycle[id] = cyc;
        }
    }
}

void At_Vsido_Connect_Sender::setObjectPacketParam(int id, int angle)
{
    if (!isValidServoID(id))
        return;

    if (servo_connected[id] == false)
    {
        servo_connected[id] = true;
    }
    servo_angles[id] = angle;
}
void At_Vsido_Connect_Sender::setToruqePacketParam(int id, int torque)
{
    if (!isValidServoID(id))
        return;

    if (servo_connected[id] == false)
    {
        servo_connected[id] = true;
    }
    servo_torques[id] = torque;
}

bool At_Vsido_Connect_Sender::genObjectPacket(int cyc, unsigned char *packet, int *packet_ln)
{
    unsigned char data[MAXSERVO * 3] = {};
    unsigned char lower = 0;
    unsigned char upper = 0;
    int cnt = 0;

    data[cnt++] = (unsigned char)cyc;

    for (int id = 1; id < MAXSERVO; id++)
    {
        if (servo_connected[id])
        {
            data[cnt++] = id;
            divAngle(servo_angles[id], &upper, &lower);
            data[cnt++] = upper;
            data[cnt++] = lower;
        }
    }

    if (cnt == 1)
    {
        return false;
    }
    genVSidoPacket('o', data, cnt, packet, packet_ln);

    if (packet_ln == 0)
        return false;

    return true;
}

bool At_Vsido_Connect_Sender::genTorquePacket(int cyc, unsigned char *packet, int *packet_ln)
{
    unsigned char data[MAXSERVO * 3] = {};
    unsigned char lower = 0;
    unsigned char upper = 0;
    int cnt = 0;

    data[cnt++] = (unsigned char)cyc;

    for (int id = 1; id < MAXSERVO; id++)
    {
        if (servo_connected[id])
        {
            data[cnt++] = id;
            divAngle(servo_torques[id], &upper, &lower);
            data[cnt++] = upper;
            data[cnt++] = lower;
        }
    }

    if (cnt == 1)
    {
        return false;
    }

    genVSidoPacket('t', data, cnt, packet, packet_ln);

    return true;
}
void At_Vsido_Connect_Sender::setControlTable(int id, AT_Vsido_Control_Table table)
{
}
AT_Vsido_Control_Table At_Vsido_Connect_Sender::getControlTable(int id)
{
    AT_Vsido_Control_Table table = {0};
    if (!isValidServoID(id))
        return table;

    table.present_angle = servo_present_angles[id];
    table.present_angle = servo_present_angles[id];

    return table;
}

AT_Vsido_Control_Table At_Vsido_Connect_Sender::TableConvertToProtocol(AT_Vsido_Control_Table raw_table)
{
    AT_Vsido_Control_Table table = raw_table;

    table.present_angle = convertToProtocol(raw_table.present_angle);
    table.present_torque = convertToProtocol(raw_table.present_torque);

    table.present_speed = convertToProtocol(raw_table.present_speed);
    table.present_time = convertToProtocol(raw_table.present_time);

    return table;
}

AT_Vsido_Control_Table At_Vsido_Connect_Sender::TableConvertFromProtocol(AT_Vsido_Control_Table vsido_table)
{
    AT_Vsido_Control_Table table = vsido_table;

    table.present_angle = convertFromProtocol(vsido_table.present_angle);
    table.present_torque = convertFromProtocol(vsido_table.present_torque);

    table.present_speed = convertFromProtocol(vsido_table.present_speed);
    table.present_time = convertFromProtocol(vsido_table.present_time);

    return table;
}

int At_Vsido_Connect_Sender::calcReturnDataLen()
{
    int r_len = 0;

    for (int id = 1; id < MAXPACKETLEN; id++)
    {
        if (_sent_dln[id] != 0)
        {
            r_len += _sent_dln[id];
        }
    }
    return r_len;
}

int At_Vsido_Connect_Sender::calcReturnServoNum()
{
    int r_num = 0;

    for (int id = 1; id < MAXPACKETLEN; id++)
    {
        if (_sent_dln[id] != 0)
        {
            r_num++;
        }
    }
    return r_num;
}

bool At_Vsido_Connect_Sender::unpackDataPacket()
{
    if ((pc_ln - 4) != calcReturnDataLen())
    {
        return false;
    }

    int read_offset = 4;           // header op len　cycの4つを読み飛ばす
    int data_offset = read_offset; // 読みだしたdatの長さ

    while (data_offset < pc_ln)
    {
        int servo_id = pc_rstr[data_offset];
        data_offset++;

        AT_Vsido_Control_Table table = getControlTable(servo_id); // table取得

        table = TableConvertToProtocol(table); // vsidotableに変換

        int dad = _sent_dad[servo_id];
        int dln = _sent_dln[servo_id];
        if (dad + dln > sizeof(AT_Vsido_Control_Table) / sizeof(unsigned char))
            return false;

        unsigned char *table_ptr = &(table.reserved[0]);
        for (int i = 0; i < dln; i++)
        {
            table_ptr[dad + i] = pc_rstr[data_offset + i];
        }
        table = TableConvertFromProtocol(table); // vsido型からrawに変換
        setControlTable(servo_id, table);        //値をセット
        data_offset += dln;
    }
    return true;
}

void At_Vsido_Connect_Sender::setStatusByte(int id, unsigned char status_byte)
{
    if (!isValidServoID(id))
    {
        return;
    }
    servo_status_error[id] &= MASK_ERROR;
    servo_status_servoon[id] &= MASK_SERVOON;
}

bool At_Vsido_Connect_Sender::unpackObjectPacket()
{
    //データ構造がおかしければfalse
    // cycle id1 sb1 angle1_l angle1_h id2 sb2 angle2_l angle2_h…etc
    if ((pc_ln - 4 - 1) % 4 != 0)
    {
        At_Vsido_Connect_Library::resetRead1byte();
        return false;
    }

    int servo_num = (pc_ln - 4 - 1) / 4;
    int read_offset = 4; // header op len　cycの4つを読み飛ばす

    //各id毎の処理
    for (int i = 0; i < servo_num; i++)
    {
        //値の復元
        int servo_id = pc_rstr[read_offset + i * 4 + 0];
        if (!isValidServoID(servo_id))
        {
            return false; //適正なサーボIDが含まれていたらfalse
        }

        unsigned char status_byte = pc_rstr[read_offset + i * 4 + 1];
        setStatusByte(servo_id, status_byte);

        int servo_angle =
            uniAngle(pc_rstr[read_offset + i * 4 + 2], pc_rstr[read_offset + i * 4 + 3]);
        //例外値でなければ更新
        if (!isEXCEPTION_VALUE(servo_angle))
        {
            servo_present_angles[servo_id] = servo_angle;
        }
    }

    return true;
}

bool At_Vsido_Connect_Sender::unpackTorquePacket()
{
    //データ構造がおかしければfalse
    // cycle id1 sb1 angle1_l angle1_h id2 sb2 angle2_l angle2_h…etc
    if ((pc_ln - 4 - 1) % 4 != 0)
    {
        resetRead1byte();
        return false;
    }

    int servo_num = (pc_ln - 4 - 1) / 4;
    int read_offset = 4; // header op len　cycの4つを読み飛ばす

    //各id毎の処理
    for (int i = 0; i < servo_num; i++)
    {
        //値の復元
        int servo_id = pc_rstr[read_offset + i * 4 + 0];
        if (!isValidServoID(servo_id))
        {
            return false; //適正なサーボIDが含まれていたらfalse
        }

        unsigned char status_byte = pc_rstr[read_offset + i * 4 + 1];
        setStatusByte(servo_id, status_byte);

        int servo_torque =
            uniAngle(pc_rstr[read_offset + i * 4 + 2], pc_rstr[read_offset + i * 4 + 3]);
        //例外値でなければ更新
        if (!isEXCEPTION_VALUE(servo_torque))
        {
            servo_present_torques[servo_id] = servo_torque;
        }
    }

    return true;
}

bool At_Vsido_Connect_Sender::unpackPacket()
{
    r_ln = 0;

    //オペランドが'!'の処理
    if (pc_op == '!')
    {
        //何もしない
        return true;
    }

    if (pc_op == 'd')
    {
        return unpackDataPacket();
    }

    //オペランドが'o'なら目標角度登録
    if (pc_op == 'o')
    {
        return unpackObjectPacket();
    }
    //オペランドが't'なら目標トルク登録
    if (pc_op == 't')
    {
        return unpackTorquePacket();
    }

    return false;
}

void At_Vsido_Connect_Sender::setDataPacketParam(int id, int dad, int dln)
{
    if (!isValidServoID(id))
    {
        return;
    }

    _sent_dad[id] = dad;
    _sent_dln[id] = dln;
}
bool At_Vsido_Connect_Sender::genDataPacket(unsigned char *packet, int *packet_ln)
{
    unsigned char data[VSIDO_MAXPACKETLEN];
    int cnt = 0;

    for (int id = 1; id < MAXSERVO; id++)
    {
        if (_sent_dln[id] != 0)
        {
            data[cnt++] = id;
            data[cnt++] = _sent_dad[id];
            data[cnt++] = _sent_dln[id];
        }
    }

    genVSidoPacket('d', data, cnt, packet, packet_ln);
}
