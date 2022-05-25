/*
 * at_vsido_connect.h
 * ライブラリの作り方で参考にしたURL https://stupiddog.jp/note/archives/266
 */
#include "At_Vsido_Connect_Library.h"

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

// error fN
static const unsigned int MASK_ERROR = BIT_FLAG_0;
// servo on
static const unsigned int MASK_SERVOON = BIT_FLAG_1;

At_Vsido_Connect_Library::At_Vsido_Connect_Library()
{
  for(int id=0;id<MAXSERVO;id++){
  servo_angles[id]=0;
  servo_present_angles[id] = 0;
  servo_torques[id] = 0;
  servo_present_torques[id] = 0;
  
  servo_status_servoon[id]=false;
  servo_status_error[id]=false;
  
  servo_connected[id] = false;
  }
}

short At_Vsido_Connect_Library::convertToProtocol(short raw_value)
{
  UNIWORD uniWord; // データ変換用共用体
  uniWord.sData = raw_value;
  uniWord.sData = (uniWord.sData << 1);
  uniWord.aucData[1] = (uniWord.aucData[1] << 1);
  return uniWord.sData;
}
short At_Vsido_Connect_Library::convertFromProtocol(short vsido_value)
{
  UNIWORD uniWord; // データ変換用共用体
  uniWord.sData = vsido_value;
  uniWord.aucData[1] = (uniWord.aucData[1] & 0x80) | (uniWord.aucData[1] >> 1);
  uniWord.sData = (uniWord.sData & 0x8000) | (uniWord.sData >> 1);
  return uniWord.sData;
}

//  角度情報の統合
short At_Vsido_Connect_Library::uniAngle(unsigned char upper,
                                         unsigned char lower)
{
  UNIWORD uniWord; // データ変換用共用体
  uniWord.aucData[0] = upper;
  uniWord.aucData[1] = lower;

  short raw_angle=convertFromProtocol(uniWord.sData);
  return raw_angle;

/*
  UNIWORD uniWord; // データ変換用共用体
  uniWord.aucData[0] = upper;
  uniWord.aucData[1] = (lower & 0x80) | (lower >> 1);
  uniWord.sData = (uniWord.sData & 0x8000) | (uniWord.sData >> 1);

  return uniWord.sData;
  */
}
//  角度情報の分割
void At_Vsido_Connect_Library::divAngle(short data, unsigned char *upper,
                                        unsigned char *lower)
{
  UNIWORD uniWord; // データ変換用共用体
  uniWord.sData=convertToProtocol(data);
  *upper = uniWord.aucData[0];
  *lower = uniWord.aucData[1];

  /*
    UNIWORD uniWord; // データ変換用共用体
    uniWord.sData = data;
    uniWord.sData = (uniWord.sData << 1);
    *upper = uniWord.aucData[0];
    *lower = (uniWord.aucData[1] << 1);
    */
}

void At_Vsido_Connect_Library::resetRead1byte()
{
  pc_cnt = 0;
  pc_ln = 0;
  pc_op = '_';

  r_ln = 0; //返信データなし
}

//一文字ずつ読み込み
bool At_Vsido_Connect_Library::read1byte(unsigned char ch)
{
  pc_rstr[pc_cnt] = ch;
  //受信パケットが長すぎる場合
  if (pc_cnt > MAXPACKETLEN)
  {
    resetRead1byte();
    return false;
  }
  //最初の文字が0xFF（スタートフラグ）ではない場合
  if (ch != 0xFF && pc_cnt == 0)
    return false;

  if (ch == 0xFF)
  { //スタートフラグ
    resetRead1byte();
    pc_rstr[pc_cnt] = ch;
  }
  else if (pc_cnt == 1)
  { //オペランド

    if (!isValidOP(ch))
    { //知らないオペランドの場合は受信をリセット
      resetRead1byte();
      return false;
    }
    pc_op = ch;
  }
  else if (pc_cnt == 2)
  { //命令長
    pc_ln = ch;
    if (pc_ln < 4 ||
        pc_ln >= MAXPACKETLEN)
    { //パケット長が長すぎる場合は受信をリセット
      resetRead1byte();
      return false;
    }
  }
  else if (pc_cnt == pc_ln - 1 && pc_ln >= 4)
  { //チェックサム

    unsigned char sum = 0;
    for (int j = 0; j < pc_ln; j++)
    {
      sum = sum ^ pc_rstr[j];
    }
    //チェックサムが合わない場合は受信をリセット
    if (sum != 0)
    {
      resetRead1byte();
      return false;
    }
    //受信がすべてうまくいった場合trueを返す
    return true;
  }
  pc_cnt++;
  return false;
}

bool At_Vsido_Connect_Library::unpackDataPacket()
{
  //データ構造がおかしければfalse
  // id1 dad1 dln1 id2 dad2 dln2 …etc

  if ((pc_ln - 4) % 3 != 0)
  {
    resetRead1byte();
    return false;
  }

  int servo_num = (pc_ln - 4) / 3;
  int read_offset = 3; // header op lenの3つを読み飛ばす

  unsigned char r_op = pc_op;
  unsigned char r_data[128];
  int r_cnt = 0;

  //各id毎の処理
  for (int i = 0; i < servo_num; i++)
  {
    //値の復元
    int servo_id = pc_rstr[read_offset + i * 3 + 0];
    int dad = pc_rstr[read_offset + i * 3 + 1];
    int dln = pc_rstr[read_offset + i * 3 + 2];

    if (dad != 19 && dad != 25)
    {
      //現状、特定のdad以外はエラーとする
      resetRead1byte();
      return false;
    }

    if (dad == 19 && dln == 2)
    {
      //返信データ　statusbyte＋角度
      unsigned char upper, lower;
      divAngle(servo_present_angles[servo_id], &upper, &lower);
      r_data[r_cnt++] = servo_id;
      r_data[r_cnt++] = upper;
      r_data[r_cnt++] = lower;
    }
    else if (dad == 25 && dln == 2)
    {
      //返信データ トルク
      unsigned char upper, lower;
      divAngle(servo_present_angles[servo_id], &upper, &lower);
      r_data[r_cnt++] = servo_id;
      r_data[r_cnt++] = upper;
      r_data[r_cnt++] = lower;
    }
    else
    {
      //現時点では、特定のdad,dlnの組み合わせ以外はエラーとする
      resetRead1byte();
      return false;
    }
  }

  //返信処理
  genVSidoCmd(r_op, r_data, r_cnt);
  return true;
}

bool At_Vsido_Connect_Library::unpackObjectPacket()
{
  //データ構造がおかしければfalse
  // cycle id1 angle1_l angle1_h id2 angle2_l angle2_h…etc
  if ((pc_ln - 4 - 1) % 3 != 0)
  {
    resetRead1byte();
    return false;
  }

  int servo_num = (pc_ln - 4 - 1) / 3;
  int read_offset = 4; // header op len　cycの4つを読み飛ばす

  unsigned char r_op = pc_op;
  unsigned char r_data[128];
  int r_cnt = 0;

  //各id毎の処理
  for (int i = 0; i < servo_num; i++)
  {
    //値の復元
    int servo_id = pc_rstr[read_offset + i * 3 + 0];
    /*
    if (!isValidServoID(servo_id))
    {
      return false; //適正なサーボIDが含まれていたらfalse
    }
    */

    int servo_angle =
        uniAngle(pc_rstr[read_offset + i * 3 + 1], pc_rstr[read_offset + i * 3 + 2]);
    //例外値でなければ更新
    if (!isEXCEPTION_VALUE(servo_angle))
    {
      servo_angles[servo_id] = servo_angle;
    }

    //返信データ
    unsigned char lower, upper;
    divAngle(servo_present_angles[servo_id], &upper, &lower);
    r_data[r_cnt++] = servo_id;
    r_data[r_cnt++] = getStatusByte(servo_id);
    r_data[r_cnt++] = upper;
    r_data[r_cnt++] = lower;
  }

  //返信処理
  genVSidoCmd(r_op, r_data, r_cnt);
  return true;
}

bool At_Vsido_Connect_Library::unpackTorquePacket()
{
  //データ構造がおかしければfalse
  // cycle id1 torque1_l torque1_h id2 torque2_l torque2_h…etc
  if ((pc_ln - 4 - 1) % 3 != 0)
  {
    resetRead1byte();
    return false;
  }

  int servo_num = (pc_ln - 4 - 1) / 3;

  int read_offset = 4; // header op len　cycの4つを読み飛ばす
  unsigned char r_op = pc_op;
  unsigned char r_data[128];
  int r_cnt = 0;

  //各id毎の処理
  for (int i = 0; i < servo_num; i++)
  {
    //値の復元
    int servo_id = pc_rstr[read_offset + i * 3];
    if (!isValidServoID(servo_id))
    {
      return false; //適正なサーボIDが含まれていたらfalse
    }

    int servo_torque =
        uniAngle(pc_rstr[read_offset + i * 3 + 1], pc_rstr[read_offset + i * 3 + 2]);
    //例外値でなければ更新
    if (!isEXCEPTION_VALUE(servo_torque))
    {
      servo_torques[servo_id] = servo_torque;
    }
    //返信データ
    unsigned char lower, upper;
    divAngle(servo_present_torques[servo_id], &upper, &lower);
    r_data[r_cnt++] = servo_id;
    r_data[r_cnt++] = getStatusByte(servo_id);
    r_data[r_cnt++] = upper;
    r_data[r_cnt++] = lower;
  }

  //返信処理
  genVSidoCmd(r_op, r_data, r_cnt);
  return true;
}

bool At_Vsido_Connect_Library::unpackPacket()
{
  r_ln = 0;

  //オペランドが'!'の処理
  if (pc_op == '!')
  {
    genVSidoCmd('!', NULL, 0);

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

  //規定のない場合はackを返す
  genVSidoCmd('!', NULL, 0);
  return true;
}

bool At_Vsido_Connect_Library::unpack()
{
  return unpackPacket();
}

unsigned char At_Vsido_Connect_Library::calcSum(const unsigned char packet[],
                                                int packet_ln)
{
  unsigned char sum = 0;
  for (int i = 0; i < packet_ln; i++)
  {
    sum = sum ^ packet[i];
  }
  return sum;
}

void At_Vsido_Connect_Library::genVSidoPacket(unsigned char op, const unsigned char data[], int data_ln, unsigned char *packet, int *packet_len)
{
  int cnt = 0;
  *packet_len = 0;

  //opが不適の場合処理しない
  if(!isValidOP(op))
  {
    return;
  }

  packet[cnt++] = 0xff;
  packet[cnt++] = op;
  packet[cnt++] = 4 + data_ln;

  for (int i = 0; i < data_ln; i++)
  {
    packet[cnt++] = data[i];
  }

  unsigned char sum = calcSum(packet, cnt);
  packet[cnt++] = sum;
  *packet_len = cnt;
}

void At_Vsido_Connect_Library::genVSidoCmd(unsigned char r_op, const unsigned char data[], int data_ln)
{
  genVSidoPacket(r_op,data,data_ln,r_str,&r_ln);
}

unsigned char At_Vsido_Connect_Library::getStatusByte(int id)
{
  if (!isValidServoID(id))
    return MASK_ERROR;

  unsigned char status_byte = 0;
  if (servo_status_error[id])
  {
    status_byte |= MASK_ERROR;
  }
  if (servo_status_servoon[id])
  {
    status_byte |= MASK_SERVOON;
  }
  return status_byte;
}

bool At_Vsido_Connect_Library::isEXCEPTION_VALUE(int value)
{
  if (value == VSIDO_EXCEPTION_VALUE)
  {
    return true;
  }
  return false;
}

bool At_Vsido_Connect_Library::isValidServoID(int id)
{
  if(id>=1 && id<=MAXSERVO){
    return true;
  }

  if (id < 1)
  {
    return false;
  }

  if (id >= MAXSERVO)
  {
    return false;
  }

  return false;
}

bool At_Vsido_Connect_Library::isValidOP(unsigned char ch)
{

  // asratec 予約文字
  if (ch >= 'a' && ch <= 'z')
    return true;

  // asratec 予約文字
  if (ch == 'M' || ch == '!')
    return true;

  //ユーザー利用可能　文字
  if (ch >= '0' && ch <= '9')
    return true;
  if (ch >= 'A' && ch <= 'Z')
    return true;

  return false;
}
