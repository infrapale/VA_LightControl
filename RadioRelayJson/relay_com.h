#ifndef __RELAY_COM_H__
#define __RELAY_COM_H__
#include "Arduino.h"
#include <SoftwareSerial.h>

#define SOFT_RX_PIN 8
#define SOFT_TX_PIN 7
#define SCOM_BUFF_LEN 40


boolean SoftComFlag(void);
void InitSoftCom (void);
void SendSoftcomRelayMsg(char unit,char relay);
void SendSoftCom(const char *b);
void DebugSoftCom(void);
#endif
