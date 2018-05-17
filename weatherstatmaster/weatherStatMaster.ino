#include <Wire.h>
#include <ModbusRtu.h>
#define ID    2
#define TXEN  2 

Modbus slave(ID, 0, TXEN); // this is slave ID and RS-232 or USB-FTDI

boolean led;
int8_t state = 0;
unsigned long tempus;

uint16_t au16data[9];

uint16_t weatherData[9]={0,0,0,0,0,0,0,0,0};
char cmdByte[8]={'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
int dataIdx;

void setup() {
  /* I2C */
  Wire.begin();
 
  /* MODBUS */
  slave.begin(9600);
  tempus = millis() + 100;
  digitalWrite(13, HIGH );
}

byte x = 0;

void loop() {
   /* Modbus Heart-Beat */
   state = slave.poll( au16data, 9 );
   if (state > 4) {
    tempus = millis() + 50;
    digitalWrite(13, HIGH);
   }
   if (millis() > tempus) digitalWrite(13, LOW );
   
   dataIdx = 0;

   for (dataIdx=0; dataIdx<8; dataIdx++) {
     Wire.beginTransmission(2);
     Wire.write(cmdByte[dataIdx]);
     Wire.endTransmission();
     Wire.requestFrom(2, 2);
  
     byte response[2];
     int index = 0;

    while (Wire.available()) {
        byte b = Wire.read();
        response[index] = b;
        index++;
    }
                     
    weatherData[dataIdx] = (response[1] << 8)| response[0];
   }

   /* Fill Modbus Register from I2C Slave Data */
   for (dataIdx=0; dataIdx<8; dataIdx++) {
    au16data[dataIdx] = weatherData[dataIdx]; 
   }

  delay(100);
}
