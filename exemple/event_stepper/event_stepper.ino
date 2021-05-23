/*************************************************
 *************************************************
    Sketch event_stepper.ino   test stepper moter in event context
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
    V1.0 (27/04/2021)
    need https://github.com/tyhenry/CheapStepper  created by Tyler Henry, 6/2016 GNU General Public License v3.0

 *************************************************/

#define APP_NAME "event_minimal V2.0"

#include "betaEvents.h"
#include <CheapStepper.h>

EventManager MyEvent;   // local instance de eventManager

/* Evenements du Manager (voir betaEvents.h)
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

CheapStepper stepper;
// here we declare our stepper using default pins:
// arduino pin <--> pins on ULN2003 board:
// 8 <--> IN1
// 9 <--> IN2
// 10 <--> IN3
// 11 <--> IN4



#if  defined(__AVR__)
#define BP0 2  // D2    //
#elif defined(ESP8266) || defined(ESP32)
#define BP0 D2 // D2
#endif

// instance Push Button
evHandlerButton MyBP0(evBP0, BP0);

// instance LED
evHandlerLed    MyLed0(evLed0, LED_BUILTIN);

// instance Serial
evHandlerSerial MyKeyboard;

boolean moveClockwise = true;

void setup() {
  Serial.begin(115200);
  Serial.println(F("\r\n\n" APP_NAME));
  // Start instance
  MyEvent.begin();
  MyEvent.addEventHandler(&MyKeyboard);
  MyEvent.addEventHandler(new evHandlerDebug );
  MyEvent.addEventHandler(&MyBP0);        // ajout d'un bouton sur BP0
  MyEvent.addEventHandler(&MyLed0);       // ajout LED0
  MyLed0.setFrequence(1, 10);
  stepper.setRpm(20);
  Serial.println("Bonjour ....");
}

void loop() {
  // test
  MyEvent.getEvent(false);
  MyEvent.handleEvent();
  int step = stepper.getStepsLeft();
  if ( step > 0 && step < 100 ) {
    Serial.println(stepper.getStepsLeft());
    //stepper.stop();
    
    stepper.newMove (true, 4076);
  }
  stepper.run();
  switch (MyEvent.currentEvent.code)
  {
    case evBP0:
      switch (MyEvent.currentEvent.param) {
        case evBPDown:
          MyLed0.setMillisec(500, 50);
          Serial.println(F("BP0 Down"));
          stepper.newMove (true, 4076);
          break;
        case evBPUp:
          MyLed0.setMillisec(1000, 10);
          Serial.println(F("BP0 Up"));
          stepper.stop();
          stepper.off();
          break;
        case evBPLongDown:
          Serial.println(F("BP0 Long Down"));
          break;
        case evBPLongUp:
          Serial.println(F("BP0 Long Up"));
          break;

      }
      break;

  }
}
