/*************************************************
 *************************************************
    Sketch sonoff_basic.ino   validation of lib betaEvents to deal nicely with events programing with Arduino
    Copyright 2020 Pierre HENRY net23@frdev.com All - right reserved.

    This file is part of betaEvents.

    betaEvents is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    betaEvents is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with betaEvents.  If not, see <https://www.gnu.org/licenses/lglp.txt>.


   History
    V1.0 (20/092022)
   

    *************************************************/

//#include <ESP8266WiFi.h>
// Definition des constantes pour les IO
//#include "ESP8266.h"
static_assert(sizeof(time_t) == 8, "This version works with time_t 64bit  move to ESP8266 kernel 3.0 or more");

#define APP_NAME "SonoffBasic V1.0"



#include "EventsManager.h"
// Preinstantiate Objects /// as Nicolas Zambetti with Wire.cpp /////

const int LED0_PIN = 13;
const int RELAY0_PIN = 12;
const int BP0_PIN = 0;

EventManager Events = EventManager();

/* Evenements du Manager (voir betaEvents.h)
  evNill = 0,      // No event  about 1 every milisecond but do not use them for delay Use delayedPushEvent(delay,event)
  ev100Hz,         // tick 100HZ    non cumulative (see betaEvent.h)
  ev10Hz,          // tick 10HZ     non cumulative (see betaEvent.h)
  ev1Hz,           // un tick 1HZ   cumulative (see betaEvent.h)
  ev24H,           // 24H when timestamp pass over 24H
  evInChar,
  evInString,
*/

// Liste des evenements specifique a ce projet
enum tUserEventCode {
  // evenement recu
  evBP0 = 100,
  evLed0,
  evRelay0,
};

// poussoir
evHandlerButton BP0(evBP0, BP0_PIN);
// led clignotante a 1Hz
evHandlerLed Led0(evLed0, LED0_PIN, LOW, 1);
// Commande relay
evHandlerLed Relay0(evRelay0, RELAY0_PIN, LOW, 0);



// serial
#define SERIAL_SPEED 115200
#define SERIAL_BUFFERSIZE 100
evHandlerSerial Keyboard(SERIAL_SPEED, SERIAL_BUFFERSIZE);
evHandlerDebug  Debug;



void setup() {
  //enableWiFiAtBootTime();   // obligatoire pour kernel ESP > 3.0
  //WiFi.forceSleepBegin();

  // Start instance
  Events.begin();
  Serial.println(F("\r\n\n" APP_NAME));
  Led0.setFrequence(1, 10);

  Serial.println("Bonjour ....");
  D_println(sizeof(stdEvent_t));
  D_println(sizeof(size_t));
}

bool sleepOk = true;

void loop() {
  Events.get(sleepOk);
  Events.handle();
  //D_println(Events.code);
  switch (Events.code) {
      //    case evNill:
      //      break;


    case evInit:
      Serial.println(F("Init"));
      break;


    case ev24H:
      {
        Serial.println(F("ev24H"));
      }
      break;

    case evInChar:
      {
        if (Debug.trackTime < 2) {
          char aChar = Keyboard.inputChar;
          if (isPrintable(aChar)) {
            D_println(aChar);
          } else {
            D_println(int(aChar));
          }
        }
        switch (Keyboard.inputChar) {
          case '0': delay(10); break;
          case '1': delay(100); break;
          case '2': delay(200); break;
          case '3': delay(300); break;
          case '4': delay(400); break;
          case '5': delay(500); break;
        }
      }
      break;

    case evInString:
      if (Debug.trackTime < 2) {
        D_println(Keyboard.inputString);
      }



      if (Keyboard.inputString.equals(F("FREE"))) {
        D_println(Events.freeRam());
      }
      if (Keyboard.inputString.equals("S")) {
        sleepOk = !sleepOk;
        D_println(sleepOk);
      }
      break;      
      
  }
}