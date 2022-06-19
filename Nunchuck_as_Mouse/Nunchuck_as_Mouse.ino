/*
*  Project     Nunchuk as a mouse
*  @author     Alvaro Alea Fernandez
*  @link       github.com/alvaroalea/????
*  @license    LGPLv3 - Copyright (c) 2018 David Madison
*
*  This file create a USB HID mouse that is managed by joy and keys on nintendo's nunchuk
*  is to help people with reduced movement to use a android mobile
*  
*  Z button is "touch" in screen, or left click.
*  C button (hold for about 2 second) is "back" or right click 
*  C and before 2 seconds press Z is "home" o click in the middle button (wheel)
*  
*  */

#include <NintendoExtensionCtrl.h>
#include "Mouse.h"
Nunchuk nchuk;

//#define DEBUG
#ifdef DEBUG
#define DEBUGPRINT(par) Serial.print(par)
#define DEBUGPRINTLN(par) Serial.println(par)
#else
#define DEBUGPRINT 
#define DEBUGPRINTLN 
#endif

// parameters for reading the joystick:
#define RANGE  32
#define CENTER 16
#define STEP1  10

#define threshold 3    
               
#define responseDelay 10        
#define CDELAY 200

int ac1x=3,ac1y,ac2x=6,ac2y,ac3x,ac3y;
int inx[]={32,65,122,164,192}; //22,122,222
int iny[]={32,64,125,164,192}; //16,125,225

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

void autocal(int x, int *in, int *ac1,int *ac2){
//this calibrate minumun and maximun values of in table,hold for about 2second in this position and new value is catch if over the previous.
DEBUGPRINT("Autocal x=");
DEBUGPRINT(x);

DEBUGPRINT(" in[]=");
DEBUGPRINT(in[0]); DEBUGPRINT (", ");
DEBUGPRINT(in[4]); DEBUGPRINT (" ");

DEBUGPRINT("AC1=");
DEBUGPRINT(*ac1);
DEBUGPRINT(" AC2=");
DEBUGPRINT(*ac2);

  if (x>=in[0]){
    *ac1=0;
    DEBUGPRINT("+");
  } else {
    *ac1= *ac1 +1;
    if (*ac1>200){
      *ac1=0;
      in[0]=x;
      in[1]= ((in[2]-in[0])/2)+in[0];
    }
  }
  if (x<=in[4]){
    *ac2=0;
  } else {
    *ac2 = *ac2 +1;
    if (*ac2>200){
      *ac2=0;
      in[4]=x;
      in[3]= ((in[4]-in[2])/2)+in[2];
    }
  }
DEBUGPRINTLN("");
}

int mouseCalc(int value,int * in) {
//calculate offser of mouse movement, based on read value and input table
  int out[] = { 0,STEP1 ,CENTER , (RANGE-STEP1) ,RANGE };
  int reading = multiMap(value, in, out, 5);
  int distance = reading - CENTER;
  if (abs(distance) < threshold) {
    distance = 0;
  }
  return distance;   // return the distance for this axis
}

//=====================================================================================

void setup() {
#ifdef DEBUG
	Serial.begin(115200);
#endif

	nchuk.begin();

	while (!nchuk.connect()) {
		DEBUGPRINTLN("Nunchuk not detected!");
		delay(1000);
	}
  Mouse.begin();
  DEBUGPRINTLN("----- Nunchuk Demo -----"); // Making things easier to read
  boolean success = nchuk.update();
  inx[2] = nchuk.joyX();
  iny[2] = nchuk.joyY();
}


void loop() {
  static bool bc=0;
  static bool bz=0;
  static int ccount=0;
	
	boolean success = nchuk.update();  // Get new data from the controller
	if (!success) {  // Ruh roh
		DEBUGPRINTLN("Controller disconnected!");
		delay(1000);
	}
	else {
		// Read a button (on/off, C and Z)
		boolean zButton = nchuk.buttonZ();   
    boolean cButton = nchuk.buttonC();
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
    autocal(x, inx, &ac1x,&ac2x);
    int xReading = mouseCalc(x,inx);
    
    int y = nchuk.joyY();
    autocal(y, iny, &ac1y,&ac2y);
    int yReading = mouseCalc(y,iny);
    
    if ((xReading!=0) or (yReading!=0)) {
      Mouse.move(xReading, -yReading, 0);
      delay(responseDelay);
    }
		// Print all the values!
		//nchuk.printDebug();
	} // IF nunchuk conected.
}
