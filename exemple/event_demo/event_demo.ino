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

 *************************************************/

#define APP_NAME "event_demo V2.0"

/* Evenements du Manager (voir EventsManager.h)
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
  // evenement utilisateurs
//  evBP0 = 100,
//  evBP1,
//  evLed0,
//  evLed1,

  // evenement action
  doReset,
};

//#if  defined(__AVR__)
//#define BP0 2  // D2
//#define BP1 3  // D3
//#define LED1 4
//#elif defined(ESP8266) || defined(ESP32)
#define BP0 D1 // D1
//#define BP1 D2 // D2
//#define LED1 16
//
//#endif

// instance betaEvent

//  une instance "betaEvents" avec un poussoir "MyBP0" une LED "MyLed0" un clavier "MyKeyboard"
//  MyBP0 genere un evenement evBP0 a chaque pression
//  MyLed0 genere un evenement evLed0 a chaque clignotement

#include "BetaEvents.h"



bool sleepOk = true;
int  multi = 0; // nombre de clic rapide


void setup() {

  // Start instance
  BetaEvents.begin();

//  Serial.begin(115200);
  Serial.println(F("\r\n\n" APP_NAME));

  Serial.println("Bonjour ....");

}

byte BP0Multi = 0;


void loop() {

  BetaEvents.getEvent(sleepOk);
  BetaEvents.handleEvent();
  switch (BetaEvents.currentEvent.code)
  {

    case ev24H:
      Serial.println("---- 24H ---");
      break;


 
    case evBP0:
      switch (BetaEvents.currentEvent.ext) {
        case evxBPDown:
          MyLed0.setMillisec(500, 50);
          BP0Multi++;
          Serial.println(F("BP0 Down"));
          if (BP0Multi > 1) {
            Serial.print(F("BP0 Multi ="));
            Serial.println(BP0Multi);
          }
          break;
        case evxBPUp:
          MyLed0.setMillisec(1000, 10);
          Serial.println(F("BP0 Up"));
          break;
        case evxBPLongDown:
          if (BP0Multi == 5) {
            Serial.println(F("RESET"));
            BetaEvents.pushEvent(doReset);
          }

          Serial.println(F("BP0 Long Down"));
          break;
        case evxBPLongUp:
          BP0Multi = 0;
          Serial.println(F("BP0 Long Up"));
          break;

      }
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
      switch (toupper(BetaEvents.currentEvent.aChar))
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

      if (MyKeyboard.inputString.equals(F("RESET"))) {
        Serial.println(F("RESET"));
        BetaEvents.pushEvent(doReset);
      }

      break;

  }
}
