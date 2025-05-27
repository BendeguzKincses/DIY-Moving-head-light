#include <DMXSerial.h>
#include <Wire.h>

const int led = 13;
const int stEnable = 7;
int dmxCh[] = {0,0,0,0,0,0,0,0,0,0};//10 csatorna
int dmxStartAdress = 1;
int dmxReady = 0;
int ST_EN = 0;
const int L_switch1 = 6;
const int L_switch2 = 5;
const int L_switch3 = 4; //Lens
const int L_switch4 = 3; //ColorWheel
int limit1 = 0;
int limit2 = 0;
int limit3 = 0;
int limit4 = 0; 

void setup()
{
 DMXSerial.init(DMXReceiver);
 pinMode(led,OUTPUT);
 pinMode(stEnable,OUTPUT);
 pinMode(L_switch1,INPUT);
 pinMode(L_switch2,INPUT);
 pinMode(L_switch3,INPUT);
 pinMode(L_switch4,INPUT);
 Wire.begin(8);                /* join i2c bus with address 8 */
 Wire.onReceive(receiveEvent); /* register receive event */
 Wire.onRequest(requestEvent); /* register request event */
}

void receiveEvent(int howMany)//adatok fogadása
{
 while (0 <Wire.available())
  {
    dmxStartAdress = Wire.read();
    ST_EN = Wire.read();
  }
}

void requestEvent()//adatok küldése
{
 Wire.write(dmxCh[0]); //stepper
 Wire.write(dmxCh[1]);
 Wire.write(dmxCh[2]);
 Wire.write(dmxCh[3]);
 Wire.write(dmxCh[4]); //MainBrightness
 Wire.write(dmxCh[5]); //ColorWheelStepper
 Wire.write(dmxCh[6]); //LENS-Beam angle adjuster stepper
 Wire.write(dmxCh[7]); 
 Wire.write(dmxReady);
 Wire.write(limit1);
 Wire.write(limit2);
 Wire.write(limit3);
 Wire.write(limit4);
}

void loop()
{
  if(dmxStartAdress < 1 )
  {
    dmxStartAdress = 1;
  }
  else if(dmxStartAdress > 512)
  {
    dmxStartAdress = 512;
  }

  unsigned long lastPacket = DMXSerial.noDataSince();

  if (lastPacket < 2000)
  {
    for(int i = 0; i<9; i++)
    {
      dmxCh[i] = DMXSerial.read(i + dmxStartAdress);
    }
    dmxReady = 1;
  }
  else
  {
    dmxReady = 0;
    if (millis() / 400 % 2 == 0 )
    {
     digitalWrite(led,1);
    } 
    else 
    {
     digitalWrite(led,0);
    }
  } 

  if(dmxCh[0] > 127 && dmxReady == 1)
  {
    digitalWrite(led,1);
  }
  else if(dmxCh[0] <= 127 && dmxReady == 1)
  {
    digitalWrite(led,0);
  }

  if(ST_EN == 1)
  {
    digitalWrite(stEnable, 1);
  }
  else
  {
    digitalWrite(stEnable,0);
  }

  limit1 = digitalRead(L_switch1);
  limit2 = digitalRead(L_switch2);
  limit3 = digitalRead(L_switch3);
  limit4 = digitalRead(L_switch4);
}


