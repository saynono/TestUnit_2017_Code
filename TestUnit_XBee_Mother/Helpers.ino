

/*-----------------------------------------------------------------------*
* returns received signal strength value for the last RF data packet.   *
*-----------------------------------------------------------------------*/
uint8_t getRSSI() {
    sendAtCommand('D','B');
}

unsigned long millisSynced(){
  return millis() + heartbeatOffset;
}

void flashLed(int pin, int times, int wait) {

  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(wait);
    digitalWrite(pin, LOW);

    if (i + 1 < times) {
      delay(wait);
    }
  }
}

void showHeartbeat(){
//  flashLed(statusLed,2,100);
}

void printPayload( uint8_t* pl, int len ){
  Serial.print("PAYLOAD: ");
  for( int i=0;i<len;i++ ){
    Serial.print(pl[i],HEX);
    Serial.print(",");
  }
  Serial.println("");
}

void printNodeDef( uint8_t* def ){
  NodeDefinition* nodeDef = (NodeDefinition*) def;
  
  Serial.println(" NODE ============ " );

  Serial.print("NAME : " );
  Serial.println(nodeDef->name);
  Serial.print("ADDRESS ");
  printHex( Serial, selfDef.address.getMsb() );
  Serial.print(" ");
  printHex( Serial, selfDef.address.getLsb() );    
  Serial.println("");
 
  Serial.println(" ================== | " );

}

