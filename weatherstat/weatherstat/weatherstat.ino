#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD.h>        //
#include <Wire.h>
#include <TimerOne.h>   //
#include "SystemFont5x7.h"
#include "Arial_black_16.h"

#define DISPLAYS_ACROSS 3
#define DISPLAYS_DOWN   3
#define TXEN            2 

uint16_t au16data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int MQSensor = A0;

char weatherSTR[9][9];
unsigned long tempus;
char c;
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
byte sensdata[50];

/* LED MATRIX */
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
/* MODBUS SLAVE */

/*--------------------------------------------------------------------------------------
  Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  called at the period set in Timer1.initialize();
--------------------------------------------------------------------------------------*/
void ScanDMD()
{ 
  dmd.scanDisplayBySPI();
}

/*--------------------------------------------------------------------------------------
  setup
  Called by the Arduino architecture before the main loop begins
--------------------------------------------------------------------------------------*/
void setup(void)
{
   pinMode(13, OUTPUT);
   pinMode(2, OUTPUT);
   digitalWrite(13, HIGH);
   digitalWrite(2, LOW);
    
   Serial.begin(9600);
   inputString.reserve(50);
   
   Wire.begin(2); /* I2C COMM INIT */
   Wire.onReceive(receiveEvent); // register event
   Wire.onRequest(requestEvent); // data request to slave
 
   //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()
   //clear/init the DMD pixels held in RAM
   dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
 }

/*--------------------------------------------------------------------------------------
  loop
  Arduino architecture main loop
--------------------------------------------------------------------------------------*/
void loop(void)
{
   digitalWrite(13, LOW);
  if (stringComplete) {
    digitalWrite(2, HIGH);
    delay(1000);
    Serial.println(inputString);
    delay(1000);
    digitalWrite(2, LOW);
    // PARSING 
    inputString.getBytes(sensdata, 17);
    au16data[0] = sensdata[5]; // humidity 
    au16data[1] = sensdata[8];
    au16data[1] = (au16data[1] << 8) | sensdata[9]; //rain
    au16data[1] = au16data[1] * 0.3;
    au16data[2] = sensdata[6] * 0.137; // windspeed
    au16data[3] = (sensdata[3] & 0x07);
    au16data[3] = (au16data[3] << 8) | sensdata[4];
    au16data[3] = (int)(au16data[3] - 400) * 0.1;
    au16data[4] = analogRead(MQSensor);
    au16data[4] = (int)((3.3 - (0.0032 * ((au16data[4] * -1)+1024))) * 5.2);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  
  dmd.drawBox(  0,  0, (32*DISPLAYS_ACROSS)-1, (16*DISPLAYS_DOWN)-1, GRAPHICS_NORMAL );
  dmd.selectFont(System5x7);
  /* Draw Weather Details */
  dmd.drawString( 2 , 2, "SISFO Indonesia", 15, GRAPHICS_NORMAL );
  dmd.drawString( 2 , 11, "Temp : ", 7, GRAPHICS_NORMAL );
  dmd.drawString( 2 , 20, "Humi : ", 7, GRAPHICS_NORMAL );
  dmd.drawString( 2 , 29, "Wind : ", 7, GRAPHICS_NORMAL );
  dmd.drawString( 2 , 38, "Rain : ", 7, GRAPHICS_NORMAL );

  sprintf(weatherSTR[0],"%3d", au16data[3]);
  sprintf(weatherSTR[1],"%3d", au16data[0]);
  sprintf(weatherSTR[2],"%3d", au16data[2]);
  sprintf(weatherSTR[3],"%3d", au16data[1]);

  dmd.drawString( 39 , 11, weatherSTR[0], 3, GRAPHICS_NORMAL );
  dmd.drawString( 39 , 20, weatherSTR[1], 3, GRAPHICS_NORMAL );
  dmd.drawString( 39 , 29, weatherSTR[2], 3, GRAPHICS_NORMAL );
  dmd.drawString( 39 , 38, weatherSTR[3], 3, GRAPHICS_NORMAL );
  
  dmd.drawString( 64 , 11, "Celc", 4, GRAPHICS_NORMAL );
  dmd.drawString( 64 , 20, "%RH", 3, GRAPHICS_NORMAL );
  dmd.drawString( 64 , 29, "Knot", 4, GRAPHICS_NORMAL );
  dmd.drawString( 64 , 38, "mm  ", 5, GRAPHICS_NORMAL );
   
  delay( 2000 );      
  dmd.drawString( 2 , 38, "CO2  : ", 6, GRAPHICS_NORMAL );
  sprintf(weatherSTR[3],"%3d", au16data[4]);
  dmd.drawString( 39 , 38, weatherSTR[3], 3, GRAPHICS_NORMAL );
  dmd.drawString( 64 , 38, "ppm", 5, GRAPHICS_NORMAL );
  delay( 2000 );

}

void receiveEvent(int howMany) {
    digitalWrite(13, HIGH);
    while (0 < Wire.available()) {  // loop through all but the last
    byte x = Wire.read();         // receive byte as a character
    c = x;
  } 
}

void requestEvent() {

    byte response[2];
  
    switch(c) {

      case 'A' :
          response[0] = au16data[0] & 0x00FF; //LSB
          response[1] = (au16data[0] & 0xFF00) >> 8; //LSB
          Wire.write(response,2);
          break;

      case 'B' :
          response[0] = au16data[1] & 0x00FF; //LSB
          response[1] = (au16data[1] & 0xFF00) >> 8; //LSB
          Wire.write(response,2);
          break;

      case 'C' :
          response[0] = au16data[2] & 0x00FF; //LSB
          response[1] = (au16data[2] & 0xFF00) >> 8; //LSB
          Wire.write(response,2);
          break;

      case 'D' :
          response[0] = au16data[3] & 0x00FF; //LSB
          response[1] = (au16data[3] & 0xFF00) >> 8; //LSB
          Wire.write(response,2);
          break;

      case 'E' :
          response[0] = au16data[4] & 0x00FF; //LSB
          response[1] = (au16data[4] & 0xFF00) >> 8; //LSB
          Wire.write(response,2);
          break;

      case 'F' :
          response[0] = au16data[5] & 0x00FF; //LSB
          response[1] = (au16data[5] & 0xFF00) >> 8; //LSB
          Wire.write(response,2);
          break;

      case 'G' :
          response[0] = au16data[6] & 0x00FF; //LSB
          response[1] = (au16data[6] & 0xFF00) >> 8; //LSB
          Wire.write(response,2);
          break;

      case 'H' :
          response[0] = au16data[7] & 0x00FF; //LSB
          response[1] = (au16data[7] & 0xFF00) >> 8; //LSB
          Wire.write(response,2);
          break;
    }
   
}

void serialEvent() {
  int x=0;
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (x == 16) {
      stringComplete = true;
      x = 0;
    }
    x++;
  }
}


