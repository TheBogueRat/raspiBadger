/*
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */
 /*
  Hack.lenotta.com
  Modified code of Getting Started RF24 Library
  * 6/3/2014  - Changed to activate R, G, B as directed
  * 6/4/2014  - Added Purple
  * 6/6/2014  - Added Vibration Sensor
  * 6/26/2014 - Changed to handle unsigned long message array of 2 elements
                  First is Badge#, second is action.
  * 6/27/2014 - Sending in badgeID and Total Ticks to raspiBadger
*/
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware conf 
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

const uint8_t led_pins[] = { 3,5,6 };  // Pins for RGB LED
const uint8_t vibrSensor = 4; // vibration sensor (10k resist)
                              // switch to 3.3v, resistor to ground

uint8_t buttonState;
uint8_t lastButtonState = HIGH;
long unsigned lastMessage = 1;  // Assign initial state
long unsigned badgeID = 18;      // should be unique for each badge.
long unsigned totalTicks = 0;   // Track total times the vibration sensor registers
long lastDebounceTime = 0;      // the last time the output pin was toggled
long debounceDelay = 200;       // the debounce time; increase if the output flickers

// Topology
 
// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
 
void setup(void)
{
  // Print preamble
  Serial.begin(57600);
  
  // Set output pins and turn on
  for (int i=0; i<=3; i+=1) {
    pinMode(led_pins[i], OUTPUT);
    digitalWrite(led_pins[i], HIGH);
  }
  pinMode(vibrSensor, INPUT);
  
  printf_begin();
  printf("\nRemote Control Arduino\n\r");
 
  // Setup and configure rf radio
  radio.begin();
  radio.setRetries(15,15);
  radio.setAutoAck(1);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();
  radio.printDetails();
  
  // Turn on LED - green
  ledChange(lastMessage);
}
 
void loop(void)
{
  // read the state of the vibration sensor 
  int reading = digitalRead(vibrSensor);
  if ((reading == HIGH) && (millis() - lastDebounceTime) > debounceDelay) {
    // Turn Lights On then reset Delay period
      totalTicks++;
      lastDebounceTime=millis();
      digitalWrite(led_pins[0], LOW);
      digitalWrite(led_pins[1], LOW);
      digitalWrite(led_pins[2], LOW);
      printf("Vibration detected and taking action!\n\r");
      delay(200);
      // Send info to raspiBadgerComm
       unsigned long response[2] = {badgeID, totalTicks};
      // First, stop listening so we can talk
      radio.stopListening();
      radio.write( &response, sizeof(response) );
      printf("Sent totalTicks; badgeID: %lu  Ticks: %lu.\n\r", response[0], response[1]);
      delay(20);
      radio.startListening();
      // reset LED to last command color
      ledChange(lastMessage);
    } 
  // need to record # of vibrations and send data 
  // place holder for vibration ...
  
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      unsigned long message[2];
      bool done = false;
      while (!done){
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &message, sizeof(message) );
        //done = radio.read( &message, sizeof(unsigned long) );
         
        // Spew it
        printf("Got message badge#: %lu...msg: %lu \n\r",message[0], message[1]);
        
        if (message[0] == badgeID) {
          ledChange(message[1]);
          lastMessage = message[1];
          printf(" and took action...");
        } else {
          printf(" and ignored...Not my badgeID (%lu), received %lu \n\r", badgeID, message[0]);
        }
    // Delay just a little bit to let the other unit transition to receiver
    delay(20);  // raised from 20; receiving 5 transmissions
      }
/* not replying, using autoAck?
      // First, stop listening so we can talk
      radio.stopListening();
 
      // Send the final one back.
      radio.write( &message, sizeof(message) );
      printf("Sent response. BadgeID: %lu  Action: %lu\n\r",message[0],message[1]);

      // Now, resume listening so we catch the next packets.
      radio.startListening();
*/      
      // To avoid a bounce due to LED state change.
      lastDebounceTime=millis();
    }
}
void ledChange(long unsigned msg) {
            if (msg == 3) { // Purple
            analogWrite(led_pins[0], 55);
            digitalWrite(led_pins[1], HIGH);
            analogWrite(led_pins[2], 0);
          } else if (msg == 0) { // Red
            digitalWrite(led_pins[0], LOW);
            digitalWrite(led_pins[1], HIGH);
            digitalWrite(led_pins[2], HIGH);
          } else if (msg == 1) { // Green
            digitalWrite(led_pins[0], HIGH);
            digitalWrite(led_pins[1], LOW);
            digitalWrite(led_pins[2], HIGH);
          } else if (msg == 2) { // Blue
            digitalWrite(led_pins[0], HIGH);
            digitalWrite(led_pins[1], HIGH);
            digitalWrite(led_pins[2], LOW);
          } else if (msg == 255) {  // Black
            digitalWrite(led_pins[0], HIGH);
            digitalWrite(led_pins[1], HIGH);
            digitalWrite(led_pins[2], HIGH);
          }
/*
   Adding color reference below: (invert for common Cathode/Anode?)
 0 - RED (LOW, HIGH, HIGH)
 ORANGE[] = {172, 251, HIGH}; 
 YELLOW[] = {LOW, LOW, HIGH}; 
 1 - GREEN const byte GREEN[] = {HIGH, LOW, HIGH}; 
 2 - BLUE {HIGH, HIGH, LO}; 
 INDIGO[] = {251, HIGH, 236}; 
 VIOLET[] = {232, HIGH, 233}; 
 CYAN[] = {HIGH, LOW, LOW}; 
 MAGENTA[] = {LOW, HIGH, LOW}; 
 WHITE[] = {LOW, LOW, LOW}; 
 BLACK[] = {HIGH, HIGH, HIGH}; 
 PINK[] = {97, 251, 176};
 3 - VIOLET (55, 255, 0)
 */
}
