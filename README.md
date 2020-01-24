# Docgreen Esa 5000 Arduino Setspeed Button
Use this arduino sketch and connect your arduino to your docgreen ESA 5000 Escooter. You can define a sequence that you have to press the powerbutton of your scooter to set a new speed limit.

# Tested Boards
- Arduino Pro Mini
- Arduino Nano
- ATtiny45

#ToDo ESP-01, ESP8266

# Connect Your Arduino 
Connect your arduino like in this project: https://Github.Com/M4gnv5/Docgreenesa5000-Dashboard
5V power and databus should now connected between the scooter and your arduino.

In generic Arduino you have to connect TX PIN throug a diode to [TR] on scooter and PIN 14 throug a diode to [SW] on scooter. Use the diode as shown in ATtiny picture below.

For ATtiny you have to connect PIN 4 throug a diode to [TR] on escooter and PIN 2 throug a diode to [SW] on escooter.

![](pictures/arduino.png)

![](pictures/tiny45.jpg)


# Usage

After turning on your scooter, the arduino will get powered too. Klick 6 times fast on the power button and the scooters max speed will be 30 kilometers/h. Three more times and the scooters max speed will reset to default 20 kilometers/h. You now have to turn your scooter off and on again.
