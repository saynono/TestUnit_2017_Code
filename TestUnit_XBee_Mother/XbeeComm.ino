
#define MSG_HEADER_INIT           'I'
#define MSG_HEADER_HI             'H'
#define MSG_HEADER_BLINK          'B'
#define MSG_HEADER_FLASH_ON       'O'
#define MSG_HEADER_FLASH_OFF      'X'
#define MSG_HEADER_HEARTBEAT      'V'
#define MSG_HEADER_PING           'P'
#define MSG_HEADER_SENSOR_VALUES  'S'



// create the XBee object
XBeeWithCallbacks xbee;


//ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

// Define NewSoftSerial TX/RX pins
uint8_t ssRX = 10;
uint8_t ssTX = 12;
//uint8_t ssRX = 2;
//uint8_t ssTX = 3;

SoftwareSerial SerialXBee(ssRX, ssTX);



void setupXBee(){
  SerialXBee.begin(57600);
  xbee.setSerial(SerialXBee);
  // These are called when an actual packet received
  
  
  xbee.onZBRxResponse(zbReceive);
  xbee.onRx16Response(receive16);
  xbee.onRx64Response(receive64);
  xbee.onAtCommandResponse(receiveAT);

  
    // Enable this to print the raw bytes for _all_ responses before they
  // are handled
  //xbee.onResponse(printRawResponseCb, (uintptr_t)(Print*)&Serial2);
//
//  // Set AO=0 to make sure we get ZBRxResponses, not
//  // ZBExplicitRxResponses (only supported on series2). This probably
//  // isn't needed, but nicely shows how to use sendAndWait().
//  delay(1000);
//  uint8_t value = 0;
//  AtCommandRequest req((uint8_t*)"AO", &value, sizeof(value));
//  req.setFrameId(xbee.getNextFrameId());
//  // Send the command and wait up to 150ms for a response
//  uint8_t status = xbee.sendAndWait(req, 150);

//  delay(2000);    
  sendAtCommand('S','L',true);
  sendAtCommand('N','I',true);   
}

void loopXBee(){
  SerialXBee.listen();
  xbee.loop();
}


void zbReceive(ZBRxResponse& rx, uintptr_t) {
  
  /*
  Serial.println("// -------------------------------");  
  
  Serial.println("Incoming type: zbReceive");
//  Serial.print(" RX Address 16: ");
//  Serial.println( rx.getRemoteAddress16(), HEX );
  Serial.print(" RX Address 64: ");
  printHex( Serial, rx.getRemoteAddress64() );
  Serial.println("");
  Serial.print(" Frame Data : " );
  printHex( Serial, rx.getFrameData()+ rx.getDataOffset(), rx.getDataLength() );
  Serial.println("");  
  
  int i = rx.getDataOffset();
  int s = i + rx.getDataLength();
  Serial.print(" Data String : ");
  for( ;i<s;i++){
    Serial.print((char)rx.getFrameData()[i]);
  }
  Serial.println("");
  Serial.println("// -------------------------------");  
  
  */
  
  processZBData( rx );
}


void receive16(Rx16Response& rx, uintptr_t) {
  // Create a reply packet containing the same data
  // This directly reuses the rx data array, which is ok since the tx
  // packet is sent before any new response is received
//  Tx16Request tx;
//  tx.setAddress16(rx.getRemoteAddress16());
//  tx.setPayload(rx.getFrameData() + rx.getDataOffset(), rx.getDataLength());
  Serial.println("Incoming type: receive16");
//  flashLed(statusLed,1,100);
}

void receive64(Rx64Response& rx, uintptr_t) {
//  flashLed(statusLed,6,1000);
  Serial.println("Incoming type: receive64");
}










void sendAtCommand( uint8_t cmd1, uint8_t cmd2 ) {
  sendAtCommand( cmd1, cmd2, false);
}

void sendAtCommand( uint8_t cmd1, uint8_t cmd2, bool wait ) {

  uint8_t cmd[] = {cmd1,cmd2};
  AtCommandRequest reqAt( cmd );
  reqAt.setFrameId(xbee.getNextFrameId());  
  if( wait ){
    uint8_t status = xbee.sendAndWait(reqAt, 200);
    if( status == XBEE_WAIT_TIMEOUT ){
      Serial.println("AT COMMAND RESPONSE TIMEOUT!");
    }else{
      if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
	AtCommandResponse atResponse;
        xbee.getResponse().getAtCommandResponse(atResponse);
        processATCommand(atResponse);
      }
    }    
  }else{
    xbee.send(reqAt);
  }
}


void receiveAT(AtCommandResponse& atResponse, uintptr_t) {
  processATCommand(atResponse);
}



void processATCommand(AtCommandResponse& atResponse){
  Serial.print(">>>> AT Response [");
  Serial.print((char)atResponse.getCommand()[0]);
  Serial.print((char)atResponse.getCommand()[1]);  
  Serial.println("] <<<<");
  
  char cmd1 = (char)atResponse.getCommand()[0];
  char cmd2 = (char)atResponse.getCommand()[1];  

  if( cmd1 == 'D' && cmd2 == 'B'){
    
    Serial.print("Signal Strength Is : ");
    int val = atResponse.getValue()[0];
//    signalStrength = map( val, 0x24,0x64,0,100);
    float percent = constrain( map( val, 0x24,0x64,100,0) , 0, 100) / 100.0f;
    if( signalStrength < 0 ) signalStrength = percent;
    else signalStrength = percent*.4 + signalStrength*.6;
    Serial.print((signalStrength*100));
    Serial.print("%    ");
    Serial.println( val, HEX );

  }else if( cmd1 == 'S' && cmd2 == 'L'){
    uint32_t msb = uint32_t( 0x0013a200 );
    uint32_t lsb = uint32_t( (atResponse.getValue()[2] << 8) + (atResponse.getValue()[3] << 0)) + (uint32_t( (atResponse.getValue()[0] << 8) + (atResponse.getValue()[1] << 0)) << 16 );
    selfDef.address.setMsb( msb );
    selfDef.address.setLsb( lsb );
    
    isMother = ( selfDef.address.get() == addr64_Mothership.get() );
    if( isMother ) Serial.println("I AM YOUR MOTHER");
      Serial.println(" SETTING ADDRESS ");
      printNodeDef( (uint8_t*) &selfDef );
  }else if( cmd1 == 'N' && cmd2 == 'I'){
    selfDef.nameLength = atResponse.getValueLength();
    memset(selfDef.name,0,sizeof(selfDef.name));
    memcpy(selfDef.name, atResponse.getValue(), selfDef.nameLength);
      Serial.println(" SETTING NAME ");
      printNodeDef( (uint8_t*) &selfDef );
  }
  
}

void processZBData(ZBRxResponse& rx){
  // Create a reply packet containing the same data
  // This directly reuses the rx data array, which is ok since the tx
  // packet is sent before any new response is received
//  ZBTxRequest tx;
//  tx.setAddress64(rx.getRemoteAddress64());
//  tx.setAddress16(rx.getRemoteAddress16());
//  tx.setPayload(rx.getFrameData() + rx.getDataOffset(), rx.getDataLength());
  
//  unsigned long tNew = 0;

  uint8_t msgHeader = rx.getFrameData()[rx.getDataOffset()];
  Serial.print("INCOMING MESSAGE HEADER: ");
  Serial.println(msgHeader);
  switch(msgHeader){
    case MSG_HEADER_INIT:
      break;
    case MSG_HEADER_HI:
      break;
    case MSG_HEADER_FLASH_OFF:
      Serial.println(F("FLASH OFF"));
      remoteButtonTrigger = false;
      break;
//    case MSG_HEADER_BLINK:
//      flashLed(statusLed,1,1000);
//      break;
      
    case MSG_HEADER_PING:
    
      Serial.print(F(" PING "));
      printNodeDef( rx.getFrameData() + rx.getDataOffset()+1);
      break;
      
      
    case MSG_HEADER_FLASH_ON:
      Serial.println(F("FLASH ON"));
      remoteButtonTrigger = true;
//      {
//        unsigned long tNew = 0;
//        for(int i=0;i<sizeof(tNew);i++){
//          uint8_t val = rx.getFrameData()[rx.getDataOffset()+i+1];
//          tNew = (tNew<<8)+val;
//        }
//        Serial.print("Ping time: ");
//        Serial.println(tNew);
//      }
      break;
      
    case MSG_HEADER_HEARTBEAT:
    
      {
        unsigned long tHeartbeat = 0;
        for(int i=0;i<sizeof(tHeartbeat);i++){
          uint8_t val = rx.getFrameData()[rx.getDataOffset()+i+1];
          tHeartbeat = (tHeartbeat<<8)+val;
        }
        heartbeatOffset = tHeartbeat-millis();        
//        lastHeartbeat = millis();
        getRSSI();

      }
//      showHeartbeat();
//      sendStatus();
//      getRSSI();
      break;
    case MSG_HEADER_SENSOR_VALUES:
      for( int i=0;i<sizeof(sensorValuesRemote);i++ ){
        uint8_t val = rx.getFrameData()[rx.getDataOffset()+i+1];
        sensorValuesRemote[i] = val;
      }
      processRemoteSensorValues();
    default:
//      Serial.println(F("Unknown Command:"));
//      Serial.println(rx.getFrameData()[0],HEX);
//      flashLed(statusLed,2,100);
      break;
  }
  
  
}

void sendStatus(){
  
   uint8_t data[sizeof(selfDef)+1];
   memset(data,0,sizeof(data));
 
   uint8_t* dataRef = (uint8_t*) &selfDef;
   data[0] = (uint8_t) MSG_HEADER_PING;
   memcpy(data+1, dataRef, sizeof(data)-1);
   
   ZBTxRequest req( addr64_all, data, sizeof(data));
   xbee.send(req);
   Serial.println(F("Sent Status"));
}


void sendButtonStatus(){
  
  Serial.println(F("Start sending..."));
  
  unsigned long t = millisSynced();
  int l = sizeof(t);
  
  Serial.println(t);
  
  uint8_t payload_flash[1+l];// = { buttonStatus ? (MSG_HEADER_FLASH_ON & 0xff) : MSG_HEADER_FLASH_OFF,0};
  memset(payload_flash,0,sizeof(payload_flash));
  payload_flash[0] = buttonStatus ? (MSG_HEADER_FLASH_ON & 0xff) : MSG_HEADER_FLASH_OFF;  

  for(int i=0;i<l;i++){
    payload_flash[1+i] = ( t>>(8*(l-1-i)) ) &0xff;
  }
 
  ZBTxRequest req(addr64_Friendly,payload_flash,sizeof(payload_flash));
  if( selfDef.address.getLsb() == 0x40A04B97 ){
      req.setAddress64( addr64_Mothership );
  }
  xbee.send(req);
  
//   ZBTxRequest req(addr64_xxx,payload_flash,sizeof(payload_flash));
  Serial.println(F("Sent Button State"));
}

void sendHeartbeat() {
  
  unsigned long t = millisSynced();
  uint8_t payload_hb[1+4];// = { buttonStatus ? (MSG_HEADER_FLASH_ON & 0xff) : MSG_HEADER_FLASH_OFF,0};
  memset(payload_hb,0,sizeof(payload_hb));
  payload_hb[0] = MSG_HEADER_HEARTBEAT;

  for(int i=0;i<4;i++){
    payload_hb[1+i] = ( t>>(8*(3-i)) ) &0xff;
  }

  ZBTxRequest req(addr64_Friendly,payload_hb,sizeof(payload_hb));
  if( selfDef.address.getLsb() == 0x40A04B97 ){
      req.setAddress64( addr64_Mothership );
  }
  xbee.send(req);
  getRSSI();
}

void sendSensorValues(){  
//  Serial.println("Sending Sensor Values");
  unsigned long t = millisSynced();
  uint8_t payload_hb[1+sizeof(sensorValues)];// = { buttonStatus ? (MSG_HEADER_FLASH_ON & 0xff) : MSG_HEADER_FLASH_OFF,0};
  memset(payload_hb,0,sizeof(payload_hb));
  payload_hb[0] = MSG_HEADER_SENSOR_VALUES;

  for(int i=0;i<sizeof(sensorValues);i++){
    payload_hb[1+i] = sensorValues[i];
    
  }
  ZBTxRequest req(addr64_Friendly,payload_hb,sizeof(payload_hb));
  if( selfDef.address.getLsb() == 0x40A04B97 ){
      req.setAddress64( addr64_Mothership );
  }
  xbee.send(req);
}
