//LIBRARIES
#include "Wire.h" //library for the I2C communication
#include <Adafruit_GFX.h>//library for the small OLED screen
#include <Adafruit_SSD1306.h>//library for controlling the small OLED screen
#include <AccelStepper.h>//library for controlling the stepper motors
#include <math.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
TaskHandle_t Task1; //code for running tasks on two CPU cores

//PINS
//const int ledPin[] = {27,14,12};//{RedPin,GreenPin,BluePin}
const int ST1 = 16; //Stepper motor 1 STEP control
const int ST2 = 2; //Stepper motor 2 STEP control
const int ST3 = 18; //Stepper Lens
const int ST4 = 14; //Stepper Color Wheel
const int D1 = 4; //Stepper motor 1 DIRECTION control
const int D2 = 15; //Stepper motor 2 DIRECTION control
const int D3 = 5; //Stepper Lens
const int D4 = 12; //StepperColorWheel
const int L_SW = 39;//Limit switch for the stepper motor initialization
const int PB[] = {33,32,35,34};////Push Buttons 1,2,3,4 on the user interface board
const int IndR = 25;//RED->Indicator LED on the user interface board
const int IndG = 26;//GREEN->Indicator LED on the user interface board
const int FAN = 19;//12V-os ventillátor vezérlése
const int MainSwitch = 13;//relé kapcsolása
AccelStepper stepper1(1, ST1, D1);//define pins for stepper driver (Typeof driver: with 2 pins, STEP, DIR)
AccelStepper stepper2(1, ST2, D2);//define pins for stepper driver (Typeof driver: with 2 pins, STEP, DIR)
AccelStepper stepper3(1, ST3, D3);//define pins for stepper driver (Typeof driver: with 2 pins, STEP, DIR)
AccelStepper stepper4(1, ST4, D4);//define pins for stepper driver (Typeof driver: with 2 pins, STEP, DIR)
//VALUES
//led
double led[] = {0,0,0,0}; //{R,G,B,Master Brightness}
int ledRounded[] = {0,0,0,0}; //{R,G,B,Master Brightness}
const int freq = 1000;
const int ledChannel[] = {0,1,2};//{R,G,B} PWM Channels
const int resolution = 8;
int LEDBrightness = 0; //The brightness value of the 150W LED that is controlled by the MySpotV4 board.
//-values for the buttons

int ST_EN = 0;
int button2 = 1; //2-es nyomógomb számlálója
int button2_OLED = 0; // button2 értéke páros-e vagy páratlan; páros = 1; páratlan = 0;
//miscellaneous
int dmxStartAdress = 1;//value for the displayed number on the OLED display
int maxSpeed = 2500;//sebbeség kalibráció közben
int maxAccel = 500;
int maxSpeed2 = 60000;
int maxAccel2 = 8000;
float stepper1_16bit = 0;
float stepper2_16bit = 0;
float stepper1_pos = 0;
float stepper2_pos = 0;
float stepper3_pos = 0;
float stepper4_pos = 0;
int step1_pos_round = 0;
int step2_pos_round = 0;
int step3_pos_round = 0;
int step4_pos_round = 0;
float PANmultiplier = 0.388125;
float TILTmultiplier = 0.1276311275;
int stepper_calibrated = 0;
int stepper2_calibrated = 0;
int stepper3_calibrated = 0; //lens
int stepper4_calibrated = 0; //color wheel
int main_count = 0;
int alreadyRun = 0;
int limit1 = 0;//végállás kapcsolók
int limit1_pushed = 0;
int limit2 = 0;
int limit2_pushed = 0;
int limit3 = 0;
int limit3_pushed = 0;
int limit4 = 0;
int limit4_pushed = 0;
int dmxCh[] = {0,0,0,0,0,0,0,0};//8 csatorna kiolvasása
float dmxCh_f[] = {0,0,0,0,0,0,0,0};//8 csatorna kiolvasása
int dmxReady = 0;
//FAN control values
int fan_state = 0;
float fan_pwm_f = 0;
int fan_pwm_i = 0;
float fan_pwm_out = 0;
int fan_channel = 3;
double FanTime = 0;
double FanT10 = 16;
double FanT20 = 16;
double FanT30 = 16;
double Fan32 = 0;
int LEDTempCelsius = 0;
int LEDFanPwm1Val = 0;
//times for the OLED-writing
int time_calib1 = 0;
int time_calib2 = 0;
int calib_done = 0;
int time_calib1_alreadydone = 0;

//----------------------------------------------------------------//
//MENU SYSTEM//

//Buttons:
//From left to right:
//--------PB1--------//--------PB2--------//--------PB3--------//--------PB4--------//
//--Menu dowm        //--Menu up          //--Value down       //--Value up

/*Menus
1. Dmx Adress //Val:1-512  //step:1
2. Input selection //Text:DMX,Manual//Val:1-2  //step:1
3. LED //Val:0-255  //step:1
4. Lens //Val:0-255  //step:1
5. Color Wheel //Val:0-255  //step:1
   Colors: 0-L.Green_0 ; 1-L.Purple_17 ; 2-L.Pink_35 ; 3-LemonY._54 ; 4-D.Pink_71 ; 5-Blue_87 ; 6-Amber_107 ; 7-WarmW._125 ; 8-Half-White._143 ; 9-D.Blue_160 ; 10-White_178 ; 11-Red_195 ; 12-Orange_213 ; 13-Cyan_232 ; 14-D.Green_248
6. PAN //Val:0-255  //step:1
7. TILT //Val: 0-255  //step:1
8. PAN speed //Val:??  //step:1
9. TILT speed //Val:??  //step:1
10. Recalibration //Text:NO,YES//Val:0-1  //step:1
11. Stepper en. //Text:NO,YES//Val:0-1  //step:1
*/
//Values:
int MenuCounter = 1;
int ValueCounter[] =     {0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//The array is used from 1 and not from 0. So Menu1 corresponds to value[1]
                                                           //Each menu has its own value.
int ValueCounterStep[] = {1,1,1,5,10,1,5,5,1,1,1,1,1,1,1,1,1,1}; //This array stores the size of the steps that is taken while counting ip or down.
int ValueCounterMax[]  = {0,512,2,255,255,14,255,255,255,255,1,1};
int ValueCounterMin[]  = {0,1,1,0,0,0,0,0,0,0,0,0,0};
int wait = 0;
int count = 0;
int wait2 = 0;
int count2 = 0;
int wait3 = 0;
int count3 = 0;
int wait4 = 0;
int count4 = 0;
int PB_state[] = {0,0,0,0};//Push button high or low state
int MenuSpeed = 10;
int MenuColor = 0;
//----------------------------------------------------------------//


void I2C()
{
 Wire.beginTransmission(8); /* begin with device address 8 */  //Sending to the Atmega328p
 Wire.write(dmxStartAdress);
 Wire.write(ST_EN);
 Wire.endTransmission();    /* stop transmitting */
 delay(1);
 Wire.beginTransmission(9); /* begin with device address 9*/  //Sending to the MySpotV4 powerLED control board
 Wire.write(LEDBrightness);
 if(ValueCounter[2] == 1)
 {
  Wire.write(dmxReady);
 }
 else 
 {
  Wire.write(1);
 }
 Wire.endTransmission();    /* stop transmitting */
 delay(1);

 Wire.requestFrom(8, 13); /* request & read data of size 7 from slave */
 dmxCh[0]=Wire.read();
 dmxCh[1]=Wire.read();
 dmxCh[2]=Wire.read();
 dmxCh[3]=Wire.read();
 dmxCh[4]=Wire.read();
 dmxCh[5]=Wire.read();
 dmxCh[6]=Wire.read();
 dmxCh[7]=Wire.read();
 dmxReady=Wire.read();
 limit1=Wire.read();
 limit2=Wire.read();
 limit3=Wire.read();
 limit4=Wire.read();
 delay(1);
 Wire.requestFrom(9, 2); /* request & read data of size 7 from slave */
 LEDFanPwm1Val=Wire.read();
 LEDTempCelsius=Wire.read();
 delay(1);
}

void OLED_writing()
{

//SYSTEM STARTUP

  if(millis() < 3000 ) //"Kincses" felirat a kijelzőn
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(4, 10);
    display.println("Kincses");
    display.display();
  }
  if(millis() > 4500 && millis() < 6000)  //"MySpot" felirat a kijelzőn
   {
    display.clearDisplay();
    display.setCursor(4, 10);
    display.println("MySpot");
    display.display();
   }
   if(millis() > 6000 && stepper2_calibrated == 0) //"Calibrating" felirat a kijelzőn és a két végálláskapcsoló állása a kijelzőn
   {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.println("Calibrating...");
    display.setCursor(120, 16);
    display.println(limit1_pushed);
    display.setCursor(120, 24);
    display.println(limit2_pushed);
    display.display();
   }
   if(stepper_calibrated == 1 && stepper2_calibrated == 1 && stepper3_calibrated == 1 && stepper4_calibrated == 1 && calib_done == 0) //"Done" felirat a kijelzőn a calibrálástól számítot 1,5 másodpercig
   {
     if(time_calib1_alreadydone == 0)
     {
      time_calib1 = millis();
      time_calib1_alreadydone = 1;
     }
    time_calib2 = millis() - time_calib1;
    if(time_calib2 < 1500)
    {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(4, 10);
    display.println("DONE");
    display.display();
    }
    else if(time_calib2 > 1500)
    {
    calib_done = 1;
    }
   }

    if(calib_done == 1) //Menu system:
    {
      if(MenuCounter == 1) //DMX Adress
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("Adress:");
        display.setCursor(0, 16);
        display.println(ValueCounter[MenuCounter]);
        display.display();
      }
      else if(MenuCounter ==2) //Input selection
      {
        if(ValueCounter[2] == 1)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("INPUT:");
         display.setCursor(0, 16);
         display.println("DMX");
         display.display();
        }
        else if(ValueCounter[2] == 2)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("INPUT:");
         display.setCursor(0, 16);
         display.println("Manual");
         display.display();          
        }
      }
      else if(MenuCounter ==3) //LED
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("LED:");
        display.setCursor(0, 16);
        display.println(ValueCounter[MenuCounter]);
        display.display();
      }
      else if(MenuCounter ==4) //Lens
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("Lens:");
        display.setCursor(0, 16);
        display.println(ValueCounter[MenuCounter]);
        display.display();
      }
      else if(MenuCounter == 5) //ColorWheel
      {
        if(ValueCounter[5] == 0)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("L.Green");
         display.display();    
        }
        else if(ValueCounter[5] == 1)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("L.Purple");
         display.display();    
        } 
        else if(ValueCounter[5] == 2)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("L.Pink");
         display.display();    
        }        
        else if(ValueCounter[5] == 3)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("LemonY");
         display.display();    
        }
        else if(ValueCounter[5] == 4)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("D.Pink");
         display.display();    
        }
        else if(ValueCounter[5] == 5)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("Blue");
         display.display();    
        }
        else if(ValueCounter[5] == 6)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("Amber");
         display.display();    
        }
        else if(ValueCounter[5] == 7)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("WarmW.");
         display.display();    
        }
        else if(ValueCounter[5] == 8)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("Half-White.");
         display.display();    
        }
        else if(ValueCounter[5] == 9)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("D.Blue");
         display.display();    
        }
        else if(ValueCounter[5] == 10)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("White");
         display.display();    
        }
        else if(ValueCounter[5] == 11)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("Red");
         display.display();    
        }
        else if(ValueCounter[5] == 12)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("Orange");
         display.display();    
        }
        else if(ValueCounter[5] == 13)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("Cyan");
         display.display();    
        }
        else if(ValueCounter[5] == 14)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("COLOR:");
         display.setCursor(0, 16);
         display.println("D.Green");
         display.display();    
        }
         //End of colors// 
      }
      else if(MenuCounter ==6) //PAN
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("PAN pos.:");
        display.setCursor(0, 16);
        display.println(ValueCounter[MenuCounter]);
        display.display();
      }
      else if(MenuCounter ==7) //TILT
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("TILT pos.:");
        display.setCursor(0, 16);
        display.println(ValueCounter[MenuCounter]);
        display.display();
      }
      else if(MenuCounter ==8) //PAN speed
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("LED Temp.:");
        display.setCursor(40, 16);
        display.println("C");
        display.setCursor(0, 16);
        display.println(LEDTempCelsius);
        display.display();
      }
      else if(MenuCounter ==9) //Tilt speed
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("FAN speed:");
        display.setCursor(0, 16);
        display.println(LEDFanPwm1Val);
        display.display();
      }
      else if(MenuCounter ==10) //Recalibration
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("Recalib.:");
        display.setCursor(0, 16);
        display.println(ValueCounter[MenuCounter]);
        display.display();
      }
      else if(MenuCounter ==11) //Stepper enable
      {
        if(ValueCounter[11] == 0)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("ST enable:");
         display.setCursor(0, 16);
         display.println("YES");
         display.display();
        }
        else if(ValueCounter[11] == 1)
        {
         display.clearDisplay();
         display.setTextSize(2);
         display.setCursor(0, 0);
         display.println("ST enable:");
         display.setCursor(0, 16);
         display.println("NO");
         display.display();          
        }
      }
      else if(MenuCounter < 1 || MenuCounter > 11) 
      {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("OUT OF:");
        display.setCursor(0, 16);
        display.println("RANGE");
        display.display();
      }
    }
   
   
}

void button_handling ()
{
  
   
    for(int i=0;i<=3;i++)
  {
    PB_state[i] = digitalRead(PB[i]);
  }

//Menu counter-----------------------------------------//
  if (PB_state[1] == HIGH) //Button2 //Counting up
  {
    if(wait == 0)  
    {
     if(MenuCounter < 11)
     {
      MenuCounter += 1;
     }
     else
     {
      MenuCounter == 11;
     }
     wait = 1;
    }
    else  
    {
      count +=1;
      if (count > MenuSpeed)
      {
        wait = 0;
        count = 0;
      }
    }
  }
  else
  {
     if ( PB_state[1] == LOW)
    {
      wait = 0;
      count = 0;
    }
  }
  

  if (PB_state[0] == HIGH) //Button1 // Counting down
  {
    if(wait2 == 0)
    {
      if(MenuCounter > 1)
      {
       MenuCounter -= 1;
      }
      else
      {
        MenuCounter = 1;
      }
      
     wait2 = 1;
    }
    else
    {
      count2 +=1;
      if (count2 > MenuSpeed)
      {
        wait2 = 0;
        count2 = 0;
      }
    }
  }
  else
  {
    if ( PB_state[0] == LOW)
    {
      wait2 = 0;
      count2 = 0;      
    }
    
  }
//----------------------------------------//
//ValueCounter

if (PB_state[3] == HIGH)//Button4 //Counting up
  {
    if(wait3 == 0)
    {
      if(ValueCounter[MenuCounter] < ValueCounterMax[MenuCounter] )
      {
        ValueCounter[MenuCounter] += ValueCounterStep[MenuCounter];
      }
      else
      {
        ValueCounter[MenuCounter] = ValueCounterMax[MenuCounter];
      }

     wait3 = 1;
    }
    else
    {
      count3 +=1;
      if (count3 > MenuSpeed)
      {
        wait3 = 0;
        count3 = 0;
      }
    }
  }
  else
  {
     if ( PB_state[3] == LOW)
    {
      wait3 = 0;
      count3 = 0;
    }
  }
  

  if (PB_state[2] == HIGH)//Button3 //Counting down
  {
    if(wait4 == 0)
    {
      if(ValueCounter[MenuCounter] > ValueCounterMin[MenuCounter])
      {
        ValueCounter[MenuCounter] -= ValueCounterStep[MenuCounter];;
      }
      else
      {
       ValueCounter[MenuCounter] = ValueCounterMin[MenuCounter];
      }
     
     wait4 = 1;
    }
    else
    {
      count4 +=1;
      if (count4 > MenuSpeed)
      {
        wait4 = 0;
        count4 = 0;
      }
    }
  }
  else
  {
    if ( PB_state[2] == LOW)
    {
      wait4 = 0;
      count4 = 0;      
    }
    
  }


/*  
 if( PB_state[0] == HIGH)
 {
   ST_EN = 1;
 }
 else
 {
   ST_EN = 0;
 }
 
 if (PB_state[1] == HIGH)
  {
    if(wait2 == 0)
    {
      button2 += 1;
      wait2 = 1;
    }
    else
    {
      count2 +=1;
      if (count2 >25)
      {
        wait2 = 0;
        count2 = 0;
      }
    }
  }
  else
  {
    wait2 = 0;
    count2 = 0;
  }
  /*Serial.print("Button2:");
  Serial.print(button2);
  Serial.print("_____");*/
  /*
  if(button2 % 2 == 0)
  {
    button2_OLED = 1;
  }
  else
  {
    button2_OLED = 0;
  }
 /*Serial.print("B2_OLED:");
 Serial.print(button2_OLED);
 Serial.println(" ");
 */
 

}

void MenuControl()
{

 dmxStartAdress = ValueCounter[1];
 ST_EN = ValueCounter[11];
 
 if(ValueCounter[5] == 0)
 {
  MenuColor = 0;
 }
 else if(ValueCounter[5] == 1)
 {
  MenuColor = 17;
 }
 else if(ValueCounter[5] == 2)
 {
  MenuColor = 35;
 }
 else if(ValueCounter[5] == 3)
 {
  MenuColor = 54;
 }
 else if(ValueCounter[5] == 4)
 {
  MenuColor = 71;
 }
 else if(ValueCounter[5] == 5)
 {
  MenuColor = 84;
 }
 else if(ValueCounter[5] == 6)
 {
  MenuColor = 107;
 }
 else if(ValueCounter[5] == 7)
 {
  MenuColor = 125;
 }
 else if(ValueCounter[5] == 8)
 {
  MenuColor = 143;
 }
 else if(ValueCounter[5] == 9)
 {
  MenuColor = 160;
 }
 else if(ValueCounter[5] == 10)
 {
  MenuColor = 178;
 }
 else if(ValueCounter[5] == 11)
 {
  MenuColor = 195;
 }
 else if(ValueCounter[5] == 12)
 {
  MenuColor = 213;
 }
 else if(ValueCounter[5] == 13)
 {
  MenuColor = 232;
 }
 else if(ValueCounter[5] == 14)
 {
  MenuColor = 248;
 }

}


void FanControl()
{
  //fan_pwm_f = LEDTempCelsius * 5; //átlag számolás a rgb és master dimmer értékekből
     fan_pwm_out = map(LEDFanPwm1Val, 100 , 240 , 0 , 100 );  //0.3260869 * fan_pwm_f + 16.847; //f(x)=75/230x+16,847 //fügvény a kívánt tartomány eléréséhez
    
   /* FanTime = round(millis() / 1000);
    Fan32 = floor(FanTime / 32);//a Fan32 az eltelt idő és 32 lefelé kerekített hányadosa
    if(FanTime > 32);
    {
      FanTime = FanTime - 32 * Fan32;
    }

    if(FanTime > 10 && FanTime < 12)
    {
      FanT10 = fan_pwm_out;
    }
    else if(FanTime > 20 && FanTime < 22)
    {
      FanT20 = fan_pwm_out;
    }
    else if(FanTime > 29 && FanTime < 31)
    {
      FanT30 = fan_pwm_out;
    }
    
     fan_pwm_i = round((FanT10 + FanT20 + FanT30)/3);
    ledcWrite(fan_channel, fan_pwm_i );*/

   ledcWrite(fan_channel, fan_pwm_out );
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//LOOOOP0


void codeForTask1( void * parameter )
{
  
  //SETUP//SETUP//SETUP

  //FAN PWM SETUP
  ledcSetup(fan_channel, freq, resolution);
  ledcAttachPin(FAN, fan_channel);
  ledcWrite(fan_channel ,0);

  //setup for the user interface unit
  pinMode(IndR, OUTPUT);
  pinMode(IndG, OUTPUT);
  pinMode(PB[0], INPUT);
  pinMode(PB[1], INPUT);
  pinMode(PB[2], INPUT);
  pinMode(PB[3], INPUT);
  //fan control set-up
  //pinMode(FAN, OUTPUT);
  
  //relé kapcsolása
  pinMode(MainSwitch, OUTPUT);
  digitalWrite(MainSwitch, LOW);

  delay(500);
  Serial.begin(115200);
  Wire.begin();
  //CODE->for the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
   Serial.println(F("SSD1306 allocation failed"));
   for(;;);
  }
  delay(1000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  //display.setRotation(2);

  for(;;)//LOOP//LOOP//LOOP
  {
    //Power led control 
    /*
   for(int i=0;i<=2;i++)
   {
     dmxCh_f[3] = dmxCh[3];
     dmxCh_f[i] = dmxCh[i];
     led[i] = dmxCh_f[i] / 255 * dmxCh_f[3]; //A 4. DMX csatorna használata az 1-3. csatornák fényerő szabályozására.
     ledRounded[i] = round(led[i]);
     ledcWrite(ledChannel[i],ledRounded[i]); //Setting the PWM Duty Cyle on the GPIO pins.
   }
   */
    //FAN control
   MenuControl();
   FanControl();
   button_handling();//Does not contains delay!
   I2C();
   
   //stepper control//
   //Setting the limit swithces 1 if the Arduino Uno sends it over i2c that it has been pushed.
   //-----------------------------------------------------//
   if(limit1 == 0 && limit1_pushed == 0) //Limit switch 1 
   {
     limit1_pushed = 0;
   }
   else if(limit1 == 1 && limit1_pushed == 0)
   {
     limit1_pushed = 1;
   }


    if(limit2 == 0 && limit2_pushed == 0) //Limit switch 2
   {
     limit2_pushed = 0;
   }
   else if(limit2 == 1 && limit2_pushed == 0)
   {
     limit2_pushed = 1;
   }


   if(limit3 == 0 && limit3_pushed == 0) //Limit switch 3
   {
     limit3_pushed = 0;
   }
   else if(limit3 == 1 && limit3_pushed == 0)
   {
     limit3_pushed = 1;
   }


    if(limit4 == 0 && limit4_pushed == 0) //Limit switch 4
   {
     limit4_pushed = 0;
   }
   else if(limit4 == 1 && limit4_pushed == 0)
   {
     limit4_pushed = 1;
   }

   Serial.print("LimitSwitches:");
   Serial.print("\t");
   Serial.print(limit1_pushed);
   Serial.print("\t");
   Serial.print(limit2_pushed);
   Serial.print("\t");
   Serial.print(limit3_pushed);
   Serial.print("\t");
   Serial.print(limit4_pushed);
   Serial.println();
//---------------------------------------------------------------------------------------------------------//
//első kalibrálás
   if(stepper_calibrated == 0)
   {
     if(millis() > 6000)
     {

      if(limit1_pushed == 0)
      {
        stepper1_pos = -42200;
      }
      if(limit1_pushed == 1)
      {
       stepper1.setCurrentPosition(0);
       stepper1_pos = 0;
       stepper_calibrated = 1;
       stepper1.setMaxSpeed(maxSpeed2); // Set maximum speed value for the stepper
       stepper1.setAcceleration(maxAccel2); // Set acceleration value for the stepper
      }
     }
   }
//második kalibrálás
     if(stepper2_calibrated == 0 && stepper_calibrated == 1)
   {
     if(millis() > 6000)
     {

      if(limit2_pushed == 0)
      {
        stepper2_pos = -42200;
      }
      if(limit2_pushed == 1)
      {
       stepper2.setCurrentPosition(0);
       stepper2_pos = 0;
       stepper2_calibrated = 1;
       stepper2.setMaxSpeed(maxSpeed2); // Set maximum speed value for the stepper
       stepper2.setAcceleration(maxAccel2); // Set acceleration value for the stepper
      }
     }
    
   }
   
    if(stepper3_calibrated == 0 && stepper_calibrated == 1 && stepper2_calibrated == 1)
   {
     if(millis() > 6000)
     {

      if(limit3_pushed == 0)
      {
        stepper3_pos = -200000;
      }
      if(limit3_pushed == 1)
      {
       stepper3.setCurrentPosition(0);
       stepper3_pos = 0;
       stepper3_calibrated = 1;
       stepper3.setMaxSpeed(8000); // Set maximum speed value for the stepper //LENS
       stepper3.setAcceleration(4000); // Set acceleration value for the stepper
      }
     }
    
   }

    if(stepper4_calibrated == 0 && stepper3_calibrated == 1 && stepper_calibrated == 1 && stepper2_calibrated == 1)
   {
     if(millis() > 6000)
     {

      if(limit4_pushed == 0)
      {
        stepper4_pos = -10000;
      }
      if(limit4_pushed == 1)
      {
       stepper4.setCurrentPosition(0);
       stepper4_pos = 0;
       stepper4_calibrated = 1;
       stepper4.setMaxSpeed(6000); // Set maximum speed value for the stepper //COLOR WHEEL
       stepper4.setAcceleration(4000); // Set acceleration value for the stepper
      }
     }
    
   }

   if(stepper_calibrated == 1 && stepper2_calibrated == 1 && stepper3_calibrated == 1 && stepper4_calibrated == 1)
   {
    if(ValueCounter[2] == 1)//Menu:Input Select --> DMX mode
    {
      stepper1_16bit = dmxCh[4] * 255 + dmxCh[6];
      stepper2_16bit = dmxCh[5] * 255 + dmxCh[7];
      stepper1_pos = stepper1_16bit * PANmultiplier;
      stepper2_pos = stepper2_16bit * TILTmultiplier;
      stepper3_pos = dmxCh[2]*261; //adress-3 //lens stepper   //Maximum position in 1/16 step mode is 135000 -> 135000/255=529
      stepper4_pos = dmxCh[3]*6; //adress-4 //color wheel stepper //Maximum position (one full rotation of the wheel) in 1/16 step mode is 3120   -> 3120/255=12,13
      LEDBrightness = dmxCh[1]; //adress-2 //The LEDBrightness value is sent over to the MySpotV4 board where it will control the 150W LED light source.
    }
    else if(ValueCounter[2] == 2)  //Menu:Input Select --> Manual Mode
    {
      stepper1_16bit = ValueCounter[6] * 255 + 0;
      stepper2_16bit = ValueCounter[7] *255 + 0;
      stepper1_pos = stepper1_16bit * PANmultiplier;
      stepper2_pos = stepper2_16bit * TILTmultiplier;
      stepper3_pos = ValueCounter[4] * 261; //adress-3 //lens stepper   //Maximum position in 1/16 step mode is 135000 -> 135000/255=529
      stepper4_pos = MenuColor * 6; //adress-4 //color wheel stepper //Maximum position (one full rotation of the wheel) in 1/16 step mode is 3120   -> 3120/255=12,13
      LEDBrightness = ValueCounter[3]; //adress-2 //The LEDBrightness value is sent over to the MySpotV4 board where it will control the 150W LED light source.
    }
    
   }
   //-------------------------------------------------------------------------------------------------------------------------------------------//
   //Serial.println(stepper1_16bit);
   //Serial.print("   ");
   //Serial.print(stepper1_pos);
   //Serial.print("   ");
   //Serial.print(dmxCh[6]);
   
   OLED_writing();
   //Serial.printf("Cs1: %d  Cs2: %d  Cs3: %d  Cs4: %d  \n",dmxCh[0],dmxCh[1],dmxCh[2],dmxCh[3]);
   if(millis() > 4000 && millis() < 6000)//relé bekapcsolás időzítése
   {
     digitalWrite(MainSwitch, HIGH);
   }



   if(dmxReady == 1 && stepper4_calibrated == 1)
   {
     digitalWrite(IndG,1);
     digitalWrite(IndR,0);
   }
   else
   {
     digitalWrite(IndG,0);
      if (millis() / 1000 % 2 == 0 )
      {
       digitalWrite(IndR,1);
      } 
      else 
      {
       digitalWrite(IndR,0);
      }
   }
   vTaskDelay(1);
  }
}
  

//-------------------------------------------------------------------------

void setup()
{
  
  xTaskCreatePinnedToCore(
    codeForTask1,            /* Task function. */
    "Task_1",                 /* name of task. */
    4000,                    /* Stack size of task */
    NULL,                     /* parameter of the task */
    1,                        /* priority of the task */
    &Task1,                   /* Task handle to keep track of created task */
    0);                       /* Core */

  
  delay(100);
  //setup for the stepper drivers
  
   stepper1.setMaxSpeed(maxSpeed); // Set maximum speed value for the stepper
   stepper1.setAcceleration(maxAccel); // Set acceleration value for the stepper

   stepper2.setMaxSpeed(maxSpeed); // Set maximum speed value for the stepper
   stepper2.setAcceleration(maxAccel); // Set acceleration value for the stepper

   stepper3.setMaxSpeed(3000); // Set maximum speed value for the stepper   //LENS
   stepper3.setAcceleration(2000); // Set acceleration value for the stepper

   stepper4.setMaxSpeed(250); // Set maximum speed value for the stepper //COLOR WHEEL
   stepper4.setAcceleration(200); // Set acceleration value for the stepper

   stepper1.setCurrentPosition(0);
   stepper2.setCurrentPosition(0);
   stepper3.setCurrentPosition(0);
   stepper4.setCurrentPosition(0);

}
void loop()
{
  
   

   stepper1.moveTo(stepper1_pos);
   stepper2.moveTo(-stepper2_pos);
   stepper3.moveTo(stepper3_pos);
   stepper4.moveTo(stepper4_pos);
   stepper1.run();
   stepper2.run();
   stepper3.run();
   stepper4.run();
 
}