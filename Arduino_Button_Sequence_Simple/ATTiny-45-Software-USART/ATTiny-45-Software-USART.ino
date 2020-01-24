#include <SoftwareSerial.h>
#include <Arduino.h>
#include <stdint.h>
#include <avr/power.h>
#include <avr/sleep.h>


#define TX_PIN 3                            // PIN for [TR] on scooter
#define BUTTON_PIN 2                        // PIN for [SW] on scooter
#define RESEND 4                            // times that we send the speed unlock message to scooter data bus
#define MILLISPERPRESS 280                  // max time in millis that a press or time between presses are allowed to last: rounded to x*10
#define MINMILLISPERPRESS (presses*100)     // minimum time in millis for the whole unlock procedure for debounce (example: 5 times x 100 millis: fastest 500 millis)
#define UNLOCKAFTERPOWERON false            // ignore buttons.... unlock after qC gets powered on

SoftwareSerial ScooterSerial(TX_PIN, 4);
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

bool waitpress(int signal) {
          int i=0;
           while(digitalRead(BUTTON_PIN) == signal) {
              delay(10);
              if(i>(MILLISPERPRESS/10)) return false;
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
  pinMode(BUTTON_PIN, INPUT);
  ScooterSerial.begin(115200);
}

void loop()
{
  if(UNLOCKAFTERPOWERON) {
    setSpeed(speed);
    delay(150);
  } else {
    if(checkCode(presses)) {
      setSpeed(speed);
      presses=3;
      speed=20;
      if(speed == 20) { // end of program; we can sleep now; please reboot the scooter
        power_all_disable();
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sleep_mode();
        sleep_disable();
      } //if(speed == 20)
    } //if(checkCode(presses))
  }
  delay(50);
}
