// (C) 2019 Alvaro Alea Fernandez <alvaroalea AT gmail DOT com>
// Under GPL Licence
/*************************************
 * LAMPARA PARA ARDUINO CON 16 NEOPIXELS
 * 
 * 3 modos de funcionamiento:
 * A) Simula Fuego, con el encoder regulamos la intensidad del fuego
 * B) Lampara Blanca, con el encoder regulamos la intensidad de la luz
 * C) Alien Cristal, con el encoder regulamos el tono de la luz
 * 
 * Con el pulsador del encoder encendemos o apagamos la luz
 * Con un doble-click cambiamos entre modos.
 */
#include <Adafruit_NeoPixel.h>

#define BUTTON_PIN  A0
#define ENC_A        9 
#define ENC_B       10
bool BoldState = HIGH;
bool EoldState = HIGH;
#define DOCLICKTIMEOUT 20000

#define PIXEL_PIN   11 
#define PIXEL_COUNT 16
Adafruit_NeoPixel ring = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// DEFINICIONES PARA EL MODO 1 - Fuego

#define FIRE_HEIGHT 8
#define FIRE_DIV 6.2
#define FIRE_DELAY 12000

int showType = 0;

const int palette[32][3]={ 
  {0,0,0},
  
  {0x07,0,0},
  {0x0E,0,0},
  {0x15,0,0},
  
  {0x21,0,0},
  {0x37,0,0},
  {0x4D,0,0},
  {0x63,0,0},
  
  {0x85,0,0},
  {0xA6,0,0},
  {0xD2,0,0},
  {0xFF,0,0},
  
  {0xFF,25,0},
  {0xFF,50,0},
  {0xFF,75,0},
  {0xFF,100,0},
  {0xFF,125,0},
  {0xFF,150,0},
  {0xFF,175,0},
  {0xFF,200,0},
  {0xFF,225,0},
  {0xFF,0xFF,0x00},
  
  {0xEE,0xEE,0x19},
  {0xDD,0xDD,0x33},
  {0xCC,0xCC,0x3c},
  {0xBB,0xBB,0x66},
  {0xAB,0xAB,0x7F},
  {0x9A,0x9A,0x99},
  {0x89,0x89,0xB2},
  {0x78,0x78,0xCC},
  {0x67,0x67,0xE5},
  {0x57,0x57,255}
};

//vamos a terner 2 lineas de fuego, cada una de 8 pixel, y otras dos lineas "fantasma" por randomizar el efecto
// la primera linea el seed, nunca se muestra, esto permite evitar los picos aleatorios.
int fire[FIRE_HEIGHT+1][4]={
  {31,31,31,31},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0}
};

// DEFINICIONES PARA EL MODO 2 - LUZ REGULABLE
int brillo=255;

// DEFINICIONES PARA EL MODO 3 - LATIDO
#define BEATSTEPS 6
#define STEP_DELAY 7500
int beats1[]={0,  128, 255, 128,  0, 128};
int beats2[]={128,127,-127,-128,128,-128};
int hue=0;
int bbstep=0;
int bsstep=0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ENC_A, INPUT);
  pinMode(ENC_B, INPUT);
//  Serial.begin(9600);
//  while (!Serial) {  }
  ring.begin();
  ring.show(); 
}

int modo=2;
int encendido=1;
int doclickcount=0;
int counter=0;
float firediv= FIRE_DIV;

void loop() {
// Boton de encendido
  bool newState = digitalRead(BUTTON_PIN);
  if (newState == LOW && BoldState == HIGH) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON_PIN);
    if (newState == LOW) {
    encendido=!encendido;
    }
  }
  BoldState = newState;

// detectamos un doble click 
// si volvimos a encender antes del timeout cambiamos de modo.  
if ((encendido==1) and (doclickcount>0) and (doclickcount<DOCLICKTIMEOUT)){
  modo++;
  if (modo>2) modo=0;
}
if (encendido==1) doclickcount=0;
if ((encendido==0) and (doclickcount<DOCLICKTIMEOUT)) doclickcount++;

// Encoder
  newState = digitalRead(ENC_A);
  if ((newState == LOW) && (EoldState == HIGH) && (encendido==1)) {
    if (digitalRead(ENC_B) == LOW) {
      //  Movemos sentido reloj
      switch (modo){
         case 0:
          firediv=firediv + 0.1;
          if (firediv>15) firediv= 15;
          break;
        case 1:
          brillo=brillo -5;
          if (brillo<5) brillo =5;
          break;          
       case 2:
          hue=hue -3;
          if (hue<=0) hue =360;
          break;          
      }
    } else {
      // Movemos sentido contrario reloj 
      switch (modo){
         case 0:
           firediv=firediv - 0.1;
           if (firediv<1) firediv= 1;
          break;
        case 1:
          brillo=brillo +5;
          if (brillo>255) brillo =255;
          break;          
        case 2:
          hue=hue +3;
          if (hue>=360) hue =0;
          break;          
      }
    }
  }
  EoldState = newState;

// Actualizamos una de cada n veces los led.
  counter++;
  if ((modo==0) and (counter==FIRE_DELAY)){
    counter =0;
    fuego();
  }
  if ((modo==1) and (counter==FIRE_DELAY)){
    counter =0;
    luz();
  }
  if ((modo==2) and (counter==STEP_DELAY)){
    counter =0;
    latido();
  }
}

void fuego(void){
// paso 1 añadimos semillas al fondo o apagamos.
  int c;
     for(c=0;c<4;c++){
       fire[0][c]=(encendido?random(32):0); 
     }

// paso 2 recalculamos los colores
  int d,a1,a2,a3,a4,b;
  for(c=FIRE_HEIGHT;c>0;c--){ // no se recalcula el ultimo, ya que es la semilla
    for(d=0;d<4;d++){
     a1=fire[c>1?c-1:0][d>1?d-1:3];     
     a2=fire[c>1?c-1:0][d];
     a3=fire[c>1?c-1:0][d>3?d+1:0];
     a4=fire[c>2?c-2:0][d];
     
     b=(a1+a2+a3+a4)/firediv;
     if (b>31) b=31;
     fire[c][d]=(int)b;                 
    }
  }
     
// paso 3 los mostramos
  int color_idx;
  for(c=0;c<FIRE_HEIGHT;c++){
    color_idx=fire[c+1][0];
    ring.setPixelColor(c,ring.Color(palette[color_idx][0],palette[color_idx][1],palette[color_idx][2]));
  }
  for(c=0;c<FIRE_HEIGHT;c++){
    color_idx=fire[c+1][2];
    ring.setPixelColor(8+c,ring.Color(palette[color_idx][0],palette[color_idx][1],palette[color_idx][2]));
  }
  ring.show();
}

void luz(void){
  int c;
  for(c=0;c<PIXEL_COUNT;c++){
    ring.setPixelColor(c,brillo*encendido,brillo*encendido,brillo*encendido);
  }
  ring.show();
}


struct irgb{
    int r;       // a integer between 0 and 255
    int g;       // a integer between 0 and 255
    int b;       // a integer between 0 and 255
};

//static irgb hsv2rgb(int h, int s, int v);
struct irgb hsv2rgb(int h, int s, int v){
    struct irgb out;
    float      hh, p, q, t, ff,ss,vv;
    long        i;
    int         p1,q1,t1;

    vv=v/255.0;
    ss=s/255.0;

    if(ss <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = out.g = out.b = v;
        return out;
    }
    
    hh = h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;

    p = vv * (1.0 -  ss);
    q = vv * (1.0 - (ss * ff));
    t = vv * (1.0 - (ss * (1.0 - ff)));

    p1=round(255*p);
    q1=round(255*q);
    t1=round(255*t);

    switch(i) {
    case 0:
        out.r = v;
        out.g = t1;
        out.b = p1;
        break;
    case 1:
        out.r = q1;
        out.g = v;
        out.b = p1;
        break;
    case 2:
        out.r = p1;
        out.g = v;
        out.b = t1;
        break;

    case 3:
        out.r = p1;
        out.g = q1;
        out.b = v;
        break;
    case 4:
        out.r = t1;
        out.g = p1;
        out.b = v;
        break;
    case 5:
    default:
        out.r = v;
        out.g = p1;
        out.b = q1;
        break;
    }
    return out;     
}

void latido(void){
  int c,v;
  struct irgb col;
  bsstep=bsstep+1;
  if (bsstep>=10){
    bsstep=0;
    bbstep=bbstep+1;
    if (bbstep>=BEATSTEPS){
      bbstep=0;
    }
  }
  v=beats1[bbstep]+beats2[bbstep]*(bsstep/10.0);
  col = hsv2rgb(hue,255,v*encendido);
  for(c=0;c<PIXEL_COUNT;c++){
    ring.setPixelColor(c,col.r,col.g,col.b);
  }
  ring.show();
}
