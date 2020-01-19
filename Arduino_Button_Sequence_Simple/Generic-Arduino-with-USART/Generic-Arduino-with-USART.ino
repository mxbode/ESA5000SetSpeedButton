#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>
#include "xbot_display_data.h"

#define ScooterSerial Serial
#define RX_DISABLE UCSR0B &= ~_BV(RXEN0);
#define RX_ENABLE  UCSR0B |=  _BV(RXEN0);
#define BUTTON_PIN 14
#define RESEND 4
#define MILLISPERPRESS 275
#define MINMILLISPERPRESS (presses*100)

#define THROTTLE_MIN 0x2C
#define THROTTLE_MAX 0xC5
#define BRAKE_MIN 0x2C
#define BRAKE_MAX 0xB5

int speed = 30;
int presses=5;
xbot_display_data scooterData;

uint16_t calculateChecksum(uint8_t *data)
{
  uint8_t len = data[0] + 2;
  uint16_t sum = 0;
  for (int i = 0; i < len; i++)
    sum += data[i];

  sum ^= 0xFFFF;
  return sum;
}

void transmitPacket(uint8_t throttle, uint8_t brake)
{
  static int counter = 0;

  uint8_t fifthHeader[] = {
    0x09, 0x27, 0x63, 0x07, 0x06,
  };
  uint8_t regularHeader[] = {
    0x07, 0x25, 0x60, 0x05, 0x04,
  };

  uint8_t data[] = {
    0, 0, 0, 0, 0, // header, will be either regularHeader or fifthHeader
    throttle, // range 0x2C - 0xC5 
    brake, // range 0x2C - 0xB5
    0x00, 0x00, // ?
    0x00, 0x04, // ?, only present on every fith package
    0x00, 0x00, // checksum
  };

  uint8_t len;
  if(counter == 4)
  {
    len = 13;
    memcpy(data, fifthHeader, 5);

    counter = 0;
  }
  else
  {
    len = 11;
    memcpy(data, regularHeader, 5);

    counter++;
  }

  // XXX we assume our architecture uses LE order here
  *(uint16_t *)&data[len - 2] = calculateChecksum(data);

  RX_DISABLE;
  ScooterSerial.write(0x55);
  ScooterSerial.write(0xAA);
  ScooterSerial.write(data, len);
  RX_ENABLE;
}

void setSpeed(const int speed) { //Setze Maximalgeschwindigkeit in km/h
  uint8_t data[] = {
    0x55, 0xAA, 0x04, 0x22, 0x01, 0xF2,
    0, 0, //rpm
    0, 0, //checksum
  };

  *(uint16_t *)&data[6] = (speed * 252) / 10;
  *(uint16_t *)&data[8] = calculateChecksum(data + 2);

  for (int i=0; i<=RESEND; i++) {
    if (i != 0) {
      delay(150);
    }
    RX_DISABLE;
    ScooterSerial.write(data, sizeof(data) / sizeof(uint8_t));
    RX_ENABLE;
  }

}

bool waitpress(int signal) {
          int i=0;
           while(digitalRead(BUTTON_PIN) == signal) {
              delay(50);
              if(i>2) return false;
              i++;
            }
            return true;
}

bool checkCode(int presses) {
  unsigned long time=0;
  unsigned long temp=0;

      if(digitalRead(BUTTON_PIN) == HIGH) return false;
         time = millis();
         for(int i=0; i<(presses+1); i++) {
                   if(!waitpress(LOW)) return false;
                   delay(50);
                   if(!waitpress(HIGH)) return false;
                   delay(50);
         }

         temp = millis();
         if((temp-time) < (MILLISPERPRESS*presses) && (temp-time) > MINMILLISPERPRESS) return true;
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(LED_BUILTIN, LOW);
  ScooterSerial.begin(115200);
  scooterData.init();
  RX_ENABLE;
  delay(5000); //to get a chance to programm with usart
}

void loop()
{
  int cruisecontroltimer=0;

    while (ScooterSerial.available() >= 2)
  {
    if (ScooterSerial.read() == 0x55 && ScooterSerial.peek() == 0xAA) 
    {
      // remove 0xAA
      ScooterSerial.read();
      if(ScooterSerial.peek() == 0x07 || ScooterSerial.peek() == 0x09 || ScooterSerial.peek() == 0x0B)
      {
        int size = 0;
        switch (ScooterSerial.peek())
        {
          case 0x07: size = 0x07 + 4;break;
          case 0x09: size = 0x09 + 4;break;
          case 0x0B: size = 0x0B + 4;break;
          default: break;
        }

        byte readData[size];

        for(int i = 0; i < size; i++)
        {
          readData[i] = ScooterSerial.read();        
        }

        if(calculateChecksum(readData) == readData[size-2] + readData[size-1] * 256)
        {
          scooterData.newData(readData);
        }
      }
    }
  }

if(scooterData.throttle_ == 100) {
  if(cruisecontroltimer == 0) {
    cruisecontroltimer=millis();
  } else {
    cruisecontroltimer+=millis();
  }  
//  digitalWrite(LED_BUILTIN, HIGH);
//  delay(500);
//  digitalWrite(LED_BUILTIN, LOW);
}

if(scooterData.break_ == 100) {
  cruisecontroltimer=0;
}

if(cruisecontroltimer>2000) {
  transmitPacket(THROTTLE_MAX, 0);
}

           if(checkCode(presses)) {
            digitalWrite(LED_BUILTIN, HIGH);
            setSpeed(speed);
            delay(500);
            presses=3;
            speed=20;
            digitalWrite(LED_BUILTIN, LOW);
           }
           //delay(10);
}
