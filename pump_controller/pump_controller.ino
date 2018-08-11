#include <DS1307RTC.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <Wire.h>


int relay_1 = 6;
int relay_2 = 7;
int man_fill = 8;
int man_spray = 9;

// flow sensor vars
volatile int NbTopsFan; //measuring the rising edges of the signal
int hallsensor = 2;    //The pin location of the sensor

// water sensor vars
int water_sens = 3;

int fill_time = 60; // in seconds
int flow_thresh = 10; // in L/hr

void setup() {
  Serial.begin(9600);
  // while (!Serial) ; // wait for serial
  Alarm.delay(200);

  // Set internal time to RTC
  setSyncProvider( RTC.get );
  toLog("RTC set");
  
  // Set up relays
  pinMode(relay_1, OUTPUT);
  pinMode(relay_2, OUTPUT);
  digitalWrite(relay_1, HIGH); //Turn off relay
  digitalWrite(relay_2, HIGH); //Turn off relay
  
  // Set up water sensor
  pinMode(water_sens, INPUT_PULLUP);
  
  // Set up flow sensor
  pinMode(hallsensor, INPUT_PULLUP); //initializes digital pin 2 as an input
  attachInterrupt(0, rpm, RISING);   //and the interrupt is attached

  // Set up manual triggers
  pinMode(man_fill, INPUT_PULLUP);
  pinMode(man_spray, INPUT_PULLUP);

  // Initialze timers
  // Alarm.timerRepeat(10, Fill); fill_time=5; // testing fill timer 
  Alarm.timerRepeat(5400, Fill); // fill timer, 1.5 hr 
  // Alarm.timerRepeat(4500, Fill); // fill timer, 1 hr 15min
  // Alarm.timerRepeat(3600, Fill); // fill timer, once an hour 
  //Alarm.timerRepeat(1800, Fill); // fill timer, twice an hour 


  // Alarm.alarmRepeat(hour(),minute(),second()+5, Spray); // testing spray time
  Alarm.alarmRepeat(8,30,0, Spray);  // 8:30am every day


}


void loop() {
  // digitalClockDisplay();
  if( digitalRead(man_fill) == 0 ) { toLog("Manual fill"); Fill(); }
  if( digitalRead(man_spray) == 0 ) { toLog("Manual spray"); Spray(); }
  Alarm.delay(1000);
//check_flow();


}



void Fill()
{  
  toLog("--Fill cycle--");
  if ( digitalRead(water_sens) == 1 ) { toLog("Tank full"); toLog("--End fill cycle--"); return; }
  long end_fill = now() + fill_time;
  
  toLog("Start well pump");
  while ( end_fill > now() ) 
    {
      digitalWrite(relay_1, LOW); // Turn on relay
      Alarm.delay(1000); 
      if ( digitalRead(water_sens)==1 ) { toLog("Tank full"); break; }
      if ( digitalRead(man_fill)==0 ) { toLog("Manual stop"); break; }
    }
  toLog("Stop well pump");
  digitalWrite(relay_1, HIGH); // Turn off relay

  toLog("--End fill cycle--");
  return;
}


void Spray()
{
  toLog("--Start spray cycle--");
  toLog("Start spray pump");
  digitalWrite(relay_2, LOW); // Turn on relay

  toLog("Priming delay");
  Alarm.delay(5000);
  int flow = check_flow();
  toLog ( String( "Flow: " + String(flow) ) );
  while ( flow > flow_thresh ) {
    //Alarm.delay(4000);
    flow = check_flow();
    toLog ( String( "Flow: " + String(flow) ) );
    if ( digitalRead(man_spray)==0 ) { toLog("Manual stop"); break; }

  }
  toLog("Stop spray pump");
  digitalWrite(relay_2, HIGH); // Turn off relay
  
  toLog("--End spray cycle--");
}



int check_flow ()
{
  int Calc;      
  NbTopsFan = 0;   //Set NbTops to 0 ready for calculations
//  sei();      //Enables interrupts
  Alarm.delay(1000);   //Wait 1 second
//  cli();      //Disable interrupts
//  Calc = (NbTopsFan * 60 / 5.5); //(Pulse frequency x 60) / 5.5Q, = flow rate in L/hour 
  Calc = (NbTopsFan / 5.5); // Pulse frequency / 5.5Q, = flow rate in L/min 

//  Serial.print (Calc, DEC); //Prints the number calculated above
//  Serial.print (" L/hour\r\n"); //Prints "L/hour" and returns a  new line 
  return Calc;
}

void rpm ()     //This is the function that the interupt calls 
{ 
  NbTopsFan++;  //This function measures the rising and falling edge of the hall effect sensors signal
} 



void toLog(String message) 
{
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(": ");
  Serial.println(message);
}


void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println(); 
}

void printDigits(int digits)
{
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
