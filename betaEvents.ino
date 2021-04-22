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
    V1.4   6/3/2021
    - Inclusion TimeLib.h
    - Gestion des event en liste chainée
    V2.0  20/04/2021
    - Mise en liste chainée de modules 'events' test avec un evButton
  Le croquis utilise 269404 octets (25%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
  Les variables globales utilisent 27400 octets (33%) de mémoire dynamique, ce qui laisse 54520 octets pour les variables locales. Le maximum est de 81920 octets.
  Le croquis utilise 269548 octets (25%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
  Les variables globales utilisent 27400 octets (33%) de mémoire dynamique, ce qui laisse 54520 octets pour les variables locales. Le maximum est de 819
  Le croquis utilise 269708 octets (25%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
  Les variables globales utilisent 27400 octets (33%) de mémoire dynamique, ce qui laisse 54520 octets pour les variables locales.
  Le croquis utilise 269388 octets (25%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
  Les variables globales utilisent 27392 octets (33%) de mémoire dynamique, ce qui laisse 54528 octets pour les variables locales.
  Le croquis utilise 269924 octets (25%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
  Les variables globales utilisent 27472 octets (33%) de mémoire dynamique, ce qui laisse 54448 octets pour les variables locales.
  e croquis utilise 270052 octets (25%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
  Les variables globales utilisent 27472 octets (33%) de mémoire dynamique, ce qui laisse 54448 octets pour les variables locales.
  e croquis utilise 269968 octets (25%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
  Les variables globales utilisent 27492 octets (33%) de mémoire dynamique, ce qui laisse 54428 octets pour les variables locales.
  e croquis utilise 269936 octets (25%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
  Les variables globales utilisent 27484 octets (33%) de mémoire dynamique, ce qui laisse 54436 octets pour les variables locales.
  e croquis utilise 269908 octets (25%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
  Les variables globales utilisent 27480 octets (33%) de mémoire dynamique, ce qui laisse 54440 octets pour les variables locales.
  e croquis utilise 8574 octets (27%) de l'espace de stockage de programmes. Le maximum est de 30720 octets.
  Les variables globales utilisent 357 octets (17%) de mémoire dynamique, ce qui laisse 1691 octets pour les variables locales.
  e croquis utilise 10858 octets (35%) de l'espace de stockage de programmes. Le maximum est de 30720 octets.
  Les variables globales utilisent 439 octets (21%) de mémoire dynamique, ce qui laisse 1609 octets pour les variables locales. Le maximum est de 2048 octets.


    *************************************************/

#define APP_NAME "betaEvents V2.0"

#if  defined(__AVR__)
#include <avr/wdt.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "betaEvents.h"
#include "evHandlerButton.h"
#include "evHandlerLed.h"
#include "evHandlerDebug.h"
#include "evHandlerSerial.h"

#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");

EventManager MyEvent;   // local instance de eventManager

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
  evBP0 = 100,
  evBP1,
  evLed0,
  evLed1,
  ev1S,
  ev2S,
  ev3S,
  // evenement action
  doReset,
};

#if  defined(__AVR__)
#define BP0 2  // D2
#define BP1 3  // D3
#define LED1 4
#elif defined(ESP8266) || defined(ESP32)
#define BP0 D1 // D1
#define BP1 D2 // D2
#define LED1 16

#endif

// instances poussoir
evHandlerButton MyBP0(evBP0, BP0);
evHandlerButton MyBP1(evBP1, BP1);

// instance LED
evHandlerLed    MyLed0(evLed0, LED_BUILTIN);
evHandlerLed    MyLed1(evLed1, LED1);

// instance Serial
evHandlerSerial MyKeyboard;

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
  Serial.begin(115200);
  Serial.println(F("\r\n\n" APP_NAME));

  // Start instance
  MyEvent.begin();
  MyEvent.addEventHandler(&MyKeyboard);
  MyEvent.addEventHandler(new evHandlerDebug );
  MyEvent.addEventHandler(&MyBP0);        // ajout d'un bouton sur BP0
  MyEvent.addEventHandler(&MyBP1);        // ajout d'un bouton sur BP1
  MyEvent.addEventHandler(&MyLed0);       // ajout LED0
  MyLed0.setFrequence(1, 10);
  MyEvent.addEventHandler(&MyLed1);       // ajout LED1
  Serial.println("Bonjour ....");
  //D_println(sizeof(EventManagerPtr));
}

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

    //    case evBP0Down:
    //      Serial.println(F("BP0 Down"));
    //      break;
    //
    //    case evBP0Up:
    //      Serial.println(F("BP0 Up"));
    //      break;
    //
    //    case evBP0LongDown:
    //      Serial.println(F("BP0 Long Down"));
    //      if (multi == 5) {
    //        Serial.println(F("RESET"));
    //        MyEvent.pushEvent(doReset);
    //      }
    //      break;
    //
    //    case evBP0LongUp:
    //      BP0Multi = 0;
    //      Serial.println(F("BP0 Long Up"));
    //      break;
    //
    //    case evBP0MultiDown:
    //      multi = MyEvent.currentEvent.param;
    //      Serial.print(F("BP0 Multi Clic:"));
    //      Serial.println(multi);
    //
    //      break;

    case evBP0:
      switch (MyEvent.currentEvent.param) {
        case evBPDown:
          MyLed0.setMillisec(500, 50);
          BP0Multi++;
          Serial.println(F("BP0 Down"));
          if (BP0Multi > 1) {
            D_println(BP0Multi);
          }
          break;
        case evBPUp:
          MyLed0.setMillisec(1000, 10);
          Serial.println(F("BP0 Up"));
          break;
        case evBPLongDown:
          if (BP0Multi == 5) {
            Serial.println(F("RESET"));
            MyEvent.pushEvent(doReset);
          }

          Serial.println(F("BP0 Long Down"));
          break;
        case evBPLongUp:
          BP0Multi = 0;
          Serial.println(F("BP0 Long Up"));
          break;

      }
      break;
    case evBP1:
      switch (MyEvent.currentEvent.param) {
        case evBPDown:
          MyLed1.setFrequence(3, 50);
          Serial.print(F("BP1 Down "));
          Serial.println(MyBP0.isDown() ? "and BP0 Down" : "and BP0 Up");
          break;
        case evBPUp:
          MyLed1.setOn(false);
          Serial.println(F("BP1 Up"));
          break;
        case evBPLongDown:
          MyLed1.setFrequence(1, 50);
          Serial.println(F("BP1 Long Down"));
          break;
        case evBPLongUp:  Serial.println(F("BP1 Long Up")); break;

      }
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
      switch (toupper(MyEvent.currentEvent.param))
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

      if (MyKeyboard.inputString.equals(F("S"))) {
        sleepOk = !sleepOk;
        Serial.print(F("Sleep=")); Serial.println(sleepOk);
      }

      if (MyKeyboard.inputString.equals(F("P"))) {
        Serial.println(F("Push 3 delay events"));
        Serial.print(F("Ram=")); Serial.println(MyEvent.freeRam());
        MyEvent.pushDelayEvent(1000, ev1S);
        MyEvent.pushDelayEvent(2000, ev2S);
        MyEvent.pushDelayEvent(3000, ev3S);
        Serial.print(F("Ram=")); Serial.println(MyEvent.freeRam());
      }
      if (MyKeyboard.inputString.equals(F("Q"))) {
        Serial.println(F("Push 3 events"));
        Serial.print(F("Ram=")); Serial.println(MyEvent.freeRam());
        MyEvent.pushDelayEvent(0, ev1S);
        MyEvent.pushDelayEvent(0, ev2S);
        MyEvent.pushDelayEvent(0, ev3S);
        Serial.print(F("Ram=")); Serial.println(MyEvent.freeRam());
      }


      if (MyKeyboard.inputString.equals(F("FREE"))) {
        Serial.print(F("Ram=")); Serial.println(MyEvent.freeRam());
      }

      if (MyKeyboard.inputString.equals(F("RESET"))) {
        Serial.println(F("RESET"));
        MyEvent.pushEvent(doReset);
      }

      break;

  }
}
