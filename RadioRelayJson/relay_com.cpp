
#include "relay_com.h"

SoftwareSerial softCom(SOFT_RX_PIN, SOFT_TX_PIN); 

byte softComState;
char softComBuff[SCOM_BUFF_LEN];
short int softComBuffIndx =0;
short int i;
boolean softComFlag;

boolean SoftComFlag(void){
  return softComFlag;
}

void InitSoftCom (void){
  pinMode(SOFT_RX_PIN, INPUT);
  pinMode(SOFT_TX_PIN, OUTPUT);
  softCom.begin(9600);
  
  //softComState = 0;
  //softComBuffIndx =0;
  //for (i=0;i<SCOM_BUFF_LEN;i++){
  //  softComBuff[i]= '0'+i;
  //}
  softCom.listen();
  //softComFlag = false;
}

void SendSoftcomRelayMsg(char unit,char relay){
    char soft_buff[16]= "";
    uint8_t i= 0;
    soft_buff[i++] = '<';
    soft_buff[i++] = '#';
    soft_buff[i++] = 'R';
    soft_buff[i++] = unit;
    soft_buff[i++] = relay;
    soft_buff[i++] = '=';
    soft_buff[i++] = 'T';
    soft_buff[i++] = '>';
    softCom.println(soft_buff);
    Serial.println(soft_buff);


     
}
void SendSoftCom(const char *b){
  Serial.println(b);
  softCom.print('<');
  softCom.print(b);
  softCom.println(">");
 
}

 void DebugSoftCom(void){
 
 if (softCom.available()>0){
     char c;
     c = softCom.read();
     Serial.print(c);
  }
 }
