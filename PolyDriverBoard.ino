/* 16 Channel LED Driver Board for /The Polytechnic/.
 *
 * Copyright 2014 Ethan Spitz
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Connections:
 * # Potentiometer: A0
 * # Button: D2
 * # Seven-Segment Display Driver
 *    # A: A1 //LSB
 *    # B: 6
 *    # C: 7
 *    # D: A2 //MSB
 * # 
 *
 */
 
#include <Wire.h>
 
int mode = 0; // current mode
int button = 2; // pin of button
 
int potValue = 0; // value of pot
bool sevSegOff = true;

// variables varied by the pot
float intensityScale;
float speedScale;
 
// values used for mode set/buttton debouncing
long modeSetTime = 0;
int buttonState;
int lastButtonState = HIGH; 
long lastDebounceTime = 0;
long debounceDelay = 50;
  
// definition of registers on the pwm chip
#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define PWM_ADDR 0x04 //address of the PWM chip


void setup()
{
  // begin configure seven seg driver
  pinMode(A1, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(A2, OUTPUT);
  // end configure seven seg driver
   
  // configure pushbutton
  pinMode(2, INPUT);
   
  // configure serial and i2c communication
  Serial.begin(9600);
  Wire.begin();
   
  // configure PWM chip
  setConfiguration();
}
 
void loop()
{  
     
  potValue = analogRead(A0); // save value from potentiometer
  checkMode(); // check button/update seven seg
  
  // defines what each mode does
  switch (mode)
  {
    case 1:
    
      break;
    case 2:
      if (sevSegOff)
      {
        speedScale = (float)potValue/1023;
        setSevenSeg((int)(9*speedScale));
      }
      break;
    case 3:
      if (sevSegOff)
      {
        intensityScale = (float)potValue/1023;
        setSevenSeg((int)(9*intensityScale));
      }
      break;
    default:
      break;
  }
}
 
 /*
  * function to control the seven seg display, display value
  */
void setSevenSeg (int value)
{
  switch(value)
  {
    case 0:
      digitalWrite(A1, LOW);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      digitalWrite(A2, LOW);
      break;
    case 1:
      digitalWrite(A1, HIGH);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      digitalWrite(A2, LOW);
      break;
    case 2:
      digitalWrite(A1, LOW);
      digitalWrite(6, HIGH);
      digitalWrite(7, LOW);
      digitalWrite(A2, LOW);
      break;
    case 3:
      digitalWrite(A1, HIGH);
      digitalWrite(6, HIGH);
      digitalWrite(7, LOW);
      digitalWrite(A2, LOW);
      break;
    case 4:
      digitalWrite(A1, LOW);
      digitalWrite(6, LOW);
      digitalWrite(7, HIGH);
      digitalWrite(A2, LOW);
      break;
    case 5:
      digitalWrite(A1, HIGH);
      digitalWrite(6, LOW);
      digitalWrite(7, HIGH);
      digitalWrite(A2, LOW);
      break;
    case 6:
      digitalWrite(A1, LOW);
      digitalWrite(6, HIGH);
      digitalWrite(7, HIGH);
      digitalWrite(A2, LOW);
      break;
    case 7:
      digitalWrite(A1, HIGH);
      digitalWrite(6, HIGH);
      digitalWrite(7, HIGH);
      digitalWrite(A2, LOW);
      break;
    case 8:
      digitalWrite(A1, LOW);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      digitalWrite(A2, HIGH);
      break;
    case 9:
      digitalWrite(A1, HIGH);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      digitalWrite(A2, HIGH);
      break;
    default: // blank screen for numbers not in 0-9
      digitalWrite(A1, HIGH);
      digitalWrite(6, HIGH);
      digitalWrite(7, HIGH);
      digitalWrite(A2, HIGH);
  }
}
 
/*
 *  Checks state of button and increments mode if pressed and displays value on seven seg
 */
void checkMode()
{
  // debounced button reading below
  int reading = digitalRead(button);
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  } 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        mode++;
        modeSetTime = millis(); // set time mode was set so seven seg can be turned off after
                                // duration
        if (mode>3) mode = 0;
        setSevenSeg(mode); //set seven seg to mode number
        sevSegOff = false;
      }
    }
  }
  lastButtonState = reading;
  if ((millis() - modeSetTime) > 2000)
  { 
    setSevenSeg(10); // disable seven seg after 2000 milliseconds
    sevSegOff = true;
  }
}
 
/*
 * setPWM brightness on channel given over I2C
 */
void setPWM(int channel, uint16_t brightness)
{
  Wire.beginTransmission(PWM_ADDR);
  Wire.write(LED0_ON_L+4*channel);
  Wire.write(0x00); //turn the LED on at 0
  Wire.write(0x00); //turn the LED on at 0
  
  //turn the LED off when it hits this value (out of 4095)
  Wire.write(brightness); //first four LSB
  Wire.write(brightness>>8); //last four MSB
  Wire.endTransmission();
}
 
/*
 * Configure the PWM chip for easy suage and external MOSFET drive
 */
void setConfiguration()
{
  Wire.beginTransmission(PWM_ADDR);
  Wire.write(0x00); //enter Mode 1 Register
  Wire.write(0x21); //enable ocsillator and auto-increment register
  Wire.beginTransmission(PWM_ADDR);
  Wire.write(0x01); //enter Mode 2 Register
  Wire.write(0x01); //set drive mode for external MOSFETS 
  Wire.endTransmission();
}
