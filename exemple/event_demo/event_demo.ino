/*************************************************
 *************************************************
    Sketch event_demo.ino   validation of lib betaEvents to deal nicely with events programing with Arduino
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


  e croquis utilise 267736 octets (25%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
  Les variables globales utilisent 27348 octets (33%) de mémoire dynamique, ce qui laisse 54572 octets pour les
e croquis utilise 267904 octets (25%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
Les variables globales utilisent 27300 octets (33%) de mémoire dynamique, ce qui laisse 54620 octets pour les variables locales. Le maximum est de 81920 oct
 *************************************************/

#define APP_NAME "event_demo V2.0"

/* Evenements du Manager (voir EventsManager.h)
  evNill = 0,      // No event  about 1 every milisecond but do not use them for delay Use pushDelayEvent(delay,event)
  ev100Hz,         // tick 100HZ    non cumulative (see betaEvent.h)
  ev10Hz,          // tick 10HZ     non cumulative (see betaEvent.h)
  ev1Hz,           // un tick 1HZ   cumulative (see betaEvent.h)
  ev24H,           // 24H when timestamp pass over 24H
  evInit,
  evInChar,
  evInString,
*/

// Liste des evenements specifique a ce projet
enum tUserEventCode {
  // evenement utilisateurs
  evBP0 = 100,
  evLed0,
  // evenement action
  doReset,
};


// instance betaEvent

//  une instance "MyEvents" avec un poussoir "MyBP0" une LED "MyLed0" un clavier "MyKeyboard"
//  MyBP0 genere un evenement evBP0 a chaque pression le poussoir connecté sur D2
//  MyLed0 genere un evenement evLed0 a chaque clignotement de la led precablée sur la platine
//  MyKeyboard genere un evenement evChar a char caractere recu et un evenement evString a chaque ligne recue
//  MyDebug permet sur reception d'un "T" sur l'entrée Serial d'afficher les infos de charge du CPU

#define pinBP0   8                 //   By default BP0 is on D2 you can change it
//#define pinLed0  3 //LED_BUILTIN   //   By default Led0 is on LED_BUILTIN you can change it
#include <BetaEvents.h>



bool sleepOk = true;
int  multi = 0; // nombre de clic rapide


void setup() {

  Serial.begin(115200);
  Serial.println(F("\r\n\n" APP_NAME));

  // Start instance
  MyEvents.begin();


  Serial.println("Bonjour ....");
  //D_println(LED_BUILTIN);
}

byte BP0Multi = 0;


void loop() {

  MyEvents.getEvent(sleepOk);
  MyEvents.handleEvent();
  switch (MyEvents.currentEvent.code)
  {


    case evInit:
      Serial.println("Init");
      break;


    case ev24H:
      Serial.println("---- 24H ---");
      break;



    case evBP0:
      switch (MyEvents.currentEvent.ext) {
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
            MyEvents.pushEvent(doReset);
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
      helperReset();
      break;


    case evInChar: {
        if (MyDebug.trackTime < 2) {
          char aChar = MyKeyboard.inputChar;
          if (isPrintable(aChar)) {
            D_println(aChar);
          } else {
            D_println(int(aChar));
          }
        }
        switch (toupper(MyKeyboard.inputChar))
        {
          case '0': delay(10); break;
          case '1': delay(100); break;
          case '2': delay(200); break;
          case '3': delay(300); break;
          case '4': delay(400); break;
          case '5': delay(500); break;
        }


        break;

      }

    case evInString:
      if (MyDebug.trackTime < 2) {
        D_println(MyKeyboard.inputString);
      }
      if (MyKeyboard.inputString.equals("RESET")) {
        Serial.println(F("RESET"));
        MyEvents.pushEvent(doReset);
      }
      if (MyKeyboard.inputString.equals("S")) {
        sleepOk = !sleepOk;
        D_println(sleepOk);
      }

      break;

  }
}
