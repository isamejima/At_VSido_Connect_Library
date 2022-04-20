/*
 * at_vsido_connect.h
 * ライブラリの作り方で参考にしたURL https://stupiddog.jp/note/archives/266
 */
#include <Arduino.h>
#include "At_Vsido_Connect_Library.h"

At_Vsido_Connect_Library::At_Vsido_Connect_Library() {
}

//  角度情報の統合
short At_Vsido_Connect_Library::uniAngle(unsigned char upper,unsigned char lower)
{
    UNIWORD         uniWord;     // データ変換用共用体
    uniWord.aucData[0]= upper;
    uniWord.aucData[1]= (lower&0x80) | (lower>>1);
    uniWord.sData=(uniWord.sData&0x8000) | (uniWord.sData>>1);;
    return uniWord.sData;
}
//一文字ずつ読み込み
bool At_Vsido_Connect_Library::read1byte(unsigned char ch)
{
  pc_rstr[pc_cnt]=ch;
  //受信パケットが長すぎる場合
  if(pc_cnt>MAXPACKETLEN) {
      pc_cnt=0;
      pc_ln=0;
      pc_op='_';
      return false;
  }
  //最初の文字が0xFF（スタートフラグ）ではない場合
  if(ch!=0xFF&&pc_cnt==0)
    return false;
  
  if(ch==0xFF) { //スタートフラグ
      pc_cnt=0;
      pc_ln=0;
      pc_op='_';
      pc_rstr[pc_cnt]=ch;
  } else if(pc_cnt==1) { //オペランド
      if(ch!='o'//念のため、知っているオペランド以外は通さない
		  &&ch!='t'
		  &&ch!='v'
		  &&ch!='V'
		  &&ch!='s'
		  &&ch!='S'
		  &&ch!='1'
		  &&ch!='2'
		  &&ch!='3'
		  &&ch!='4'
		  ) {//知らないオペランドの場合は受信をリセット
          pc_cnt=0;
          pc_ln=0;
          pc_op='_';
          return false;
      }
      pc_op=ch;
  } else if(pc_cnt==2) { //命令長
      pc_ln=ch;
      if(pc_ln<4||pc_ln>=MAXPACKETLEN) {//パケット長が長すぎる場合は受信をリセット
          pc_cnt=0;
          pc_ln=0;
          pc_op='_';
          return false;
      }
  } else if(pc_cnt==pc_ln-1&&pc_ln>=4) { //チェックサム
    int servo_num=(pc_ln-4-1)/3;
    unsigned char sum=0;
    for(int j=0; j<pc_ln; j++) {
        sum = sum ^ pc_rstr[j];
    }
    //チェックサムが合わない場合は受信をリセット
    if(sum!=0) {
        pc_cnt=0;
        pc_ln=0;
        pc_op='_';
        return false;
    }
    //受信がすべてうまくいった場合

    //オペランドが'o'なら目標角度登録
    if(pc_op == 'o') {
		if((pc_ln-4-1)%3!=0) {
			pc_cnt=0;
			pc_ln=0;
			pc_op='_';
			return false;
		}
		for(int i=0; i<servo_num; i++) {
			int servo_id=pc_rstr[4+i*3];
			int servo_angle=uniAngle(pc_rstr[4+i*3+1],pc_rstr[4+i*3+2]);
			servo_angles[servo_id]=servo_angle;
		}
    }
    //オペランドが't'なら目標トルク登録
    if(pc_op == 't') {
		if((pc_ln-4-1)%3!=0) {
			pc_cnt=0;
			pc_ln=0;
			pc_op='_';
			return false;
		}
		for(int i=0; i<servo_num; i++) {
			int servo_id=pc_rstr[4+i*3];
			int servo_angle=uniAngle(pc_rstr[4+i*3+1],pc_rstr[4+i*3+2]);
			servo_torques[servo_id]=servo_angle;
		}
    }
	return true;
  }
  pc_cnt++;
  return false;
}
