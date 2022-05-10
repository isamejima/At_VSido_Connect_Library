/*
 * at_vsido_connect.h
 */
#ifndef AT_VSIDO_CONNECT_LIBRARY
#define AT_VSIDO_CONNECT_LIBRARY
#include <Arduino.h>


//  データ変換用共用体
typedef union   {
    unsigned char   aucData[2];
    short           sData;
}   UNIWORD;

class At_Vsido_Connect_Library {
public:
    At_Vsido_Connect_Library();
  //VSido関連
  int MAXSERVO = 32; //サーボの数
  int servo_angles[128];//受信した全関節角度指示値
  int servo_torques[128];//受信した全関節のトルク指示値
  int servo_angles_ofset[128];//関節角度補正値（今は外部から利用するだけで内部参照していない）
  bool servo_connected[128];//全関節接続状態（今は外部から利用するだけで内部参照していない）

  //通信用
  int MAXPACKETLEN = 128;//最大パケット長（これ以上長い場合は受信を中止）
  unsigned char pc_rstr[256];//受信データ
  int pc_cnt=0;//現在受信している文字数のカウント
  unsigned char  pc_op='_';//受信中命令のオペランド（角度命令なら'o'等）
  int pc_timeout=0;//タイムアウト（今は外部から利用するだけで内部参照していない）
  int pc_ln=0;//受信メッセージ中に表記された命令長。４～MAXPACKETLENの間になるはず

  bool read1byte(unsigned char ch);//受信文字を追加。命令の受信に成功した時だけtrueを返す

  //受信処理用
  virtual bool unpack();//命令の受信に成功した後の処理。命令が適正だった時だけtrueを返す。

  //vsido関連 返信処理用
  int servo_present_angles[128];//全関節角度現在値
  int servo_present_torques[128];//全関節トルク現在値
  unsigned char servo_status[128];//サーボの接続状態

  //unpack内で返信パケットを生成
  //unsigned char r_str[128];////返信パケット
  uint8_t r_str[128];////返信パケット  
  int r_ln=0;//返信パケットの長さ  

  //statusbyte用
  bool getStatus_ServoOn(int id);
  bool getStatus_Error(int id);

  void setStatus_ServoOn(int id,bool flag);
  void setStatus_Error(int id,bool flag);

private:
  //  角度情報の統合
  short uniAngle(unsigned char upper,unsigned char lower);

  //コマンド生成
  void divAngle(short data,unsigned char*lower,unsigned char*upper);
  unsigned char calcsum(unsigned char *packet,int packet_ln);
  void gen_vsidocmd(unsigned char r_op,unsigned char *data,int data_len);

  void reset_read1byte();

};

#endif
