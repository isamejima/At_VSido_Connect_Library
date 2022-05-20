/*
 * at_vsido_connect.h
 */
#ifndef AT_VSIDO_CONNECT_LIBRARY
#define AT_VSIDO_CONNECT_LIBRARY

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

#ifndef VSIDO_MAXSREVO
#define VSIDO_MAXSERVO 128
#endif

#ifndef VSIDO_MAXPACKETLEN
#define VSIDO_MAXPACKETLEN 254
#endif

#ifndef VSIDO_MAXPACKETBUF
#define VSIDO_MAXPACKETBUF (VSIDO_MAXPACKETLEN*2)
#endif


//  データ変換用共用体
typedef union   {
    unsigned char   aucData[2];
    short           sData;
}   UNIWORD;

class At_Vsido_Connect_Library
{
  public:
    At_Vsido_Connect_Library();
    // VSido関連
    const int MAXSERVO = VSIDO_MAXSERVO; //サーボの数
    int servo_angles[VSIDO_MAXSERVO]; //受信した全関節角度指示値
    int servo_torques[VSIDO_MAXSERVO]; //受信した全関節のトルク指示値
    // vsido関連 現在値格納用
    int servo_present_angles[VSIDO_MAXSERVO];  //全関節角度現在値
    int servo_present_torques[VSIDO_MAXSERVO]; //全関節トルク現在値
    //サーボステータス用のbit
    bool servo_status_servoon[VSIDO_MAXSERVO]; //サーボオン状態
    bool servo_status_error[VSIDO_MAXSERVO];   //エラー状態

    //外部利用のみで内部参照しない値
    int servo_angles_ofset[VSIDO_MAXSERVO]; //関節角度補正値（今は外部から利用するだけで内部参照していない）
    bool servo_connected[VSIDO_MAXSERVO];   //全関節接続状態（今は外部から利用するだけで内部参照していない）
    
    //通信用
    int MAXPACKETLEN = VSIDO_MAXPACKETLEN;     //最大パケット長（これ以上長い場合は受信を中止）
    unsigned char pc_rstr[VSIDO_MAXPACKETBUF]; //受信データ
    int pc_cnt = 0;             //現在受信している文字数のカウント
    unsigned char pc_op = '_';  //受信中命令のオペランド（角度命令なら'o'等）
    int pc_ln = 0;              //受信メッセージ中に表記された命令長。４～MAXPACKETLENの間になるはず

    bool read1byte(unsigned char ch); //受信文字を追加。1パケット分の命令の受信に成功した時だけtrueを返す

    int pc_timeout = 0; //タイムアウト（今は外部から利用するだけで内部参照していない）

    //解析用
    bool unpack(); //旧nameの関数を残している
    virtual bool unpackPacket(); //renameした解析関数の本体

    // unpackPacket内で返信パケットを生成
    unsigned char r_str[VSIDO_MAXPACKETLEN];////返信パケット
    int r_ln = 0;       //返信パケットの長さ

    //isvalid関数
    bool isValidServoID(int id);
    bool isValidOP(unsigned char op);

  protected:
    //  角度情報の統合
    short uniAngle(unsigned char upper, unsigned char lower);
    void divAngle(short data, unsigned char *upper, unsigned char *lower);

    short convertToProtocol(short raw_value);
    short convertFromProtocol(short vsido_value);

    //VSidoPacket処理
    unsigned char calcSum(const unsigned char packet[], int packet_ln);
    unsigned char getStatusByte(int id);
    void genVSidoCmd(unsigned char r_op, const unsigned char data[], int data_ln);
    void resetRead1byte();
    bool isEXCEPTION_VALUE(int value);

    //解析処理
    virtual bool unpackObjectPacket();
    virtual bool unpackTorquePacket();
    virtual bool unpackDataPacket();
  };

#endif
