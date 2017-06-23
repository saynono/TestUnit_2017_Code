
uint8_t elWirePorts[] = {2,3,4,5};

int elWireStates = 0;
int elWireStatesPrev = 0;

void setupELWire(){
  for( int i=0;i<4;i++ ){
    pinMode(elWirePorts[i],OUTPUT);
  }  
}


void loopELWire(){
  
  int regWires = 3;
  int speed = 2000 + sin( millisSynced()/20000.0f ) * 1500;
  
  int id = (millisSynced()/speed)%regWires;

  elWireStatesPrev = elWireStates;
  for( int i=0;i<regWires;i++ ){
    setELWireState( i , id == i );
  }
  
  if( elWireStatesPrev != elWireStates ){
    updateELWires();
  }
  
  
}

void setELWireState(int pos, bool state ){
  
  
  // check: http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
  
  // delete value
  elWireStates &= ~(1 << pos);
  // add value
  elWireStates |= (state << pos);
  
}

void elSwitchAll( bool on ){
  if( on ) elWireStates = 0xff;
  else elWireStates = 0x0;
//    Serial.print("ELWIRE: ");
//  Serial.println( on );
//
//  for( int i=0;i<4;i++ ){
//    digitalWrite( elWirePorts[i], HIGH );
//  }
}

void updateELWires(){
  for( int i=0;i<4;i++ ){
    digitalWrite( elWirePorts[i], (elWireStates>>i) & 1 );
  }  
}
