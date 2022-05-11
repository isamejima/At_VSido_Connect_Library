/*
 * at_vsido_connect.h
 * ライブラリの作り方で参考にしたURL https://stupiddog.jp/note/archives/266
 */
#include "At_Vsido_Connect_Library.h"
#include <Arduino.h>

// statusbyte処理用のbit演算関係
static const unsigned int BIT_FLAG_0 = (1 << 0); // 0000 0000 0000 0001
static const unsigned int BIT_FLAG_1 = (1 << 1); // 0000 0000 0000 0010
/*
static const unsigned int BIT_FLAG_2 = (1 << 2); // 0000 0000 0000 0100
static const unsigned int BIT_FLAG_3 = (1 << 3); // 0000 0000 0000 1000
static const unsigned int BIT_FLAG_4 = (1 << 4); // 0000 0000 0001 0000
static const unsigned int BIT_FLAG_5 = (1 << 5); // 0000 0000 0010 0000
static const unsigned int BIT_FLAG_6 = (1 << 6); // 0000 0000 0100 0000
static const unsigned int BIT_FLAG_7 = (1 << 7); // 0000 0000 1000 0000
*/

// error fN
static const unsigned int MASK_ERROR = BIT_FLAG_0;
// servo on
static const unsigned int MASK_SERVOON = BIT_FLAG_1;

At_Vsido_Connect_Library::At_Vsido_Connect_Library() {}

//  角度情報の統合
short At_Vsido_Connect_Library::uniAngle(unsigned char lower,
                                         unsigned char upper) {
  UNIWORD uniWord; // データ変換用共用体
  uniWord.aucData[0] = upper;
  uniWord.aucData[1] = (lower & 0x80) | (lower >> 1);
  uniWord.sData = (uniWord.sData & 0x8000) | (uniWord.sData >> 1);

  return uniWord.sData;
}
//  角度情報の分割
void At_Vsido_Connect_Library::divAngle(short data, unsigned char *lower,
                                        unsigned char *upper) {
  UNIWORD uniWord; // データ変換用共用体
  uniWord.sData = data;
  uniWord.sData = (uniWord.sData << 1);
  *upper = uniWord.aucData[0];
  *lower = (uniWord.aucData[1] << 1);
}

void At_Vsido_Connect_Library::reset_read1byte() {
  pc_cnt = 0;
  pc_ln = 0;
  pc_op = '_';

  r_ln = 0; //返信データなし
}

//一文字ずつ読み込み
bool At_Vsido_Connect_Library::read1byte(unsigned char ch) {
  pc_rstr[pc_cnt] = ch;
  //受信パケットが長すぎる場合
  if (pc_cnt > MAXPACKETLEN) {
    reset_read1byte();
    return false;
  }
  //最初の文字が0xFF（スタートフラグ）ではない場合
  if (ch != 0xFF && pc_cnt == 0)
    return false;

  if (ch == 0xFF) { //スタートフラグ
    reset_read1byte();
    pc_rstr[pc_cnt] = ch;
  } else if (pc_cnt == 1) { //オペランド
    if (ch != 'o' //念のため、知っているオペランド以外は通さない
        && ch != 't' && ch != 'v' && ch != 'V' && ch != 's' && ch != 'S' &&
        ch != '1' && ch != '2' && ch != '3' && ch != '4' &&
        ch != '!' && ch != 'd') { //知らないオペランドの場合は受信をリセット
      reset_read1byte();

      return false;
    }
    pc_op = ch;
  } else if (pc_cnt == 2) { //命令長
    pc_ln = ch;
    if (pc_ln < 4 ||
        pc_ln >= MAXPACKETLEN) { //パケット長が長すぎる場合は受信をリセット
      reset_read1byte();
      return false;
    }
  } else if (pc_cnt == pc_ln - 1 && pc_ln >= 4) { //チェックサム

    unsigned char sum = 0;
    for (int j = 0; j < pc_ln; j++) {
      sum = sum ^ pc_rstr[j];
    }
    //チェックサムが合わない場合は受信をリセット
    if (sum != 0) {
      reset_read1byte();
      return false;
    }

    //受信がすべてうまくいった場合

    //解釈処理
    //データが正しかったらtrueを返す
    bool ret = unpack();
    return ret;
  }
  pc_cnt++;
  return false;
}

bool At_Vsido_Connect_Library::unpack_d()
{
      //データ構造がおかしければfalse
    //id1 dad1 dln1 id2 dad2 dln2 …etc

   if ((pc_ln - 4) % 3 != 0) {
      reset_read1byte();
      return false;
    }

    int servo_num = (pc_ln - 4) / 3;
    int read_offset=3;//header op lenの3つを読み飛ばす

    unsigned char r_op = pc_op;
    unsigned char r_data[128];
    int r_cnt = 0;
    

    //各id毎の処理
    for (int i = 0; i < servo_num; i++) {
      //値の復元
      int servo_id = pc_rstr[read_offset + i * 3+0];
      int dad=pc_rstr[read_offset + i * 3 +1];
      int dln=pc_rstr[read_offset + i * 3 +2];      

      if(dad!=0&&dad!=5){
        //現状、特定のdad以外はエラーとする        
        reset_read1byte();
        return false;
      }

      if(dad==0&&dln==3){
      //返信データ　statusbyte＋角度
      unsigned char lower, upper;
      divAngle(servo_present_angles[servo_id], &lower, &upper);
      r_data[r_cnt++] = servo_id;
      r_data[r_cnt++] = servo_status[servo_id];
      r_data[r_cnt++] = lower;
      r_data[r_cnt++] = upper;
      }
      else if(dad==5&&dln==2){
      //返信データ トルク
      unsigned char lower, upper;
      divAngle(servo_present_torques[servo_id], &lower, &upper);
      r_data[r_cnt++] = servo_id;
      r_data[r_cnt++] = lower;
      r_data[r_cnt++] = upper;
      }
      if (dad == 19 && dln == 2)
      {
        //返信データ　statusbyte＋角度
        unsigned char lower, upper;
        divAngle(servo_present_angles[servo_id], &lower, &upper);
        r_data[r_cnt++] = servo_id;
        r_data[r_cnt++] = lower;
        r_data[r_cnt++] = upper;
      }

      else {
        //現時点で、特定のdad,dlnの組み合わせ以外はエラーとする
        reset_read1byte();
        return false;
      }
    }

    //返信処理
    gen_vsidocmd(r_op, r_data, r_cnt);
    return true;
}


bool At_Vsido_Connect_Library::unpack() {

  r_ln = 0;

  //オペランドが'!'の処理
  if (pc_op == '!') {
    gen_vsidocmd('!', NULL, 0);
    return true;
  }
  
  if (pc_op == 'd') {
    return unpack_d();
  }
  
  //オペランドが'o'なら目標角度登録
  if (pc_op == 'o') {
    //データ構造がおかしければfalse
    // cycle id1 angle1_l angle1_h id2 angle2_l angle2_h…etc
    if ((pc_ln - 4 - 1) % 3 != 0) {
      reset_read1byte();
      return false;
    }

    int servo_num = (pc_ln - 4 - 1) / 3;
    int read_offset=4;//header op len　cycの4つを読み飛ばす

    unsigned char r_op = pc_op;
    unsigned char r_data[128];
    int r_cnt = 0;

    //各id毎の処理
    for (int i = 0; i < servo_num; i++) {
      //値の復元
      int servo_id = pc_rstr[read_offset + i * 3+0];
      int servo_angle =
          uniAngle(pc_rstr[read_offset + i * 3 + 1], pc_rstr[read_offset + i * 3 + 2]);
      //値更新
      servo_angles[servo_id] = servo_angle;

      //返信データ
      unsigned char lower, upper;
      divAngle(servo_present_angles[servo_id], &lower, &upper);
      r_data[r_cnt++] = servo_id;
      r_data[r_cnt++] = servo_status[servo_id];
      r_data[r_cnt++] = lower;
      r_data[r_cnt++] = upper;
    }

    //返信処理
    gen_vsidocmd(r_op, r_data, r_cnt);
    return true;
  }
  //オペランドが't'なら目標トルク登録
  if (pc_op == 't') {
    //データ構造がおかしければfalse
    // cycle id1 torque1_l torque1_h id2 torque2_l torque2_h…etc
    if ((pc_ln - 4 - 1) % 3 != 0) {
      reset_read1byte();
      return false;
    }

    int servo_num = (pc_ln - 4 - 1) / 3;

    int read_offset=4;//header op len　cycの4つを読み飛ばす
    unsigned char r_op = pc_op;
    unsigned char r_data[128];
    int r_cnt = 0;

    //各id毎の処理
    for (int i = 0; i < servo_num; i++) {
      //値の復元
      int servo_id = pc_rstr[read_offset + i * 3];
      int servo_torque =
          uniAngle(pc_rstr[read_offset + i * 3 + 1], pc_rstr[read_offset + i * 3 + 2]);
      servo_torques[servo_id] = servo_torque;

      //返信データ
      unsigned char lower, upper;
      divAngle(servo_present_torques[servo_id], &lower, &upper);
      r_data[r_cnt++] = servo_id;
      r_data[r_cnt++] = lower;
      r_data[r_cnt++] = upper;
    }

    //返信処理
    gen_vsidocmd(r_op, r_data, r_cnt);
    return true;
  }

  //規定のない場合はackを返す
  gen_vsidocmd('!', NULL, 0);
  return true;
}

unsigned char At_Vsido_Connect_Library::calcsum(unsigned char *packet,
                                                int packet_ln) {
  unsigned char sum = 0;
  for (int i = 0; i < packet_ln; i++) {
    sum = sum ^ packet[i];
  }
  return sum;
}

void At_Vsido_Connect_Library::gen_vsidocmd(unsigned char r_op,
                                            unsigned char *data, int data_ln) {
  int cnt = 0;

  r_str[cnt++] = 0xff;
  r_str[cnt++] = r_op;
  r_str[cnt++] = 4 + data_ln;

  for (int i = 0; i < data_ln; i++) {
    r_str[cnt++] = data[i];
  }

  unsigned char sum = calcsum(r_str, cnt);
  r_str[cnt++] = sum;
  r_ln = cnt;
}

void At_Vsido_Connect_Library::setStatus_ServoOn(int id, bool flag) {
  // SON(6 bit)：サーボオン・オフ​
  // 0:サーボオフ​,1:サーボオン​
  if (flag) {
    //フラグを立てる
    servo_status[id] |= MASK_SERVOON;
  } else {
    //フラグを消す
    servo_status[id] &= ~MASK_SERVOON;
  }
}

void At_Vsido_Connect_Library::setStatus_Error(int id, bool flag) {
  // ERR(7bit)：軸エラー​ 0:正常1:異常
  if (flag) {
    //フラグを立てる
    servo_status[id] |= MASK_ERROR;
  } else {
    //フラグを消す
    servo_status[id] &= ~MASK_ERROR;
  }
}

bool At_Vsido_Connect_Library::getStatus_ServoOn(int id) {
  if (servo_status[id] & MASK_SERVOON)
    return true;
  return false;
}

bool At_Vsido_Connect_Library::getStatus_Error(int id) {
  if (servo_status[id] & MASK_ERROR)
    return true;
  return false;
}

bool At_Vsido_Connect_Library::check_servo_id(int id)
{
  
}