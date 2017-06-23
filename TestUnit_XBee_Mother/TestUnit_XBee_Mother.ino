#include <StackList.h>



#include <XBee.h>
#include <Printers.h>
#include <SoftwareSerial.h>

#define HEARTBEAT_INTERVAL 10000


struct NodeDefinition{
  uint8_t id = 0;
  char name[20];
  uint8_t nameLength = 0;
  XBeeAddress64 address;
  float lat = 0;
  float lon = 0;
  unsigned long time = 0;
};


// All:
XBeeAddress64 addr64_all   = XBeeAddress64(0x00000000, 0x0000ffff);

// Mothership:
XBeeAddress64 addr64_Mothership   = XBeeAddress64(0x0013a200, 0x40A0496C);

// Aimless Spaceship:
//XBeeAddress64 addr64_Aimless      = XBeeAddress64(0x0013a200, 0x405297E5);

// Friendly Spaceship:
XBeeAddress64 addr64_Friendly     = XBeeAddress64(0x0013a200, 0x40A04B97);

// Passenger 3:
XBeeAddress64 addr64_Pass3        = XBeeAddress64(0x0013a200, 0x40A04AC2);

// Lost Spaceship
XBeeAddress64 addr64_xx        = XBeeAddress64(0x0013a200, 0x40A04BD3);



//int statusLed = 10;
int errorLed = 13;
int buttonPin = 9;

int PIN_RELAY = 4;

uint32_t heartbeatTimer = millis();
int32_t heartbeatOffset = 0;

uint32_t lastTimeOn = millis();
uint32_t maxTimeOn = 60000;

//uint32_t lastHeartbeat = 0;
  
bool buttonStatus = false;
bool remoteButtonTrigger = false;
float signalStrength = -1;
bool isMother = false;

int buttonCounter = 0;
uint8_t sensorValues[] = {1,2,3,4,5}; 
uint8_t sensorValuesRemote[] = {0,0,0,0,0}; 
int tmpTriggerCounter = 0;

int sensorColors[][2] = {{1,8},{0,9},{4,5},{3,6},{2,7}};
bool BAY_STATES[] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};

//pink, blue, red, orange, green
//[1,8],[0,9],[4,5],[3,6],[2,7]



NodeDefinition selfDef;

void setup() {
  
//  pinMode(statusLed, OUTPUT);
  pinMode(errorLed, OUTPUT);

  pinMode(buttonPin, INPUT);
  

  Serial.begin(115200);

//  setupGPS();
  setupXBee();
  setupELWire();
  setupLights();

  if( isMother ){
      sendHeartbeat();    
//      showHeartbeat();
  }
}

void processRemoteSensorValues(){
      Serial.println(F("-------------------"));
      Serial.println(F("SENSOR VALUES REMOTE:"));
  for( int i=0;i<sizeof(sensorValuesRemote);i++ ){
    uint8_t val = sensorValuesRemote[i];
    if( val > 30 && val < 150 ){
      BAY_STATES[ sensorColors[i][0]] = LOW;
      BAY_STATES[ sensorColors[i][1]] = LOW;
    }else{
      BAY_STATES[ sensorColors[i][0]] = HIGH;
      BAY_STATES[ sensorColors[i][1]] = HIGH;
    }
    Serial.print(F("SENSOR VALUE:"));
    Serial.println( val );        
  }
      Serial.println(F("-------------------"));
}

void loopButton(){
  bool btn = digitalRead( buttonPin );
  if( buttonStatus != btn ){
    buttonStatus = btn;
//    Serial.print("Sending Button State...");
//    Serial.print( buttonStatus );
//    Serial.print("   ");
//    Serial.println(tmpTriggerCounter);
    sendButtonStatus();
    if( buttonStatus ){
      tmpTriggerCounter++;  
      sensorValues[2] = sensorValues[2]+1;
      delay(10);
      sendSensorValues();
    }
  }  
}


void loop() {   
  // break down 10-bit reading into two bytes and place in payload

//  loopButton();

  bool oldState = remoteButtonTrigger;

//  sensorValues[2] = sensorValues[2]+1;
//  sendSensorValues();
  
  loopXBee();
  loopLights();

  if( millis() - lastTimeOn > maxTimeOn ){
    remoteButtonTrigger = false;
    lastTimeOn = millis();
  }
  
  bool state = remoteButtonTrigger;

  if(oldState != state){
//    digitalWrite( statusLed , state );
    digitalWrite( PIN_RELAY, state );  
    Serial.println("State Changed!");
    if( state ) {
      lastTimeOn = millis();
    }
  }

  if( isMother ){
    if( millisSynced()-heartbeatTimer > HEARTBEAT_INTERVAL ){
      heartbeatTimer = millisSynced();
      sendHeartbeat();
    }
  }
  delay(5);
}




