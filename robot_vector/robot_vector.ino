/* Robot con personalidad para ESP8266
 * Desarrollado por Alvaro Alea Fernandez *
 * Distribuido con licencia GPL V2        *
 * (C) 2019                               */
 
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>
const char* ssid = "yellowice";
const char* password = "yellowlife";

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include "eyes.h"


void setup() {
  // put your setup code here, to run once:
  pinMode(0,INPUT_PULLUP);
  pinMode(12,INPUT_PULLUP);
  pinMode(13,INPUT_PULLUP);
  pinMode(14,INPUT_PULLUP);
  
  Serial.begin(115200);
  Serial.println("Booting");

  Wire.begin(5, 4);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  
  SPIFFS.begin();
//  SPIFFS.format();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
/*  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  } */
  WiFi.waitForConnectResult();

 
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
 
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Clear the buffer
  display.clearDisplay();
  display.display();
}

unsigned char readkey(void){ 
  unsigned char ret=0;
    if (digitalRead(0)==0 ) ret=1;  //izquierda
    if (digitalRead(12)==0) ret+=2; //arriba
    if (digitalRead(13)==0) ret+=4; //abajo
    if (digitalRead(14)==0) ret+=8; //fuego
  return (ret);
}

int xp=16;
int mood=1;

void loop() {
  int n;
  static int xd=0;
  static int espera=0;
  static int step=0;
  int x1,x2;
  if (espera>0) {
    espera--;
    delay(1);
  } else {
    x1=   xd+ (xp>16? (16+2*(xp-16)):xp);
    x2=64+xd+ (xp<16? (-16+(xp*2))  :xp);
    switch (step){
      case 0:
       display.clearDisplay(); // Clear the display buffer
       if (xp<6) { 
               display.drawBitmap(x1, 8, peyes[mood][2][0], 32, 32, WHITE);
               display.drawBitmap(x2, 8, peyes[mood][1][1], 32, 32, WHITE);
       } else if (xp<26) {
               display.drawBitmap(x1, 8, peyes[mood][0][0], 32, 32, WHITE);
               display.drawBitmap(x2, 8, peyes[mood][0][1], 32, 32, WHITE); 
       } else {
               display.drawBitmap(x1, 8, peyes[mood][1][0], 32, 32, WHITE); 
               display.drawBitmap(x2, 8, peyes[mood][2][1], 32, 32, WHITE);
       }
       display.display();
       espera=random(250, 1000);
       n=random(0,7);
       if (n==6) {
          step=1;
       } else {
          step=2;
       }
       break;
      case 1:
       display.clearDisplay(); // Clear the display buffer
       display.drawBitmap(x1, 8, eye0, 32, 32, WHITE);
       display.drawBitmap(x2, 8, eye0, 32, 32, WHITE);
       display.display();
       espera=100;
       step=0;
       break;
      case 2:
       n=random(0,10);
       if (n<5) xd--;
       if (n>5) xd++;
       if (xd<-4) xd=-3;
       if (xd>4) xd=3; 
       espera=0;
       step=0;
       break;
    }
  }

n=readkey();
if (n==2) xp=(xp<=0?0:xp-1); 
if (n==4) xp=(xp>=32?32:xp+1);
if (n==1) {
  mood=(mood>=5?0:mood+1);
  do {} while (readkey()!=0);
  }
if (n!=0) { espera=0; step=0; }

}


