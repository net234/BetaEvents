/*************************************************
 *************************************************
    Sketch event_minimal.ino   validation of lib betaEvents to deal nicely with events programing with Arduino
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
    V1.0 (21/11/2020)
    V2.0  20/04/2021

 *************************************************/

#define APP_NAME "event_minimal V2.0"

#include "EventsManager.h"
//#include "evHandlers.h"


EventManager MyEvent;   // local instance de eventManager (kernel of BetaEvents)

/* Evenements du Manager (voir BetaEvents.h)
  evNill = 0,      // No event  about 1 every milisecond but do not use them for delay Use pushDelayEvent(delay,event)
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
};

#if  defined(__AVR__)
#define BP0 2  // D2    
#elif defined(ESP8266) || defined(ESP32)
#define BP0 D1 // D1
#endif

// instances poussoir
evHandlerButton MyBP0(evBP0, BP0);

// instance LED
evHandlerLed    MyLed0(evLed0, LED_BUILTIN);

void setup() {
  Serial.begin(115200);
  Serial.println(F("\r\n\n" APP_NAME));
  // Start instance
  MyEvent.begin();
  //  MyEvent.addEventHandler(new evHandlerDebug );
  MyEvent.addEventHandler(&MyBP0);        // ajout d'un bouton sur BP0
  MyEvent.addEventHandler(&MyLed0);       // ajout LED0
  MyLed0.setFrequence(1, 10);
  Serial.println("Bonjour ....");
}

void loop() {
  // test
  MyEvent.getEvent();
  MyEvent.handleEvent();
  switch (MyEvent.currentEvent.code)
  {

    case evBP0:
      switch (MyEvent.currentEvent.ext) {
        case evxBPDown:
          MyLed0.setMillisec(500, 50);
          Serial.println(F("BP0 Down"));
          break;
        case evxBPUp:
          MyLed0.setMillisec(1000, 10);
          Serial.println(F("BP0 Up"));
          break;
        case evxBPLongDown:
          Serial.println(F("BP0 Long Down"));
          break;
        case evxBPLongUp:
          Serial.println(F("BP0 Long Up"));
          break;

      }
      break;

  }
}
