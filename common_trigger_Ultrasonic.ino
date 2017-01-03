/*
To understand what this code is doing, I will first explain the pre-existing methods 
(using pulseIn and ping timer in new ping library)

PulseIn method-
the pulseIn method pings a sensor and then uses pulseIn function to get the round trip time.
The disadvantage in this is that the microcontroller keeps waiting and is "stuck" at the pulseIn .
The controller can't do anything else for that time. This greatly increases the processing time 
because time has to be given to the main processing as well as the ultrasonic sensors. 
The "Hardware" response time is what drags down the entire bot in this case

New ping method(interrupts)
This method uses interrupts to measure the round trip time. The advantage is that unlike in
the case of pulseIn method, here the microcontroller is not stuck waiting for the pin to 
fall LOW and can use the time in between to do some processing. 
The disadvantage however is that although the "Hardware" no longer controlling the cycle time, 
the update rate on the ultrasonic sensors is still the same as the pulseIn method 
(you ping one sensor first and ping the second one only when the first one's pin falls low, ).
So for example if you have 3 sensors, worst case scenario, if there is no obstacle, 
the SCR04 ECHO will(ideally) fall LOW at 29 ms, so the total update time is 87 ms

Common trigger pin method-
Also uses interrupts (on a group of pins) but pings all sensors simultaneously,
hence, worst case scenario, we get 29 ms of total update time(Actually a little more than
that because if all of them fall LOW at 29 ms then they will be noticed in the ascending order from 8 to 13)
and we get all the advantages of new ping method(can process data while the ultrasonic waves are coming back)
The one disadvantage(which i can think of off the top of my head) is that the ultrasonic sensors have to be 
oriented appropriately such that any 2 ultrasonics do not notice each other's reflected wave.

This particular example uses pins 8-13 (B port) (PCIE0) but is expandable. 

To ensure that we dont ping the sensors again before an entire cycle is done,
I have put a counter variable that observes how many sensors have given 
an answer so far. 

*/


#define sensor 6 //number of sensors

//variables for doing the interrupt stuff
volatile unsigned long timer[7];
byte last_channel[6];
volatile int input[6];  


#define TRIG 6  //common trigger pin 

long s[6];  //distance variables

volatile uint8_t counter=0; //counter used for keeping a track of when the sensor cycle is complete

void setup()
{
  PCICR  |= (1 << PCIE0);        //enabling PC interrupts on B port
  PCMSK0 |= (1 << PCINT0); //8
  PCMSK0 |= (1 << PCINT1); //9
  PCMSK0 |= (1 << PCINT2); //10
  PCMSK0 |= (1 << PCINT3); //11
  PCMSK0 |= (1 << PCINT4); //12
  PCMSK0 |= (1 << PCINT5); //13
  Serial.begin(38400);
  pinMode(TRIG,OUTPUT); 
}

ISR(PCINT0_vect)          //interrupt servie routine 
{
  timer[0]=micros();      //note the time at which the isr is called
  
  if(last_channel[0]==0&& PINB & B00000001) //pin 8 is being checked for high right now
  {                                         
    last_channel[0]=1;       //8th pin is checked first, the others will be missed /noticed later, but because we know all of them are initialized at the same time, 
    last_channel[1]=1;      //we initialize all the timers and channels together
    last_channel[2]=1;
    last_channel[3]=1;
    last_channel[4]=1;
    last_channel[5]=1;
    
    timer[1]=timer[0];          
    timer[2]=timer[0];
    timer[3]=timer[0];
    timer[4]=timer[0];
    timer[5]=timer[0];
    timer[6]=timer[0];

    counter=0;               //initiate counter at 0 ,as each pin falls low, the counter increases. this is done so that we dont ping the sensors before all of them are done
  }
  
  //pin 8
  else if(last_channel[0]==1 && !(PINB & B00000001))   //when pin 8 falls low
  {
    last_channel[0]=0;                              
    input[0]=timer[0]-timer[1];                        //calculate time difference between when all were pinged and when 8th pin fell LOW
    counter++;
  }

  //pin 9 
  else if(last_channel[1]==1 && !(PINB & B00000010))
  {
    last_channel[1]=0;
    input[1]=timer[0]-timer[2];
    counter++;
  }

  //pin 10  
  else if(last_channel[2]==1 && !(PINB & B00000011))
  {
    last_channel[2]=0;
    input[2]=timer[0]-timer[3];
    counter++;
  }

  //pin 11
  else if(last_channel[3]==1 && !(PINB & B00000100))
  {
    last_channel[3]=0;
    input[3]=timer[0]-timer[4];
    counter++;
  }

  //pin 12 
  else if(last_channel[4]==1 && !(PINB & B00000101))
  {
    last_channel[4]=0;
    input[4]=timer[0]-timer[5];
    counter++;
  }

  //pin 13 
  else if(last_channel[5]==1 && !(PINB & B00000110))
  {
    last_channel[5]=0;
    input[5]=timer[0]-timer[6];
    counter++;
  }
}
  

long microsecondsToCentimeters(long microseconds) //function to convert time into distance  
{
  return microseconds/29/2; //launde ko physics ati hai
}


void pinger() //function to ping the sensors 
{
  digitalWrite(TRIG,LOW);  //trigger pin low so that we get a clean high
  digitalWrite(TRIG,HIGH);  //trigger pin smokes some weed
  delayMicroseconds(10);            // trigger gets normal after 10 microseconds
  digitalWrite(TRIG,LOW);  
}



void loop()
{
  if(counter==sensor)    //when all the sensors have fallen low
  {
    pinger();  //ping the sensors again only when the cycle is complete.
  }
  for(uint8_t i=0;i<sensor;i++)
  {
    s[i]=microsecondsToCentimeters(input[i]);
    Serial.print(s[i]);Serial.print(" ");
  }
}











