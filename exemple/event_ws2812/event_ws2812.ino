/**************************************************************************************
  event_ws2812.ino

   Gestion d'un bandeau de led  WS2812 pour faire des animations
   Copyright 10/2023 Pierre HENRY net23@frdev.com

   event_ws2812 is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bandeau is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with betaEvents.  If not, see <https://www.gnu.org/licenses/lglp.txt>.


  History
   V1.0 (30/10/2021)
   - Full rebuild from Bandeau  01/2021 net234
   V1.1 (29/09/2022)
   - Ajustement pour version Meluze

*************************************************/


#include <Arduino.h>
#include "ESP8266.h"
#include "WS2812.h"


#define APP_VERSION "event_ws2812 V1.0"

//enum typeEvent { evNill=0, ev100Hz, ev10Hz, ev1Hz, ev24H, evInChar,evInString,evUser=99 };
enum myEvent {
  evBP0 = 100,
  evLed0,
  //evSaveDisplayMode,
  evDisplayOff,  //Extinction
  evStartAnim,   //Allumage Avec l'animation Mode1 et activation de l'extinction automatique
  evNextAnim,    // fin de l'animation en cours
  evNextStep,    //etape suivante dans l'animation

};

// Gestionaire d'evenemnt
#define DEBUG_ON
#include <BetaEvents.h>



// varibale modifiables
const uint8_t ledsMAX = 20;        // nombre de led sur le bandeau
const uint8_t autoOffDelay = 60;  // delais d'auto extinction en secondes (0 = pas d'autoextinction)
// varibale modifiables (fin)







enum modeDsp_t : byte { modeOff,
                        modeFeu,
                        modeGlace,
                        modeVent,
                        modeTerre,
                        modeLumiere,
                        modeTenebre,
                        maxMode } currentMode = modeFeu;
uint8_t displayStep = 0;    // Etape en cours dans l'anime (de 0 a ledsMax-1)
e_rvb baseColor = rvb_red;  // Couleur base de l'animation
uint16_t speedAnim = 200;

int multiPush = -1;
bool sleepOk = true;

modeDsp_t displayMode;


WS2812rvb_t ledFixe1;
WS2812rvb_t ledFixe2;
WS2812rvb_t ledFixe3;
// Array contenant les leds du medaillon
WS2812rvb_t leds[ledsMAX];
// deux leds en rab pour la programmation
//WS2812rvb_t ledsM1;
//WS2812rvb_t ledsM2;
#include <EEPROM.h>




void setup() {

  //Serial.begin(115200);
  //Serial.println("Bonjour");
  Events.begin();
  Serial.println(F(APP_VERSION));

  //init eeprom a 16 caracteres pour l'ESP  (2 seulement sont utilisé)
  #if defined(ESP8266)
  #define EEPROM_SIZE 16
  EEPROM.begin(EEPROM_SIZE);

  #endif 


  //  toute les led a blanc a l'init
  pinMode(WS2812_PIN, OUTPUT);
  ledFixe1.setcolor(rvb_red,80,5000,5000);
  ledFixe2.setcolor(rvb_orange,80,5000,5000);
  ledFixe3.setcolor(rvb_green,80,5000,5000);
  for (uint8_t N = 0; N < ledsMAX; N++) {
    leds[N].setcolor(rvb_white, 80, 2000, 2000);
  }

  // recup des modes memorises (mode1 et mode2)
  getDisplayMode();
}



// the loop function runs over and over again forever
void loop() {
  Events.get(sleepOk);  // get standart event
  Events.handle();      // handle standart event

  switch (Events.code) {

    case evInit:
      Serial.println(F("Init Ok"));
      multiPush = -1;

      Events.delayedPush(5000, evNextAnim);


      break;


    case ev100Hz:
      // refresh led
      jobRefreshLeds(10);

      break;




    case evDisplayOff:
      currentMode = modeOff;
      Events.removeDelayEvent(evStartAnim);
      break;




    // mise en route des animations
    case evStartAnim:
      if (autoOffDelay) Events.delayedPush(1000L * autoOffDelay, evDisplayOff);
      jobStartAnim();
      TD_println("Started Anim", currentMode);
      break;

    // passage a l'animation suivante
    case evNextAnim:
      jobStartAnim();
      break;

    case evNextStep:
      jobNextStep();
      break;




    case evBP0:
      {

        switch (Events.ext) {

          case evxOff:
            Serial.println(F("BP0 Up"));

            D_println(displayMode);
            displayMode = (modeDsp_t)(displayMode + 1);
            if (displayMode >= maxMode) displayMode = modeFeu;
            D_println(displayMode);
            jobStartAnim();


            break;


          case evxOn:

            Serial.print(F("BP0 Down "));

            multiPush++;
            D_println(multiPush);

            if (multiPush == 1 or multiPush == 2) {
              if (currentMode) {
                currentMode = modeOff;
              } else {
                currentMode = displayMode;
              }
              Events.removeDelayEvent(evNextAnim);
              if (currentMode) Events.push(evStartAnim);
            }


            break;

          case evxLongOn:
            Serial.println(F("BP0 Long Down"));
            saveDisplayMode();

            break;

          case evxLongOff:
            Serial.println(F("BP0 Long Off"));
            multiPush = 0;
            break;
        }
        break;
      }

    case evInChar:
      {
        if (Keyboard.inputChar == '1') {
          displayMode = modeLumiere;
        }

        if (Keyboard.inputChar == '2') {
          displayMode = modeTenebre;
        }


        if (Keyboard.inputChar == 'S') {
          sleepOk = !sleepOk;
          D_println(sleepOk);
        }

        break;
      }
  }


  //delay(1);
}
