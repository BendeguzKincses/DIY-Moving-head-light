//Robotlámpa V4; MySpotV4
//150W LED írányító panel ESP32-es programja
//Feladata: LED pwm jel előállítása, LED hőmérséklet mérése, LED hűtő ventillátorok szabályzása, I2C kommunikáció a MySpotV3-as fő irányító panelen található ESP32-essel.
//A programot írta Kincses Bendegúz.

#include <math.h>
#include <Wire.h>

//GPIO
const int SMPSpwmPin = 4; //150W LED MOSFET 
const int LEDFanPwm1Pin = 18; //FŐ LED HŰTŐ VENTILLÁTOR
const int LEDFanPwm2Pin = 19; //MELLÉK LED ÉS LENCSE HŰTŐ VENTILLÁTOR
const int FETFanPwm1Pin = 26;
const int I2C_SDA_Pin = 21; 
const int I2C_SCL_Pin = 22;
const int LEDFan1SpeedSensorPin = 27; //FŐ LED HŰTŐ VENTILLÁTOR SEBBESÉG SZENZOR
const int BuckConverterTempFeedbackPin = 25;  
const int FETTempFeedbackPin = 33; 
const int LEDTempFeedbackPin = 32; //LED HŐMÉRSÉKLET ÉRZÉKELŐ NTC ELLENÁLLÁS
const int CurrentFeedbackPin = 35; //LED ÁRAMÁT ÉRZÉKELŐ 10mOHM-OS ELLENÁLLÁS
const int VoltageFeedbackPin = 34; //NEM FUNKCIONÁL! ; KI KELLETT KÖTNI A PCB-N A MÉRENDŐ FESZÜLTSÉG ALÓL MERT MEGAKADÁLYOZTA ISMERETLEN OK MIATT A PROGRAM FUTÁSÁT

//Values
int SMPSpwmVal = 0; //A 150w-os led meghajtó mosfetjének a pwm jele
int LEDBrightness = 0; //The brightness value that comes from the MySpotV3 main board
int dmxReady = 0; 
int LEDFanPwm2Val = 0; //Ventillátor 2 pwm-sebessége
int LEDFanPwm1Val = 0; //Ventillátor 1 pwm-sebbesége
int LEDFanVal8Bit = 0;
int LEDFanPwmRaw = 0; //A converted value that can be negative or bigger than the maximum value of 511
int FanIdleingValue = 150; //The lowest value the cooling fans will spin on.
float CorrectionBase = 1.018355133;
float CorrectedLEDBrightness = 0;
////LED Temperature 
int LEDTempRaw = 0; //ADC-ből olvasott senzor érték
int LEDTempIntermidiate = 0; //Az átlagoláshoz szükséges átmeneti változó
int LEDTempRawSampled = 0; //ADC-ből olvasott senzor érték átlagolás után
int SampleAvarge1 = 10; //Az átlagolás mértéke a LEDTemp esetén
int SampleCountunter1 = 0;//Az átlagolás kiszámításához szükségesváltozó
int LEDTempCelsius = 0; //Celciusba kb. átváltott LED hőmérséklet
//
int CurrentFeedbackValRaw = 0; //LED áram   //The value of the shunt resistor is 10mOhm
int CurrentFeedbackValRawSampled = 0;
int CurrentFeedbackValIntermidiate = 0;
int VoltageFeedbackValRaw = 0; //LED feszültség
float CurrentFeedbackMultiplier = 0.000732421875;    //   3.3V / 2^12
float CurrentFeedbackValAmperFloat = 0;
int CurrentFeedbackValAmperInteger = 0;
int VoltageFeedbackValVolt = 0;
int SampleAvarge2 = 10; //Az átlagolás mértéke a LEDTemp esetén
int SampleCountunter2 = 0;//Az átlagolás kiszámításához szükségesváltozó
bool FanTest1 = 0; //A bit to keep track that the fans had been turned on at the startup of the device.
bool FanTest2 = 0;
int SMPSpwmCh = 0; //The channel of the pwm timers.
int LEDFanPwm1Ch = 4; //The channel of the pwm timers.
int LEDFanPwm2Ch = 5; //The channel of the pwm timers.
int FETFanPwm1Ch = 6; //The channel of the pwm timers.
//Serial
int time0 = 0;
int time0Flag = 0;
//LEDFanControlTimers
int time1 = 0;
int time1Flag = 0;

const byte numChars = 5; //A LED manuális fényerőállításhoz szükséges adatok
char receivedChars[numChars];   // an array to store the received data
int receivedIntegers[numChars]; // an array to store the recievedChars converted to integers
int SerialPwmVal = 0; //An integer for the recieved data.
int CharToIntCyle = 0;
int IntArrayToIntCyle = 0;
int ResettingRecievedIntegers = 0;
bool newData = 0;



//*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*//
//*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*//


void setup() {
   //Serial
  Serial.begin(9600);
  //PinModes
  pinMode(VoltageFeedbackPin, INPUT_PULLDOWN); //Nem funkcionál! Ki lett kötve a PCB-n mert nem futott a program amikor be volt kötve.
  digitalWrite(VoltageFeedbackPin, 0);
  pinMode(CurrentFeedbackPin, INPUT);
  pinMode(LEDTempFeedbackPin, INPUT);
  pinMode(FETTempFeedbackPin, INPUT);
  pinMode(SMPSpwmPin, OUTPUT);
  //pinMode(4, INPUT);
  //pinMode(34, INPUT_PULLDOWN);
  pinMode(LEDFanPwm1Pin, OUTPUT);
  pinMode(LEDFanPwm2Pin, OUTPUT);
  pinMode(FETFanPwm1Pin, OUTPUT);


  //Configuring the PWM timers
  ledcSetup(SMPSpwmCh, 1000, 11);  //The sample:ledcSetup(channel, freq, resolution)
  ledcSetup(LEDFanPwm1Ch, 25000, 9); 
  ledcSetup(LEDFanPwm2Ch, 25000, 9);
  ledcSetup(FETFanPwm1Ch, 25000, 9);

  ledcAttachPin(SMPSpwmPin, SMPSpwmCh); //The sample:ledcAttachPin(pin, channel) 
  ledcAttachPin(LEDFanPwm1Pin, LEDFanPwm1Ch); //Attaching the channels and the output pins together.
  ledcAttachPin(LEDFanPwm2Pin, LEDFanPwm2Ch);
  ledcAttachPin(FETFanPwm1Pin, FETFanPwm1Ch); 

  
  ledcWrite(SMPSpwmCh, 0); //The sample: ledcWrite(channel, dutycycle)
  ledcWrite(LEDFanPwm1Ch, 511); //Setting the starter value of the pwm dutycyle.
  ledcWrite(LEDFanPwm2Ch, 511); //Min value of the duty cyle: 0 And max 511
  ledcWrite(FETFanPwm1Ch, 511); //Setting the FANs to fuul speed.
 
  Serial.println("ESP32 is ready");
}
//*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*//
//*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*//

void FanTurnOffAfterStartup()
{
 if(FanTest1 == 0)
 {
   if(millis() > 12000)
   {
     ledcWrite(LEDFanPwm1Ch, 400); //Fans HALF SPEED //4 seconds after startup
     ledcWrite(LEDFanPwm2Ch, 400); 
     ledcWrite(FETFanPwm1Ch, 400); 
     FanTest1 = 1;
   }
 }
 if(FanTest2 == 0)
 {
    if(millis() > 10000)
   {
     ledcWrite(LEDFanPwm1Ch, 300); //Fans STOP //8 seconds after startup
     ledcWrite(LEDFanPwm2Ch, 300); 
     ledcWrite(FETFanPwm1Ch, 300); 
     FanTest2 = 1;
   }
 } 
}

//=============================================================================================================================================================//

void SerialPrintingPerSecond()
{

  if(time0Flag == 0)
 {
   time0 = millis();
   time0Flag = 1;
 }
 
 if(millis() - time0 > 1000)
 {
   time0 = 0;
   time0Flag = 0;
   Serial.print("LEDTempRawSampled: ");
   Serial.print(LEDTempRawSampled);
   Serial.print("\t");
   Serial.print("LEDTempCelsius: ");
   Serial.print(LEDTempCelsius);
   Serial.print("\t");
   Serial.print("LEDFanPwm1&2Val: ");
   Serial.print(LEDFanPwm1Val);
   Serial.println();//carrige return
   Serial.print("LEDBrightness: ");
   Serial.print(LEDBrightness);
   Serial.print("\t");
   Serial.print("dmxReady: ");
   Serial.print(dmxReady);
   Serial.print("\t");
   Serial.print("LED Current: ");
   Serial.print(CurrentFeedbackValAmperInteger);
   Serial.print("\t");
   Serial.print("CurrentFeedbackValRawSampled: ");
   Serial.print(CurrentFeedbackValRawSampled);
   Serial.print("\t");
   Serial.print("CurrentFeedbackValRaw: ");
   Serial.print(CurrentFeedbackValRaw);
   Serial.println();//carrige return
 }

}

//=============================================================================================================================================================//

void AvaregingADCValues()
{
 if(SampleCountunter1 < SampleAvarge1)
 {
   LEDTempIntermidiate = LEDTempIntermidiate + LEDTempRaw;
   SampleCountunter1++;
 }
  else
 {
   LEDTempRawSampled = LEDTempIntermidiate / SampleAvarge1;
   LEDTempIntermidiate = 0;
   SampleCountunter1 = 0;
 }




 if(SampleCountunter2 < SampleAvarge2)
 {
   CurrentFeedbackValIntermidiate = CurrentFeedbackValIntermidiate + CurrentFeedbackValRaw;
   SampleCountunter2++;
 }
  else
 {
   CurrentFeedbackValRawSampled = CurrentFeedbackValIntermidiate / SampleAvarge2;
   CurrentFeedbackValIntermidiate = 0;
   SampleCountunter2 = 0;
 }

}

//=============================================================================================================================================================//
//I copied and edited this serial recieve tutorial from an online tutorial. Here is the link: https://forum.arduino.cc/t/serial-input-basics-updated/382007

void recieveWithEndMarker()
{
 static byte ndx = 0;
    char endMarker = '\n';
    char rc; 
    
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars)
             {
                ndx = numChars - 1;
             }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void showNewData() {
    if (newData == true) 
    {
        Serial.print("New value arrived: ");
        Serial.println(receivedChars);
        newData = false;

   /*     for(ResettingRecievedIntegers = 0 ; ResettingRecievedIntegers < 10 ; ResettingRecievedIntegers++)
        {
          receivedIntegers[ResettingRecievedIntegers] = 0;
        }
      */  
        SerialPwmVal = 0;

        for(CharToIntCyle = 0 ; CharToIntCyle < 4 ; CharToIntCyle++)  //This converts the char array into a integer arra
        {
         receivedIntegers[CharToIntCyle] = receivedChars[CharToIntCyle] - '0';
        }
        SerialPwmVal = SerialPwmVal + receivedIntegers[0]*1000 + receivedIntegers[1]*100 + receivedIntegers[2]*10 + receivedIntegers[3];

        /*Serial.print("RecievedChars: ");
        Serial.print(receivedChars[0]);
        Serial.print(receivedChars[1]);
        Serial.print(receivedChars[2]);
        Serial.print(receivedChars[3]);
        Serial.println();
        Serial.print("RecievedIntegers: ");
        Serial.print(receivedIntegers[0]);
        Serial.print(receivedIntegers[1]);
        Serial.print(receivedIntegers[2]);
        Serial.print(receivedIntegers[3]);
        Serial.println();*/
        Serial.print("SerialPwmVal: ");
        Serial.print(SerialPwmVal);
        Serial.println();
    }
}

//=============================================================================================================================================================//

void LEDTempAndLEDFanCalculations()
{
  LEDTempCelsius = map(LEDTempRawSampled , 2370 , 3800 , 26 , 65);
  LEDFanPwmRaw = map(LEDTempCelsius, 30 , 55, FanIdleingValue , 511);

  if(LEDFanPwmRaw > (FanIdleingValue - 1) && LEDFanPwmRaw < 512)
  {
    LEDFanPwm1Val = LEDFanPwmRaw;
  }
  else if(LEDFanPwmRaw > 511)
  {
    LEDFanPwm1Val = 511;
  }
  else
  {
    LEDFanPwm1Val = FanIdleingValue - 5;    //állandó minimum  sebesség a ventillátoroknak azért hogy ne akadjanak meg a csapágyaik alacsony fordulatszáos indulásnál
  }
  LEDFanPwm2Val = LEDFanPwm1Val;//+50;

  LEDFanVal8Bit = map(LEDFanPwm1Val, 0 , 511 , 0 , 255);
}

//=============================================================================================================================================================//

void LEDFanPwmControlAndExecution()
{

 if(time1Flag == 0)
 {
   time1 = millis();
   time1Flag = 1;
 }
 
 if(millis() - time1 > 5000)
 {
   time1 = 0;
   time1Flag = 0;
   if(FanTest1 == 1 && FanTest2 == 1)
   {
   ledcWrite(LEDFanPwm1Ch, LEDFanPwm1Val);
   ledcWrite(FETFanPwm1Ch, LEDFanPwm2Val);
   }
 }

}

//=============================================================================================================================================================//

void I2C()
{
 Wire.begin(9);                /* join i2c bus with address 9 */
 Wire.onReceive(receiveEvent); /* register receive event *///Receiving data from MySpotV3 board to controll the brightess
 Wire.onRequest(requestEvent); /* register request event *///for sending data
}

void requestEvent()//adatok küldése
{
 Wire.write(LEDFanVal8Bit); //led hűtőventi értékének az az elküldése
 Wire.write(LEDTempCelsius); //led hőmérsékletének az elküldése
}

void receiveEvent(int howMany)//adatok fogadása
{
 while (0 <Wire.available())
  {
    LEDBrightness = Wire.read();
    dmxReady = Wire.read();
  }
}

//*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*//
//*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*//

void loop() {

recieveWithEndMarker();
showNewData();
FanTurnOffAfterStartup();
SerialPrintingPerSecond();
AvaregingADCValues();
LEDTempRaw = analogRead(LEDTempFeedbackPin);
CurrentFeedbackValRaw = analogRead(35);
CurrentFeedbackValAmperFloat = (CurrentFeedbackValRawSampled * CurrentFeedbackMultiplier) / 0.01; // I = U / R   
CurrentFeedbackValAmperInteger = round(CurrentFeedbackValAmperFloat);
LEDTempAndLEDFanCalculations();
LEDFanPwmControlAndExecution();
I2C();

if(dmxReady == 1)
{
 CorrectedLEDBrightness = pow(CorrectionBase , LEDBrightness) * 20 - 20; // f(x) = 1.018355133^x * 20 -20  //x^255 * 20 - 20 = 2047 // x -> 0-255  // f(x) -> 0-2047
 if(LEDTempCelsius < 60)
 {
   ledcWrite(SMPSpwmCh, CorrectedLEDBrightness);
 }
 else
 {
   ledcWrite(SMPSpwmCh, 0);
 }
 
}
else
{
 
 if(LEDTempCelsius < 60)
 {
   ledcWrite(SMPSpwmCh, SerialPwmVal);
 }
 else
 {
   ledcWrite(SMPSpwmCh, 0);
 }
}

   
 
}
 





 
 


