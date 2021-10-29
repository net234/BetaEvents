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

    V2.2  27/10/2021
       more arduino like lib with self built in instance


 *************************************************/

#define APP_NAME "event_demo V2.2"

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


//  betaEvent.h est une aide pour construire les elements de base d'une programation evenementiel

//  une instance "Events" avec un poussoir "BP0" une LED "Led0" un clavier "Keyboard"
//  BP0 genere un evenement evBP0 a chaque pression le poussoir connecté sur BP0_PIN
//  Led0 genere un evenement evLed0 a chaque clignotement de la led precablée sur la platine LED_BUILTIN
//  Keyboard genere un evenement evChar a char caractere recu et un evenement evString a chaque ligne recue
//  Debug permet sur reception d'un "T" sur l'entrée Serial d'afficher les infos de charge du CPU

//#define BP0_PIN   5                 //   Par defaut BP0 est sur D5
//#define Led0_PIN  LED_BUILTIN     //   Par defaut Led0 est sur LED_BUILTIN
#include <BetaEvents.h>

int  multi = 0; // nombre de clic rapide


void setup() {

  Serial.begin(115200);
  Serial.println(F("\r\n\n" APP_NAME));

  // Start instance
  Events.begin();
  //pinMode(pinBP0, INPUT_PULLUP);
  Serial.println("Bonjour ....");
  //D_println(LED_BUILTIN);

}

byte BP0Multi = 0;  // detection de click rapide sur le poussoir
bool sleepOk = true;

void loop() {

  Events.get(sleepOk);         // generation du prochain evenement
  Events.handle();      // passage de l'evenement au systeme
  switch (Events.code)  // gestion de l'evenement
  {
    case evInit:
      Serial.println("Init");
      break;


    case ev24H:
      Serial.println("---- 24H ---");
      break;



    case evBP0:
      switch (Events.ext) {
        case evxBPDown:
          Led0.setMillisec(500, 50);
          BP0Multi++;
          Serial.println(F("BP0 Down"));
          if (BP0Multi > 1) {
            Serial.print(F("BP0 Multi ="));
            Serial.println(BP0Multi);
          }
          break;
        case evxBPUp:
          Led0.setMillisec(1000, 10);
          Serial.println(F("BP0 Up"));
          break;
        case evxBPLongDown:
          if (BP0Multi == 5) {
            Serial.println(F("RESET"));
            Events.push(doReset);
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
        if (Debug.trackTime < 2) {
          char aChar = Events.aChar;
          if (isPrintable(aChar)) {
            D_println(Keyboard.inputChar);
          } else {
            D_println(int(aChar));
          }
        }
        switch (toupper(Events.aChar))
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
            if (Debug.trackTime < 2) {
              D_println(Keyboard.inputString);
            }
            if (Keyboard.inputString.equals("RESET")) {
              Serial.println(F("RESET"));
              Events.push(doReset);
            }
            if (Keyboard.inputString.equals("S")) {
//            if (Events.aStringPtr->equals("S")) {
  
              sleepOk = !sleepOk;
              D_println(sleepOk);
            }
      
            break;

  }
}
