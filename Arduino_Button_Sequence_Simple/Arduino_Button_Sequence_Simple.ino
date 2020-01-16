#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>

#define ScooterSerial Serial
#define RX_DISABLE UCSR0B &= ~_BV(RXEN0);
#define RX_ENABLE  UCSR0B |=  _BV(RXEN0);
#define BUTTON_PIN 14
#define RESEND 4

int speed = 30;
int presses=5;

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
  if (counter == 4)
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

  ScooterSerial.write(0x55);
  ScooterSerial.write(0xAA);
  ScooterSerial.write(data, len);
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
    ScooterSerial.write(data, sizeof(data) / sizeof(uint8_t));
  }

}

bool waitwhilepress() {
          int i=0;
           while(digitalRead(BUTTON_PIN) == LOW) {
              delay(50);
              if(i>2) return false;
              i++;
            }
            return true;
}
bool waitforpress() {
          int i=0;
           while(digitalRead(BUTTON_PIN) == HIGH) {
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
                   if(!waitwhilepress()) return false;
                   delay(50);
                   if(!waitforpress()) return false;
                   delay(50);
         }

         temp = millis();
                                    // Serial.print("time: ");
                            //Serial.println((temp-time));
         if((temp-time) < 1650 && (temp-time) > 500) return true;

}

void setup()
{
  RX_DISABLE;
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(LED_BUILTIN, LOW);
  ScooterSerial.begin(115200);


}

void loop()
{
           if(checkCode(presses)) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            setSpeed(speed);
            presses=3;
            speed=20;
            digitalWrite(LED_BUILTIN, LOW);
           }
           delay(50);
}
