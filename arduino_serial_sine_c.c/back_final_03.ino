#include "DHT.h"

 
#define DHTPIN 3        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11    // DHT 11
DHT dht(DHTPIN, DHTTYPE);


////////////////////////////////////////////
//          RPM SENSOR INPUT PIN          //
////////////////////////////////////////////
// int rpmsensorPin = 2; //hall effect

////////////////////////////////////////////
//           IR SENSOR INPUT PIN          //
////////////////////////////////////////////
const int IR_PIN = 6;  // IR sensor input pin

/////////// RPM RELATED VARIABLES //////////
// float revs;               // 
int rpm=0;                // TOTAL RPM
// volatile byte rpmcount;
long previousmicros = 0;
long interval = 5;  // 5 seconds


////////////////////////////////////////////
//         PITCH CONTROL PINS             //
////////////////////////////////////////////
int hystresis = 20;
int motor1pin1 = 7;
int motor1pin2 = 8;
int motor2pin1 = 9;
int motor2pin2 = 10;
int speedPin1 = 11;
int ledPin1  = 13;
int buttonpin1 = 4;
int Pitch_state = 0;
int a;

/////////////////////////////////////////////
//           TEMPORARY VARIABLES           //
/////////////////////////////////////////////
int pitch_state=0;
int pitch_fstate=0;
int irpin_state=0;
int laststate = HIGH;
volatile unsigned int counter = 0;  // Counter variable for revolutions
int pitch_direction=0;
int pitch_duration=0;
int rpm_change=0;
/////////////////////////////////////////////


// void RPMfn()
// {
//   rpmcount++;
// } 

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  pinMode(speedPin1, OUTPUT); 
  // pinMode(10, OUTPUT);
  // pinMode(buttonpin1,INPUT_PULLUP);
  pinMode(buttonpin1,INPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(IR_PIN, INPUT);
  // pinMode(rpmsensorPin, INPUT);
  // attachInterrupt(0, RPMfn, RISING);

  dht.begin();
}

void loop() {

   //////////////////////////////////////////////////////
  // START:   PITCH CONTROL SECTION                   //
  //////////////////////////////////////////////////////
  
  /////////////////// SPEED CONTROL ////////////////////
  analogWrite(speedPin1, 100); //ENA pin
  //////////////////////////////////////////////////////
  //              TEMPORARY PUSH BUTTON               //
  //////////////////////////////////////////////////////
  a = digitalRead(buttonpin1);

  rpm_change=0; // Any Change in the RPM? 0=NO, 1=YES


  if (rpm > 100 && rpm <= 500-hystresis) {
    pitch_fstate=1; 
  } else if (rpm > 500 && rpm <= 800-hystresis) {
    pitch_fstate=2; 
  } else if (rpm > 800) {
    pitch_fstate=3;  
  } else if (rpm <= 100-hystresis) {
    pitch_fstate=0; 
  }


  if (pitch_state-pitch_fstate > 0) {
        pitch_direction=-1;
        pitch_duration=5000*(pitch_state-pitch_fstate);
        digitalWrite(motor1pin1, HIGH);
        digitalWrite(motor1pin2, LOW);
        delay(pitch_duration);
        rpm_change=1;
  } else if (pitch_state-pitch_fstate < 0) {
        pitch_direction=1;
        pitch_duration=5000*abs(pitch_state-pitch_fstate);
        digitalWrite(motor1pin1, LOW);
        digitalWrite(motor1pin2, HIGH);
        delay(pitch_duration);
        rpm_change=1;
  } 

  if (rpm_change==1) {
        previousmicros = micros();
        // Serial.println("");
        // Serial.println("************************************************************");
        // Serial.print(" RPM "); Serial.print(rpm);
        // Serial.print(" Changing From "); Serial.print(pitch_state);
        // Serial.print(" To "); Serial.println(pitch_fstate);
        // Serial.print(" DIRECTION "); Serial.println(pitch_direction);
        // Serial.print(" DURATION "); Serial.println(pitch_duration);
        // Serial.println("************************************************************");
        // delay(1000);
        pitch_state=pitch_fstate;
        digitalWrite(motor1pin1, LOW);
        digitalWrite(motor1pin2, LOW);
  }
  
  //////////////////////////////////////////////////////
  //                    RPM COUNT                     //
  //////////////////////////////////////////////////////
  pinMode(IR_PIN, INPUT);
  int state = digitalRead(IR_PIN);
  if (laststate == LOW && state == HIGH) // only count on a LOW-> HIGH transition
  {
     counter++;
    //  Serial.println(counter);
  }
  laststate = state;  // remember last state

  unsigned long currentmicros = micros();
  if (currentmicros - previousmicros > 1000000*interval)  {
    previousmicros = currentmicros;
    
    rpm = counter *(60/interval);  // Calculate RPM

    counter = 0;

      /////////////////////////////////////////////////////////////
      //                       DHT READINGS                      //
      /////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////
      //           HUMIDITY AND TEMPERATURE               //
      //////////////////////////////////////////////////////
      // Reading humidity and temperature (in C)
      int h = (int) dht.readHumidity();
      int t = (int) dht.readTemperature();
      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
      }

      // Serial.print(" Humidity: ");
      // Serial.print(h);
      // Serial.print("%  Temperature: ");
      // Serial.print(t);
      // Serial.print("C ");
      // Serial.print(" State ");
      // Serial.println(Pitch_state);
      //[Humidity;Temperature_C;RPM;STATE]
      String allValues = "HTRP;"+String(h)+";"+String(t)+";"+String(rpm)+";"+String(pitch_state);
      Serial.println(allValues);
  }  


}

