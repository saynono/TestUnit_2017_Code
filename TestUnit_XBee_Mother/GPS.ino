/*
#include <Adafruit_GPS.h>

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  false

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

SoftwareSerial SerialGPS(8,9);

Adafruit_GPS GPS(&SerialGPS);

void setupGPS(){
   // 9600 NMEA is the default baud rate for MTK - some use 4800
  GPS.begin(9600);
  
  // You can adjust which sentences to have the module emit, below
  // Default is RMC + GGA
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Default is 1 Hz update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ); 
  
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_5HZ); 
  
    // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

    // the nice thing about this code is you can have a timer0 interrupt go off
  // every 1 millisecond, and read data from the GPS for you. that makes the
  // loop code a heck of a lot easier!
  useInterrupt(true);
  
//  delay(1000);
  // Ask for firmware version
//  SerialGPS.println(PMTK_Q_RELEASE);

//  GPS.sendCommand(PMTK_API_SET_FIX_CTL_1HZ); 
  
}


uint32_t timer = millis();
void loopGPS() {
  // in case you are not using the interrupt above, you'll
  // need to 'hand query' the GPS, not suggested :(
//  if (! usingInterrupt) {
//  SerialGPS.listen();

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millisSynced())  timer = millisSynced();

  // approximately every 2 seconds or so, print out the current stats
  if (millisSynced() - timer > 19500) { 
     timer = millisSynced(); // reset the timer
     SerialGPS.listen();
     Serial.println("Listening to GPS");
//     GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); 
//     while( !updateGPSLoop() ){
//       delay(10);
//     }

//     digitalWrite(statusLed, HIGH);
     while( millisSynced() - timer < 500 ){
       updateGPSLoop();
     }
//     digitalWrite(statusLed, LOW);
     Serial.println("Not listening to GPS");
      updateGPSData();
//     GPS.sendCommand(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); 
  }
}
 
bool updateGPSLoop(){   
    while( !usingInterrupt && SerialGPS.available() ){
      // read data from the GPS in the 'main loop'
      char c = GPS.read();
      // if you want to debug, this is a good time to do it!
      if (GPSECHO)
        if (c) Serial.print(c);
    }
//    Serial.println(F("loop"));

    // if a sentence is received, we can check the checksum, parse it...
    if (GPS.newNMEAreceived()) {
      // a tricky thing here is if we print the NMEA sentence, or data
      // we end up not listening and catching other sentences! 
      // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
      //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
      if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
        return false;  // we can fail to parse a sentence in which case we should just wait for another
        
//      updateGPSData();        
      return true;
    }
    return false;
  
}

void updateGPSData(){
//     float d = calcDistance(51.5612, -0.0630,51.5613, -0.063);
//    Serial.println("Distance: ");
//    Serial.println( d );

    
    Serial.print("\nTime: ");
    Serial.println(GPS.ttime);
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
//    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print("Quality: "); Serial.println((int)GPS.fixquality); 
    if (GPS.fix) {
      Serial.print("Location: ");
//      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
//      Serial.print(", "); 
//      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
//      Serial.print("Location (in degrees, works with Google Maps): ");
      Serial.print(GPS.latitudeDegrees, 4);
      Serial.print(", "); 
      Serial.println(GPS.longitudeDegrees, 4);
      
//      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
//      Serial.print("Angle: "); Serial.println(GPS.angle);
//      Serial.print("Altitude: "); Serial.println(GPS.altitude);
//      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }  
}


// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER2_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) Serial.print(c);  
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR2A = 0xAF;
    TIMSK2 |= _BV(OCIE2A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK2 &= ~_BV(OCIE2A);
    usingInterrupt = false;
  }
}

float toRadians( float x){
  return ((x * 71.0f) / 4068.0f);
}

float calcDistance( float lat1, float lon1, float lat2, float lon2 ){
  float R = 6371000; // km, now meters
  float dLat = toRadians(lat2-lat1);  
  float dLon = toRadians(lon2-lon1);
  float lat1r = toRadians(lat1);
  float lat2r = toRadians(lat2);
  
  float a = sin(dLat/2.0) * sin(dLat/2.0f) + sin(dLon/2.0f) * sin(dLon/2.0f) * cos(lat1r) * cos(lat2r); 
  float c = 2 * atan2( sqrt(a), sqrt(1-a)); 
  return R * c;
}

*/
