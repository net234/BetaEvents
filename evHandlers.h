/*************************************************
 *************************************************
    handler evHandlers.h   validation of lib betaEvents to deal nicely with events programing with Arduino
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

    V2.0  20/04/2021
    - Mise en liste chainée de modules 'events' 
      evHandlerSerial   Gestion des caracteres et des chaines provenant de Serial
      evHandlerLed      Gestion d'une led avec ou sans clignotement sur un GPIO (Multiple instance possible)
      evHandlerButton   Gestion d'un pousoir sur un GPIO (Multiple instance possible)
      evHandlerDebug    Affichage de l'occupation CPU, de la memoire libre et des evenements 100Hz 10Hz et 1Hz

   History

   works with beteEvents 2.0

    V2.0  20/04/2021
    - Mise en liste chainée de modules 'events' test avec un evButton



    *************************************************/

#pragma once
#include <Arduino.h>
#include  "betaEvents.h"

/**********************************************************
 * 
 * gestion de Serial pour generer les   evInChar et  evInString
 * 
 ***********************************************************/

class evHandlerSerial : public eventHandler_t {
  public:
    evHandlerSerial(const uint32_t speeed = 115200);
    //virtual void handleEvent()  override;
    virtual byte nextEvent()  override;
    String inputString = "";
    char  inChar = '\0';
  private:
    const byte inputStringSizeMax = 50;
    bool stringComplete = false;
    bool stringErase = false;

};


/**********************************************************
 * 
 * gestion d'une Led sur un port   clignotement en frequence ou en millisecondes
 * 
 ***********************************************************/


enum tLedEventParam  {
  // evenement recu
  evpLedOff,           // Led Off
  evpLedOn,            // Led On
};


class evHandlerLed : public eventHandler_t {
  public:
    evHandlerLed(const uint8_t aEventCode, const uint8_t aPinNumber);
    virtual void handleEvent()  override;
    bool isOn()  {
      return ledOn;
    };
    void   setOn(const bool status = true);
    void   setFrequence(const uint8_t frequence, const uint8_t percent = 10); // frequence de la led
    void   setMillisec(const uint16_t millisecondes, const uint8_t percent = 10); // frequence de la led


  private:
    uint8_t pinNumber;
    uint8_t evCode;
    uint16_t millisecondes;
    uint8_t percent;
    bool    ledOn = false;
    const bool   levelON = false;

};


/**********************************************************
 * 
 * gestion d'un poussoir sur un port   genere evBPDown, evBPUp, evBPLongDown, evBPLongUp
 * 
 ***********************************************************/


enum tBPEventParam  {
  // evenement recu
  evBPDown,         // BP0 est appuyé
  evBPUp,            // BP0 est relaché
  evBPLongDown,      // BP0 est maintenus appuyé plus de 3 secondes
  evBPLongUp,        // BP0 est relaché plus de 3 secondes
};


class evHandlerButton : public eventHandler_t {
  public:
    evHandlerButton(const uint8_t aEventCode, const uint8_t aPinNumber);
    virtual void handleEvent()  override;
    bool isDown()  {return BPDown; };

  private:
    uint8_t pinNumber;
    uint8_t evCode;
    bool    BPDown = false;

};

/**********************************************************
 * 
 * gestion d'un traceur de debugage touche 'T' pour visualiser la charge CPU
 * 
 ***********************************************************/



class evHandlerDebug : public eventHandler_t {
  public:
    //evHandlerDebug();
    virtual void handleEvent()  override;

  private:
    uint16_t ev100HzMissed = 0;
    uint16_t ev10HzMissed = 0;
    uint8_t trackTime = 0;
};
