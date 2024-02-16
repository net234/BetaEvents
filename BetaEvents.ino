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
    V2.0.1  26/10/2021
      corections evHandlerLed sur le true/false
    V2.2  27/10/2021
       more arduino like lib with self built in instance
    V2.2a  11/11/2021 
       add begin in evHandles  

    V2.3    09/03/2022   isolation of evHandler for compatibility with dual core ESP32
    V2.4    30/09/2022   Isolation des IO (evhandlerOutput)
    V3.0    Octobre 2024   Preparation d'un fork  BetaEvents / BetaEvent32
     corrrection keyboard pour forcer une ligne de commande : setInputString(aStr
    V3.0.B2   10/02/2024  insertion evHandlerUDP   en cas de probleme ajouter un #NO_UDP 
    *************************************************/

#define APP_NAME "betaEvents V3.0.B2"

#if  defined(__AVR__)
#include <avr/wdt.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif



#define DEFAULT_PIN 

#include "EventsManager.h"
// Preinstantiate Objects /// as Nicolas Zambetti with Wire.cpp /////

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
#define BP0_PIN 5
#define BP1_PIN 6
#define LED1_PIN 4
#elif defined(ESP8266) || defined(ESP32)
#define BP0_PIN D5 // D1
#define BP1_PIN D6 // D2
#define LED1_PIN D0 // GPIO16

#endif

// instances poussoir
evHandlerButton BP0(evBP0, BP0_PIN);
evHandlerButton BP1(evBP1, BP1_PIN);
evHandlerDebug  Debug;

// instance LED
evHandlerLed    Led0(evLed0, LED_BUILTIN, HIGH);
evHandlerLed    Led1(evLed1, LED1_PIN, HIGH);

// instance Serial
evHandlerSerial Keyboard;

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
  Serial.println(F("Test1"));
  // Start instance
  Events.begin();
  Serial.println(F("Test2"));
  Led0.setFrequence(1, 10);
    Serial.println(F("Test3"));
  Led1.setMillisec(2000, 10);
    Serial.println(F("Test4"));
  Serial.println("Bonjour ....");
  D_println(sizeof(stdEvent_t));
  D_println(sizeof(size_t));
}

byte BP0Multi = 0;

//int ev1000HzCnt = 0;
//int ev100HzCnt = 0;
//int ev10HzCnt = 0;


void loop() {
  // test
  Events.get(sleepOk);
  Events.handle();
  switch (Events.code)
  {



    case evInit: {
        Serial.println("ev init");
      }
      break;
      
    case ev24H: {
        Serial.println("---- 24H ---");
        int aDay = Events.ext;
        D_println(aDay);
      }
      break;



    case evBP0:
      switch (Events.ext) {
        case evxOn:
          Led0.setMillisec(500, 50);
          BP0Multi++;
          Serial.println(F("BP0 Down"));
          if (BP0Multi > 1) {
            D_println(BP0Multi);
          }
          break;
        case evxOff:
          Led0.setMillisec(1000, 10);
          Serial.println(F("BP0 Up"));
          break;
        case evxLongOn:
          if (BP0Multi == 5) {
            Serial.println(F("RESET"));
            Events.push(doReset);
          }

          Serial.println(F("BP0 Long Down"));
          break;
        case evxLongOff:
          BP0Multi = 0;
          Serial.println(F("BP0 Long Up"));
          break;

      }
      break;
    case evBP1:
      switch (Events.ext) {
        case evxOn:
          Led1.setFrequence(3, 50);
          Serial.print(F("BP1 Down "));
          Serial.println(BP0.isOn() ? "and BP0 Down" : "and BP0 Up");
          break;
        case evxOff :
          Led1.setOn(false);
          Serial.println(F("BP1 Up"));
          break;
        case evxLongOn:
          Led1.setFrequence(1, 50);
          Serial.println(F("BP1 Long Down"));
          break;
        case evxLongOff:  Serial.println(F("BP1 Long Up")); break;

      }
      break;



    case ev1S:
      Serial.print(F("Ram=")); Serial.println(Events.freeRam());
      Serial.println(F("EV1S"));
      break;
    case ev2S:
      Serial.print(F("Ram=")); Serial.println(Events.freeRam());
      Serial.println(F("EV2S"));
      break;
    case ev3S:
      Serial.print(F("Ram=")); Serial.println(Events.freeRam());
      Serial.println(F("EV3S"));
      break;

    case doReset:
      Events.reset();
      break;


    case evInChar:
      switch (toupper(Events.charExt))
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

      if (Keyboard.inputString.equals(F("S"))) {
        sleepOk = !sleepOk;
        Serial.print(F("Sleep=")); Serial.println(sleepOk);
        D_println(*Events.StringPtr);
      }

      if (Keyboard.inputString.equals(F("O"))) {
        Serial.println(F("Push 3 delay events"));
        Serial.print(F("Ram=")); Serial.println(Events.freeRam());
        Events.delayedPush(500, ev1S);
        Events.delayedPush(11 * 1000, ev2S);
        Events.delayedPush(11L * 60  * 1000, ev3S);
        Serial.print(F("Ram=")); Serial.println(Events.freeRam());
      }
      if (Keyboard.inputString.equals(F("P"))) {
        Serial.println(F("Push 3 delay events"));
        Serial.print(F("Ram=")); Serial.println(Events.freeRam());
        Events.delayedPush(1000, ev1S);
        Events.delayedPush(2000, ev2S);
        Events.delayedPush(3000, ev3S);
        Serial.print(F("Ram=")); Serial.println(Events.freeRam());
      }
      if (Keyboard.inputString.equals(F("Q"))) {
        Serial.println(F("Push 3 events"));
        Serial.print(F("Ram=")); Serial.println(Events.freeRam());
        Events.delayedPush(0, ev1S);
        Events.delayedPush(0, ev2S);
        Events.delayedPush(0, ev3S);
        Serial.print(F("Ram=")); Serial.println(Events.freeRam());
      }


      if (Keyboard.inputString.equals(F("FREE"))) {
        Serial.print(F("Ram=")); Serial.println(Events.freeRam());
      }

      if (Keyboard.inputString.equals(F("RESET"))) {
        Serial.println(F("RESET"));
        Events.push(doReset);
      }

      break;

  }
}
