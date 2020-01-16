# Docgreen Esa 5000 Arduino Setspeed Button
Use This Arduino Sketch And Connect Your Arduino To Your Docgreen Esa 5000 Escooter. You Can Define A Sequence That You Have To Press The Powerbutton Of Your Scooter To Set A New Speed Limit.


# Connect Your Arduino 
Connect your arduino like in this project: https://Github.Com/M4gnv5/Docgreenesa5000-Dashboard
5v Power And Databus Should Now Connected Between The Scooter And Your Arduino.

Now You Have To Connect The Yellow Cable That Connects The Scooters Dashboard With The Scooters Controller Motherboard. It Gets Pulled To Ground When You Press The Button. You Have To Use An Diode And An Pullup Resistor As Shown In The Following Picture:

![](pictures/arduino.png)

When You Use This Sketch Without Modification, You Have To Connect The Button To Pin 14 On Your Arduino.

# Usage

After Turning On Your Scooter, The Arduino Will Get Powered Too. Klick 6 Times Fast On The Power Button And The Scooters Max Speed Will Be 30 Kilometers/H. Three More Times And The Scooters Max Speed Will Reset To Default 20 Km/H. You Now Have To Turn Your Scooter Off And On Again.

