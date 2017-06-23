
#define PIN_BAY_1     38
#define PIN_BAY_2     39
#define PIN_BAY_3     40
#define PIN_BAY_4     41
#define PIN_BAY_5     42
#define PIN_BAY_6     43
#define PIN_BAY_7     44
#define PIN_BAY_8     45
#define PIN_BAY_9     46
#define PIN_BAY_10    47

#define bay_cnt       10

int BAYS[] = {PIN_BAY_1,PIN_BAY_2,PIN_BAY_3,PIN_BAY_4,PIN_BAY_5,PIN_BAY_6,PIN_BAY_7,PIN_BAY_8,PIN_BAY_9,PIN_BAY_10};


float cnt = 0;

void setup() {
  // put your setup code here, to run once:
  for( int i=0;i<bay_cnt;i++ ){
    pinMode( BAYS[i], OUTPUT );
    digitalWrite(BAYS[i], LOW );
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  float multi = .02 * millis()/1000.0;

  for(int i=0;i<bay_cnt;i++){
    int value = (int)(127 + 127 * sin((i+10)*multi));
//    digitalWrite(BAYS[i], value > 127 ? HIGH : LOW );
    
  }
  
  
  cnt ++;
  
}
