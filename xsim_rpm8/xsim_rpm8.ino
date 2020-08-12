// Modifications (C) Alvaro Alea <alvaroalea@gmail.com> under GPL3 were applicable.

// Very high modify from Original by:
// X-sim3dashboard v1  (using TM1638 display and Arduino Nano v3)
// Made by TronicGr (Thanos) 4-26-2012 for X-sim3
// Shared as Public Domain

//on simhub you need to define a custom serial and use:
/*
'R'+truncate([Rpms])+','+
'S'+truncate([SpeedKmh])+','+
'G'+[Gear]+','+
'C'+truncate([DataCorePlugin.Computed.Fuel_Percent])+','
*/

#define RELEASE "REL  04A"

#include <EEPROM.h> //Needed to access the eeprom read write functions
#include <TM1638.h> //can be downloaded from https://github.com/rjbatista/tm1638-library
#include <MatrizLed.h> // it's in arduino library manager.
TM1638 module(8, 7, 9);
MatrizLed matrix;

unsigned int rpm;                  //holds the rpm data (0-65535 size)
unsigned int carspeed;             //holds the speed data (0-65535 size)
unsigned int fuel;
byte gear;                         // holds gear value data
unsigned int rpmmin;
unsigned int rpmmax;
byte mode = 0;
boolean rpmdata = false;                  
boolean speeddata = false; 
boolean geardata = false;
boolean fueldata = false;               
boolean hay = false;

word ledcfg[]={
       ( 0b00000000 | 0b00000000 <<8),
       ( 0b00000000 | 0b00011000 <<8),
       ( 0b00000000 | 0b00011000 <<8),
       ( 0b00000000 | 0b00111100 <<8),
       ( 0b00000000 | 0b00111100 <<8),
       ( 0b01000010 | 0b00111100 <<8),
       ( 0b01000010 | 0b00111100 <<8),
       ( 0b11000011 | 0b00111100 <<8),
       ( 0b11000011 | 0b00111100 <<8),
       ( 0b11111111 | 0b00000000 <<8),
       ( 0b11111111 | 0b00000000 <<8),

       ( 0b00000000 | 0b00000000 <<8),
       ( 0b00000000 | 0b00000001 <<8),
       ( 0b00000000 | 0b00000011 <<8),
       ( 0b00000000 | 0b00000111 <<8),
       ( 0b00000000 | 0b00001111 <<8),
       ( 0b00000000 | 0b00011111 <<8),
       ( 0b00100000 | 0b00011111 <<8),
       ( 0b01100000 | 0b00011111 <<8),
       ( 0b11100000 | 0b00011111 <<8), 
       ( 0b11111111 | 0b00000000 <<8),
       ( 0b11111111 | 0b00000000 <<8)};

char texto[9]="        ";
String textoo ="        ";
int menupos=0;

void mode_f1(){
  rpmmin=14500;
  rpmmax=17000;
  mode =1;
  module.setDisplayToString("TYPE  F1");    //prints the banner
}

void mode_gt(){
  rpmmin=0;
  rpmmax=9000;
  mode = 0;
  module.setDisplayToString("TYPE  GT");    //prints the banner
}

void mode_cu(){
  rpmmin=((EEPROM.read(0) << 0) & 0xFF) + ((EEPROM.read(1) << 8) & 0xFF00);
  rpmmax=((EEPROM.read(2) << 0) & 0xFF) + ((EEPROM.read(3) << 8) & 0xFF00);
  mode = EEPROM.read(4);
  module.setDisplayToString("TYPE  CU");    //prints the banner
}

void save_rpm(){
  int b=0;
  module.setDisplayToString("Y SAVE N");    //prints the banner
  while ((b!=0b00000001) && (b!=0b10000000)) {
    b = module.getButtons();
    if (b==0b00000001) {
      EEPROM.write(0, ((rpmmin >> 0) & 0xFF));
      EEPROM.write(1, ((rpmmin >> 8) & 0xFF));
      EEPROM.write(2, ((rpmmax >> 0) & 0xFF));
      EEPROM.write(3, ((rpmmax >> 8) & 0xFF));
      EEPROM.write(4,mode);
      module.setDisplayToString("SAVED   ");    //prints the banner
    }
  }
}

void inc_min(){
  rpmmin+=500;
  if (rpmmin>30000) {
    rpmmin=30000;
  }
  module.setDisplayToDecNumber(rpmmin,0,false);
  module.setDisplayToString("LO");    //prints the banner
}

void dec_min(){
  rpmmin-=500;
  if ((rpmmin<0) || (rpmmin>30000)) {
    rpmmin=0;
  }
  module.setDisplayToDecNumber(rpmmin,0,false);
  module.setDisplayToString("LO");    //prints the banner
}

void inc_max(){
  rpmmax+=500;
  if (rpmmax>30000) {
    rpmmax=30000;
  }
  module.setDisplayToDecNumber(rpmmax,0,false);
  module.setDisplayToString("hi");    //prints the banner
}

void dec_max(){
  rpmmax-=500;
  if ((rpmmax<0)||(rpmmax>30000)) {
    rpmmax=0;
  }
  module.setDisplayToDecNumber(rpmmax,0,false);
  module.setDisplayToString("hi");    //prints the banner
}

void readkeys(){
  byte b,c=0;
  b = module.getButtons();
  switch (b){
  case 0b00000001:
    mode_f1();
    c=1;
    break;
  case 0b00000010:
    mode_gt();
    c=1;
    break;
  case 0b00000100:
    mode_cu();
    c=1;
    break;
  case 0b00001000:
    save_rpm();
    c=1;
    break;
  case 0b00010000:
    dec_min();
    break;
  case 0b00100000:
    inc_min();
    break;
  case 0b01000000:
    dec_max();
    break;
  case 0b10000000:
    inc_max();
    break;
  }
  if (c==1) {
    while (module.getButtons()!=0) {delay(1);}
  }
}

void setup() {
  int i;
  Serial.begin(115200);
  matrix.begin(8, 7, 10, 1); // dataPin, clkPin, csPin, numero de matrices de 8x8
  module.clearDisplay(); //clears the display from garbage if any
  matrix.borrar();
  
  module.setDisplayToString("ALEAsoft");    //prints the banner
  for(i=7;i!=0;i--){
    module.setupDisplay(true,i);
    matrix.escribirCifra(i);
    delay(150);                        //small fadeoff about 1.5 sec
  }
  module.clearDisplay();              //clears the display from garbage if any
  matrix.borrar();
  delay(200);
  module.setDisplayToString(RELEASE);    //prints the banner
  for(i=7;i!=0;i--){
    module.setupDisplay(true,i);
    delay(150);                        //small fadeoff about 1.5 sec
  }
  module.clearDisplay();              //clears the display
  module.setupDisplay(true,2);

  rpmmin=((EEPROM.read(0) << 0) & 0xFF) + ((EEPROM.read(1) << 8) & 0xFF00);
  rpmmax=((EEPROM.read(2) << 0) & 0xFF) + ((EEPROM.read(3) << 8) & 0xFF00);
  mode=EEPROM.read(4);
  if ( (rpmmin==0) && (rpmmax=9000) && (mode==0) ) {
    module.setDisplayToString("TYPE  GT");    //prints the banner
  } else if ( (rpmmin==14500) && (rpmmax=17000) && (mode==1) ) {
    module.setDisplayToString("TYPE  F1");    //prints the banner
  } else {
    module.setDisplayToString("TYPE  CU");    //prints the banner
  }
  delay(750);                        //small fadeoff about 1.5 sec

  while (!Serial) ; //esperamos la inicializacion del serie-usb en el leonardo, despues de mostrar los banner.
  module.clearDisplay();              //clears the display from garbage if any  
}

void processdata(){
 char s[9];
 unsigned int rpmleds;              //holds the 8 leds values
  
 if ((speeddata ==true) || (fueldata == true)) {
   sprintf(s, "%02i   %3i", fuel % 99, carspeed);
   module.setDisplayToString(s);
//   if (carspeed) {
//      module.setDisplayDigit(0,7,0); // to see speed=0
//   } else {
//      module.setDisplayToDecNumber(carspeed, 0, false);  //displays numerical the speed
//   }
   fueldata=false;                     
   speeddata=false;                     
 }
 
 if (geardata == true) {
   matrix.escribirCaracter(gear,0);
   geardata=false;                     
 }

 if (rpmdata == true) {   
   if (rpm>rpmmax) rpm=rpmmax;
   if (rpm<rpmmin) rpm=rpmmin;
   rpmleds = (mode==1?11:0) + map(rpm,rpmmin,rpmmax,0,10);    // distributes the rpm level to the 8 leds + 1 for shift change
   module.setLEDs(ledcfg[rpmleds]);
   if ((rpmleds == 9)||(rpmleds == 10)||(rpmleds == 20)||(rpmleds == 21)) {
        module.setupDisplay(true,7);
        matrix.setIntensidad(15);
     } else {
        module.setupDisplay(true,2);
        matrix.setIntensidad(2);
   }
   rpmdata=false;
 }
}

void serialEvent2(void){
  static unsigned int oldfuel=99999;                  // Previous Lecture
  static unsigned int oldrpm=99999;                  
  static unsigned int oldcarspeed=999;             
  static byte oldgear=99;                        
  int c;
  while (Serial.available()!=0){
    c=Serial.read();
    switch (c){
      case 'R':
        rpm=Serial.parseInt();
        if (rpm != oldrpm){
          rpmdata=true;                          // we got new data!
          oldrpm=rpm;
        }
        hay=true;      
        break;
      case 'S':
        carspeed=Serial.parseInt();
        if (carspeed!= oldcarspeed){
          speeddata=true;                        // we got new data!
          oldcarspeed=carspeed;
        hay=true;      
        }
        break;
      case 'C':
        fuel=Serial.parseInt();
        if (fuel!= oldfuel){
          fueldata=true;                        // we got new data!
          oldfuel=fuel;
        hay=true;      
        }
        break;
      case 'G':
        gear=Serial.read();
        if (gear != oldgear){
          geardata=true;                        // we got new data!
          oldgear=gear;
        hay=true;      
        }
        break;   
    }
  }
}

void loop() {
  static int ck=0; 
  while ((Serial) && (Serial.available()>=2) ) serialEvent2();
  if (hay==true) {      
   processdata();
   hay = false; // reseteamos las estaticas.
  }
  ck++;  // solo leemos las teclas una de cada 5 veces, es un proceso lento.
  if (ck>=5) {
     readkeys(); 
     ck=0;
  }  
}
