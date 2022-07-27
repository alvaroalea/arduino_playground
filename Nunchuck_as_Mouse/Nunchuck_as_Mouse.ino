 /*
*  Project     Nunchuk as a mouse
*  @author     Alvaro Alea Fernandez
*  @link       https://github.com/alvaroalea/arduino_playground/tree/master/Nunchuck_as_Mouse
*  @license    LGPLv3 - Copyright (c) 2022 Alvaro Alea
*
*  This file create a USB HID or bluetooth mouse that is managed by joy and keys on nintendo's nunchuk
*  is to help people with reduced movement to use a android mobile
*  
*  Z button is "touch" in screen, or left click.
*  C button (hold for about 2 second) is "back" or right click 
*  C and before 2 seconds press Z is "home" o click in the middle button (wheel)
*  */

// Enable this for use as bluetooth with a ESP32
// need this library: https://github.com/T-vK/ESP32-BLE-Mouse
#define USE_BLUETOOTH

#ifdef USE_BLUETOOTH
#include <BleConnectionStatus.h>
#include <BleMouse.h>
BleMouse Mouse;
//#define ONBOARD_LED  2
//#define I2C_SDA 7
//#define I2C_SCL 8
#define ONBOARD_LED  3
#define I2C_SDA 27
#define I2C_SCL 26

#else 
#include "Mouse.h"
#define ONBOARD_LED  13
#endif

//ESP32 C3
#define ONBOARD_LED 18
#define ONBOARD_RLED 3
#define ONBOARD_GLED 4
#define ONBOARD_BLED 5
#define LED_W_BUILTIN 19
#define LED_O_BUILTIN 18
//#define HAVE_DUAL_LED


#include <NintendoExtensionCtrl.h>
Nunchuk nchuk;


//#define DEBUG
#ifdef DEBUG
//#define AUTOCAL_DEBUG
#define DEBUGCOORD
#define DEBUGPRINT(par) Serial.print(par)
#define DEBUGPRINTLN(par) Serial.println(par)
#else
#define DEBUGPRINT 
#define DEBUGPRINTLN 
#endif

// parameters for reading the joystick:
#define STEP1  10
#define CENTER 16
#define RANGE  32
#define ZSTEP1 5
#define threshold 2    
               
//#define responseDelay 15        
#define tstep 4
#define CDELAY 200
#define LEDPERIOD 20
#define SWAPGRAD 600

int ac1x=3,ac1y,ac2x=6,ac2y,ac3x,ac3y;
int inx[]={1,32,131,223,254}; //22,122,222
int iny[]={0,32,128,223,255}; //16,125,225
int out[] = { -threshold,STEP1 ,CENTER , (RANGE-STEP1) ,RANGE+threshold };
int tin[]= {1,  8,16,32};
int tout[]={25,20,15,10};

bool needcalx=0,needcaly=0;
int calibrated=0;

/* FROM: https://playground.arduino.cc/Main/MultiMap/ */
// note: the _in array should have increasing values
int multiMap(int val, int* _in, int* _out, uint8_t size)
{  
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= _in[0]) return _out[0]; // take care the value is within range
  if (val >= _in[size-1]) return _out[size-1];
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > _in[pos]) pos++; // search right interval
  if (val == _in[pos]) return _out[pos]; // this will handle all exact "points" in the _in array
  // interpolate in the right segment for the rest
  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
} 

bool autocal(int x, int *in, int *ac1,int *ac2, bool needcal){
//this calibrate minumun and maximun values of in table,hold for about 2second in this position and new value is catch if over the previous.
#ifdef AUTOCAL_DEBUG
DEBUGPRINT("Autocal x=");
DEBUGPRINT(x);

DEBUGPRINT(" in[]=");
DEBUGPRINT(in[0]); DEBUGPRINT (", ");
DEBUGPRINT(in[4]); DEBUGPRINT (" ");

DEBUGPRINT("AC1=");
DEBUGPRINT(*ac1);
DEBUGPRINT(" AC2=");
DEBUGPRINT(*ac2);
#endif
  if (x>=in[0]){
    *ac1=0;
  } else {
    needcal|=1;
    *ac1= *ac1 +1;
    if (*ac1>200){
      *ac1=0;
      in[0]=x;
      in[1]= ((in[2]-in[0])/ZSTEP1)+in[0];
      calibrated=200;
      needcal=0;
    }
  }
  if (x<=in[4]){
    *ac2=0;
  } else {
    needcal|=1;
    *ac2 = *ac2 +1;
    if (*ac2>200){
      *ac2=0;
      in[4]=x;
      in[3]= ((in[4]-in[2])/ZSTEP1)+in[2];
      calibrated=200;
      needcal=0;
    }
  }
//DEBUGPRINTLN("");
return needcal;
}

int mouseCalc(int value,int * in) {
//calculate offser of mouse movement, based on read value and input table
#ifdef DEBUGCOORD    
DEBUGPRINTLN("");
DEBUGPRINT("va=");
DEBUGPRINT(value);
#endif  
int reading = multiMap(value, in, out, ZSTEP1);
#ifdef DEBUGCOORD    
DEBUGPRINT(" re=");
DEBUGPRINT(reading);
#endif
  int distance = reading - CENTER;
#ifdef DEBUGCOORD    
DEBUGPRINT(" di=");
DEBUGPRINT(distance);
#endif
  if (abs(distance) <= threshold) {
    distance = 0;
  } else if (distance <0) {
    distance = distance + threshold;
  } else {
    distance = distance - threshold;
  }
#ifdef DEBUGCOORD    
DEBUGPRINT(" di2=");
DEBUGPRINT(distance);
#endif
  return distance;   // return the distance for this axis
}

//=====================================================================================

void setup() {
#ifdef  HAVE_DUAL_LED
  pinMode(LED_W_BUILTIN,OUTPUT);
  pinMode(LED_O_BUILTIN,OUTPUT);
  digitalWrite(LED_O_BUILTIN,HIGH);
#endif

  Wire.begin(I2C_SDA, I2C_SCL); //esto fuerza el uso de patillas no-standar

#ifdef DEBUG
	Serial.begin(115200);
#endif
  Mouse.begin();
	nchuk.begin();

	while (!nchuk.connect()) {
		DEBUGPRINTLN("Nunchuk not detected!");
		delay(1000);
	}
#ifdef  HAVE_DUAL_LED
  digitalWrite(LED_O_BUILTIN,LOW);
  digitalWrite(LED_W_BUILTIN,HIGH);
#endif
  DEBUGPRINTLN("----- Nunchuk Demo -----"); // Making things easier to read
//  digitalWrite(3,LOW);
  boolean success = nchuk.update();
  inx[2] = nchuk.joyX();
  iny[2] = nchuk.joyY();
}


void loop() {
  static bool bc=0;
  static bool bz=0;
  static int ccount=0;
  static int blink=0;
  int t;
	
	boolean success = nchuk.update();  // Get new data from the controller
	if (!success) {  // Ruh roh
		DEBUGPRINTLN("Controller disconnected!");
		delay(1000);
	}
	else {
		// Read a button (on/off, C and Z)
		boolean zButton = nchuk.buttonZ();   
    boolean cButton = nchuk.buttonC();
    if ((cButton == 0) and ( ccount>0)) {
      Mouse.click(MOUSE_RIGHT);
      ccount = 0;
    }
    if (cButton!=bc) {
      if (cButton) {
        ccount++;
        if (ccount>CDELAY) {
          bc=cButton;
          ccount=0;  
          Mouse.press(MOUSE_RIGHT);
        }
      } else {
        bc=cButton;
        ccount=0;  
        Mouse.release(MOUSE_RIGHT); 
      }
    }
    if ((ccount>0) and zButton){
      Mouse.click(MOUSE_MIDDLE);
      bz=zButton;
      bc=cButton;
      ccount=0;
    } 

    if (zButton!=bz) {
      if (zButton) {
        Mouse.press(MOUSE_LEFT);
      } else {
        Mouse.release(MOUSE_LEFT); 
      }
      bz=zButton;
    }

    int x = nchuk.joyX();
    needcalx=autocal(x, inx, &ac1x,&ac2x,needcalx);
    int xReading = mouseCalc(x,inx);

    int y = nchuk.joyY();
    needcaly=autocal(y, iny, &ac1y,&ac2y,needcaly);
    int yReading = mouseCalc(y,iny);

    // Read an accelerometer and print values (0-1023, X, Y, and Z)
    int accelX = nchuk.accelX();
    if (accelX<SWAPGRAD){
      yReading = -xReading;
      xReading = 0;
    }

    int maxt = abs(xReading)>abs(yReading)?abs(xReading):abs(yReading);
   
    if ((xReading!=0) or (yReading!=0)) {
      Mouse.move(xReading, -yReading, 0);
      delay(multiMap(maxt,tin,tout,tstep));
    }
#ifdef DEBUGCOORD    
DEBUGPRINTLN("");
DEBUGPRINT("X=");
DEBUGPRINT(xReading);
DEBUGPRINT(" Y=");
DEBUGPRINT(yReading);
DEBUGPRINT(" d=");
DEBUGPRINT(maxt);
DEBUGPRINT("->");
DEBUGPRINTLN(multiMap(maxt,tin,tout,tstep));
#endif
		// Print all the values!
		//nchuk.printDebug();
	} // IF nunchuk conected.
  if (calibrated>0) {
    digitalWrite(ONBOARD_LED,HIGH);
    calibrated--;
    if (calibrated==0) {
      digitalWrite(ONBOARD_LED,LOW);
    }
  } else if (needcalx || needcaly) {
    blink++;
    if (blink>2*LEDPERIOD) blink=0;
    if (blink>LEDPERIOD) {
      digitalWrite(ONBOARD_LED,HIGH);
     } else {
      digitalWrite(ONBOARD_LED,LOW);
      }
  }
  #ifdef DEBUG
      nchuk.printDebug(); 
  #endif
}
