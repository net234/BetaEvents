/*************************************************
 *************************************************
    Sketch betaEvents.ino   validation of lib betaEvents to deal nicely with events programing with Arduino
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
    - Full rebuild from PH_Event V1.3.1 (15/03/2020)
    V1.1 (30/11/2020)
    - Ajout du percentCPU pour une meilleur visualisation de l'usage CPU
    V1.2 02/01/2021
    - Ajout d'une globale EventManagerPtr pour l'acces par d'autre lib et respecter l'implantation C++
    - Amelioration du iddle mode pour l'ESP8266 (WiFi sleep mode)

 *************************************************/

#define APP_NAME "betaEvents V1.4"

#if  defined(__AVR__)
#include <avr/wdt.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "betaEvents.h"
#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");

EventTracker MyEvent;   // local instance de eventManager

/* Evenements du Manager (voir betaEvents.h)
  evNill = 0,      // No event  about 1 every milisecond but do not use them for delay Use pushDelayEvent(delay,event)
  ev100Hz,         // tick 100HZ    non cumulative (see betaEvent.h)
  ev10Hz,          // tick 10HZ     non cumulative (see betaEvent.h)
  ev1Hz,           // un tick 1HZ   cumulative (see betaEvent.h)
  ev24H,           // 24H when timestamp pass over 24H
  //  evDepassement1HZ,
  evLEDOn,
  evLEDOff,
  evInChar,
  evInString,
*/

// Liste des evenements specifique a ce projet
enum tUserEventCode {
  // evenement recu
  evBP0Down = 100,    // BP0 est appuyé
  evBP0Up,            // BP0 est relaché
  evBP0MultiDown,         // BP0 est appuyé plusieur fois de suite
  evBP0LongDown,      // BP0 est maintenus appuyé plus de 3 secondes
  evBP0LongUp,        // BP0 est relaché plus de 3 secondes
  ev1S,
  ev2S,
  ev3S,
  // evenement action
  doReset,
};

#if  defined(__AVR__)
#define BP0 8
#elif defined(ESP8266) || defined(ESP32)
#define BP0 0
#endif

bool sleepOk = true;
int  multi = 0; // nombre de clic rapide


void setup() {
  // IO Setup
#if defined(ESP8266)
  WiFi.forceSleepBegin();
  //   WiFi.mode(WIFI_OFF);
#elif defined(ESP32)
  WiFi.mode(WIFI_OFF);
  btStop();
#endif
  pinMode(BP0, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println(F("\r\n\n" APP_NAME));

  // Start instance
  MyEvent.begin();

  Serial.println("Bonjour ....");
  //D_println(sizeof(EventManagerPtr));
}
bool BP0Down = false;
byte BP0Multi = 0;

//int ev1000HzCnt = 0;
//int ev100HzCnt = 0;
//int ev10HzCnt = 0;


void loop() {
  // test
  MyEvent.getEvent(sleepOk);
  MyEvent.handleEvent();
  switch (MyEvent.currentEvent.code)
  {

//    case ev1000Hz:
//      ev1000HzCnt++;
//      break;
//
//    case ev100Hz:
//      ev100HzCnt++;
//      break;
//
//
//    case ev1Hz:
//      D_println(ev1000HzCnt);
//      D_println(ev100HzCnt);
//      D_println(ev10HzCnt);
//      ev1000HzCnt = 0;
//      ev100HzCnt = 0;
//      ev10HzCnt = 0;
//      break;

    case ev10Hz: {
//        ev10HzCnt++;
        if ( BP0Down != (digitalRead(BP0) == LOW)) { // changement d'etat BP0
          BP0Down = !BP0Down;
          if (BP0Down) {
            MyEvent.setMillisecLED(500, 50);
            MyEvent.pushEvent(evBP0Down);
            MyEvent.pushDelayEvent(3000, evBP0LongDown); // arme un event BP0 long down
            MyEvent.removeDelayEvent(evBP0LongUp);
            if ( ++BP0Multi > 1) {
              MyEvent.pushEvent(evBP0MultiDown, BP0Multi);
            }
          } else {
            MyEvent.setMillisecLED(1000, 10);
            MyEvent.pushEvent(evBP0Up);
            MyEvent.pushDelayEvent(1000, evBP0LongUp); // arme un event BP0 long up
            MyEvent.removeDelayEvent(evBP0LongDown);
          }
        }

        break;
      }
    case ev24H:
      Serial.println("---- 24H ---");
      break;


    //    case evLEDOn:
    //      Serial.print(F("L"));
    //      break;
    //
    //    case evLEDOff:
    //      Serial.print(F("l"));
    //      break;

    case evBP0Down:
      Serial.println(F("BP0 Down"));
      break;

    case evBP0Up:
      Serial.println(F("BP0 Up"));
      break;

    case evBP0LongDown:
      Serial.println(F("BP0 Long Down"));
      if (multi == 5) {
        Serial.println(F("RESET"));
        MyEvent.pushEvent(doReset);
      }
      break;

    case evBP0LongUp:
      BP0Multi = 0;
      Serial.println(F("BP0 Long Up"));
      break;

    case evBP0MultiDown:
      multi = MyEvent.currentEvent.param;
      Serial.print(F("BP0 Multi Clic:"));
      Serial.println(multi);

      break;


    case ev1S:
      Serial.println(F("EV1S"));
      break;
    case ev2S:
      Serial.println(F("EV2S"));
      break;
    case ev3S:
      Serial.println(F("EV3S"));
      break;

    case doReset:
      delay(100);
#ifdef  __AVR__
      wdt_enable(WDTO_120MS);
#else
      ESP. restart();
#endif
      while (1)
      {
        delay(1);
      }
      break;


    case evInChar:
      switch (toupper(MyEvent.inChar))
      {
        case '0': delay(10); break;
        case '1': delay(100); break;
        case '2': delay(200); break;
        case '3': delay(300); break;
        case '4': delay(400); break;
        case '5': delay(500); break;
      }


      break;


    case evInString:

      if (MyEvent.inputString.equals(F("S"))) {
        sleepOk = !sleepOk;
        Serial.print(F("Sleep=")); Serial.println(sleepOk);
      }

      if (MyEvent.inputString.equals(F("P"))) {
        Serial.println(F("Push 3 delay events"));
        Serial.print(F("Ram=")); Serial.println(MyEvent.freeRam());
        MyEvent.pushDelayEvent(1000, ev1S);
        MyEvent.pushDelayEvent(2000, ev2S);
        MyEvent.pushDelayEvent(3000, ev3S);
        Serial.print(F("Ram=")); Serial.println(MyEvent.freeRam());
      }
      if (MyEvent.inputString.equals(F("Q"))) {
        Serial.println(F("Push 3 events"));
        Serial.print(F("Ram=")); Serial.println(MyEvent.freeRam());
        MyEvent.pushDelayEvent(0, ev1S);
        MyEvent.pushDelayEvent(0, ev2S);
        MyEvent.pushDelayEvent(0, ev3S);
        Serial.print(F("Ram=")); Serial.println(MyEvent.freeRam());
      }


      if (MyEvent.inputString.equals(F("FREE"))) {
        Serial.print(F("Ram=")); Serial.println(MyEvent.freeRam());
      }

      if (MyEvent.inputString.equals(F("RESET"))) {
        Serial.println(F("RESET"));
        MyEvent.pushEvent(doReset);
      }

      break;

  }
}
