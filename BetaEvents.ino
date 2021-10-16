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
    - Mise en liste chainée de modules 'events' 
      evHandlerSerial   Gestion des caracteres et des chaines provenant de Serial
      evHandlerLed      Gestion d'une led avec ou sans clignotement sur un GPIO (Multiple instance possible)
      evHandlerButton   Gestion d'un pousoir sur un GPIO (Multiple instance possible)
      evHandlerDebug    Affichage de l'occupation CPU, de la memoire libre et des evenements 100Hz 10Hz et 1Hz



    *************************************************/
#pragma message "compile BetaEvents.ino"
#define APP_NAME "betaEvents V2.0"

#if  defined(__AVR__)
#include <avr/wdt.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif




#include "EventsManager.h"




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
evHandlerDebug  MyDebug;

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
//  Serial.begin(115200);
  Serial.println(F("\r\n\n" APP_NAME));
  // Start instance
  MyEvent.begin();
  MyLed0.setFrequence(1, 10);
  Serial.println("Bonjour ....");
  D_println(sizeof(stdEvent_t));
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


 
    case evBP0:
      switch (MyEvent.currentEvent.ext) {
        case evxBPDown:
          MyLed0.setMillisec(500, 50);
          BP0Multi++;
          Serial.println(F("BP0 Down"));
          if (BP0Multi > 1) {
            D_println(BP0Multi);
          }
          break;
        case evxBPUp:
          MyLed0.setMillisec(1000, 10);
          Serial.println(F("BP0 Up"));
          break;
        case evxBPLongDown:
          if (BP0Multi == 5) {
            Serial.println(F("RESET"));
            MyEvent.pushEvent(doReset);
          }

          Serial.println(F("BP0 Long Down"));
          break;
        case evxBPLongUp:
          BP0Multi = 0;
          Serial.println(F("BP0 Long Up"));
          break;

      }
      break;
    case evBP1:
      switch (MyEvent.currentEvent.ext) {
        case evxBPDown:
          MyLed1.setFrequence(3, 50);
          Serial.print(F("BP1 Down "));
          Serial.println(MyBP0.isDown() ? "and BP0 Down" : "and BP0 Up");
          break;
        case evxBPUp:
          MyLed1.setOn(false);
          Serial.println(F("BP1 Up"));
          break;
        case evxBPLongDown:
          MyLed1.setFrequence(1, 50);
          Serial.println(F("BP1 Long Down"));
          break;
        case evxBPLongUp:  Serial.println(F("BP1 Long Up")); break;

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
      switch (toupper(MyEvent.currentEvent.ext))
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
        D_println(*MyEvent.currentEvent.aStringPtr);
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
