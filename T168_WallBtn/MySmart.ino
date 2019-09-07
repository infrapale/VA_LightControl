#define ADDR_LEN  MSG_ADDR_LEN+1
#define DATA_LEN  24

//#define IN_MSG_LEN  16


char c;

char MsgDataBuff[DATA_LEN]; 
char MsgAddrBuff[ADDR_LEN]; 
char MsgType;

short int MsgInBuffIndx =0;


void InitMySmart (void){
  MsgDataBuff[0]=0;
  MsgAddrBuff[0]=0;
  
  
  SerialFlag = false;
}




void GetMsg(void){
    MsgType = Smart.GetMsgType();
    Smart.CopyAddr(MsgAddrBuff);
    Smart.CopyData(MsgDataBuff);
    Serial.println(MsgType);
    Serial.println(MsgAddrBuff);
    Serial.println(MsgDataBuff);
    Smart.clear();
    msgReady = false;
    switch(MsgType){
       case SM_ADDR_MSG:
           //HandleAddrMsg();
           break;   
       case SM_REPLY_ADDR:
           break;   
       case SM_BROADCAST:
           //HandleBroadcastMsg();
           break;   
       case SM_QUICK:
           HandleQuickMsg();
           break; 
    }
}

/*
void HandleAddrMsg(void){
  if (!strcmp(Me.addr, MsgAddrBuff)){
      if (!strcmp(MsgDataBuff,"?S")) PingReplyShort();
      if (!strcmp(MsgDataBuff,"?L")) PingReplyLong();
  }
}

void HandleBroadcastMsg(void){
  byte i=0;
  byte j;
  byte k;
  byte action_indx = 0;
  boolean done = false;

  while (! done){
      if (!strcmp(my_bcast[i].addr, MsgAddrBuff)){
         done = true;
         action_indx = my_bcast[i].action;
      }
      if (++i > MAX_BCAST) done = true;
  }
  switch(action_indx){
      case 10:
          strcpy(MsgAddrBuff, my_bcast[0].addr);
          strcpy(MsgDataBuff,"=");
          strcat(MsgDataBuff,"22.3C");
          Smart.Send(SM_REPLY_ADDR,MsgAddrBuff, MsgDataBuff);
          break;
      case 20:
          strcpy(MsgAddrBuff, my_bcast[1].addr);
          strcpy(MsgDataBuff,"=");
          strcat(MsgDataBuff,"80%");
          Smart.Send(SM_REPLY_ADDR,MsgAddrBuff, MsgDataBuff);

          break;
      case 50:
          i= MsgDataBuff[0]; 
          j=MsgDataBuff[1];
          k=MsgDataBuff[2];

          if (i>'0' && i<'9' && j== '=' && (k=='0' || k=='1' ) ){
            Serial.print(my_bcast[3].addr);
            Serial.print(" Relay ");
            Serial.print(char(i));
            Serial.print(" ");
            Serial.println(k=='1');
            //SetRelay(i-'0',k=='1');
          } 
          else {
            Serial.print("Incorrect Relay Message ");
            Serial.print(MsgAddrBuff);
            Serial.println(MsgDataBuff);
          }
          break;
      case 60: break;
      case 70: break;
  }
}
*/
void HandleQuickMsg(void){   
    switch( MsgDataBuff[0]){
       case 'K':
          Serial.print("Key ");
          Serial.print(MsgDataBuff[1]);
          Serial.println(" pressed");
          break; 
    }
}

void AppendChar(char *toArray,char newChar){
    byte i=0;
    while (toArray[i]!=0) i++;
    toArray[i] = newChar;
    toArray[i+1] = 0;
}
void PingReplyShort(void){
   strcpy(MsgDataBuff, Me.name);  strcat(MsgDataBuff,",");
   strcat(MsgDataBuff, Me.system_sketch); 
   strcat(MsgDataBuff,",");
   AppendChar(MsgDataBuff,Me.index);
   Smart.Send(SM_REPLY_ADDR,MsgAddrBuff, MsgDataBuff);
}

void PingReplyLong(void){
   char s[8];
   strcpy(MsgDataBuff, Me.name);  strcat(MsgDataBuff,",");
   strcat(MsgDataBuff, Me.system_sketch);  strcat(MsgDataBuff,",");
   strcat(MsgDataBuff, Me.system_sketch); strcat(MsgDataBuff,",");
   AppendChar(MsgDataBuff,Me.index);strcat(MsgDataBuff,",");
   strcat(MsgDataBuff, Me.hardware_sketch); strcat(MsgDataBuff,",");
   strcat(MsgDataBuff, Me.software_sketch); strcat(MsgDataBuff,",");
   strcat(MsgDataBuff, Me.version); strcat(MsgDataBuff,":");
   itoa(MyTime.year,s,10);
   strcat(MsgDataBuff, s); strcat(MsgDataBuff,",");
   itoa(MyTime.month,s,10);
   strcat(MsgDataBuff, s); strcat(MsgDataBuff,",");
   itoa(MyTime.year,s,10);
   strcat(MsgDataBuff, s); strcat(MsgDataBuff,",");
   itoa(MyTime.day,s,10);
   strcat(MsgDataBuff, s); strcat(MsgDataBuff,"-");
   itoa(MyTime.hour,s,10);
   strcat(MsgDataBuff, s); strcat(MsgDataBuff,":");
   itoa(MyTime.minute,s,10);
   strcat(MsgDataBuff, s); strcat(MsgDataBuff,":");
   itoa(MyTime.second,s,10);
   strcat(MsgDataBuff, s); 
  
   Smart.Send(SM_REPLY_ADDR,MsgAddrBuff, MsgDataBuff);
}

void InterpretMsg(void){
    byte i;
    byte j;
    boolean done;
    //String s;
    char msgType; 
    boolean this_is_for_me = false;

    Smart.CopyData(MsgDataBuff);
    msgType = Smart.GetMsgType();
    switch(msgType){
       case SM_ADDR_MSG:
           Smart.CopyAddr(MsgAddrBuff);
           this_is_for_me = !strcmp(MsgAddrBuff, "MH2-1"); 
           
           break;   
       case SM_REPLY_ADDR:
           Smart.CopyAddr(MsgAddrBuff);
           break;   
       case SM_BROADCAST:
           Smart.CopyAddr(MsgAddrBuff);
           break;   
       case SM_QUICK:
           MsgAddrBuff[0]=0;
           break;   
    }

}
    
/*
    Serial.println(MsgDataBuff);
    switch( MsgInBuff.charAt(0)){ 
    case SM_ADDR_MSG:
       done =false; 
       if (MsgInBuff.substring(ADDR_MSG_START,ADDR_MSG_START+ADDR_MSG_LEN) == Me.name &&
          MsgInBuff.charAt(ADDR_MSG_DATA) == '.' &&
          MsgInBuff.charAt(ADDR_MSG_DATA+1) == Me.index){
          
          //<@Terminal.1PINGL>  <!Terminal.1 T129 T114 T129 16v01>
          if (!done && MsgInBuff.substring(ADDR_MSG_DATA+2,ADDR_MSG_DATA+3+ADDR_CMD_LEN) == PING_SHORT){
             PingReplyShort();
             done = true;    
           }
          if (!done && MsgInBuff.substring(ADDR_MSG_DATA+2,ADDR_MSG_DATA+3+ADDR_CMD_LEN) == PING_LONG){
             PingReplyLong();
             done = true;    
           }
           
        }
        //for ( i=1; i < 5; i++) if( MsgInBuff.charAt(i) !=uAddr[i-1]) b=false;
        break; 
    case SM_BROADCAST:
        done = false;
        // <#PINGS>
        if (!done && MsgInBuff.substring(BC_MSG_START,BC_MSG_START+BC_MSG_LEN) == PING_SHORT){
           delay(random(PING_DELAY_MIN, PING_DELAY_MAX));
           PingReplyShort();
           done = true;
        }
        // <#KEYPR=x>
        if (!done && MsgInBuff.substring(BC_MSG_START,BC_MSG_START+BC_MSG_LEN) == KEY_PRESSED &&
           MsgInBuff.charAt(BC_MSG_DATA) == '=') {
             
           //SmartKeys = MsgInBuff.substring(BC_MSG_DATA+1);  
           done = true;
        }
        // <#SHOW1=text>
        if (!done && MsgInBuff.substring(BC_MSG_START,BC_MSG_START+BC_MSG_LEN) == SHOW_ROW_1 &&
           MsgInBuff.charAt(BC_MSG_DATA) == '=') {
           done = true;
        }
        // <#SHOW2=text>
        if (!done && MsgInBuff.substring(BC_MSG_START,BC_MSG_START+BC_MSG_LEN) == SHOW_ROW_2 &&
           MsgInBuff.charAt(BC_MSG_DATA) == '=') {
           done = true;
        }
       
       // <#TIME_=13:03:00> 
       if (!done && MsgInBuff.substring(BC_MSG_START,BC_MSG_START+BC_MSG_LEN) == TIME_BROADCAST &&
          MsgInBuff.charAt(BC_MSG_DATA) == '=') {
          s = MsgInBuff.substring(BC_MSG_DATA+1,BC_MSG_DATA+3);
          MyTime.hour = s.toInt();
          s = MsgInBuff.substring(BC_MSG_DATA+4,BC_MSG_DATA+6);
          MyTime.minute = s.toInt();
          s = MsgInBuff.substring(BC_MSG_DATA+7,BC_MSG_DATA+9);
          MyTime.second = s.toInt();
          //PingReplyLong();
          done = true;
      }

      #ifdef I_DO_RELAYS
      if (!done && MsgInBuff.charAt(BC_MSG_START)==SM_RELAY_CMND){
         i = 0; done = false;
         while (!done){
            s = menu_table[i].cmnd;
            //Serial.println(s); Serial.println(i);
            if (MsgInBuff.substring(BC_MSG_START,BC_MSG_START+BC_MSG_LEN) == s ){
               done = true;
            } 
            else {
              if( ++i >= MAX_MENU_INDX) done = true;
            }
         }

         if ((MsgInBuff.charAt(BC_MSG_DATA)==SM_ASSIGN) ||
             (menu_table[i].ru == Me.index ) ||
             (MsgInBuff.charAt(BC_MSG_DATA+1)==SM_ON) || 
             (MsgInBuff.charAt(BC_MSG_DATA+1)==SM_OFF) ||
             (MsgInBuff.charAt(BC_MSG_DATA+1)==SM_TOGGLE)){
                //RelayControl(menu_table[i].ri,MsgInBuff.charAt(BC_MSG_DATA+1)); 
                //Serial.print("Relay "); Serial.print(menu_table[i].ri); Serial.print("-");
                //Serial.println(MsgInBuff.charAt(BC_MSG_DATA+1)); 

         }
         
         
      }
      #endif
   }
}
*/
