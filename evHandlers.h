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
    V2.0.1  26/10/2021
      corections evHandlerLed sur le true/false
    V2.2  27/10/2021
       more arduino like lib with self built in instance
    V2.2a  11/11/2021 
       add begin in evHandles  


    *************************************************/
#pragma once
#include <Arduino.h>
#include  "EventsManager.h"



/**********************************************************
 * 
 * gestion d'une Led sur un port   clignotement en frequence ou en millisecondes
 * 
 ***********************************************************/


enum tLedEventParam  {
  // evenement recu
  evxLedOff,           // Led Off
  evxLedOn,            // Led On
};


class evHandlerLed : public eventHandler_t {
  public:
    evHandlerLed(const uint8_t aEventCode, const uint8_t aPinNumber, const bool ledOn = HIGH, const uint8_t frequence = 0);
    virtual void handle()  override;
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
    bool    levelON = false;

};


/**********************************************************
 * 
 * gestion d'un poussoir sur un port   genere evBPDown, evBPUp, evBPLongDown, evBPLongUp
 * 
 ***********************************************************/


enum tBPEventParam  {
  // evenement recu
  evxBPDown,         // BP0 est appuyé
  evxBPUp,            // BP0 est relaché
  evxBPLongDown,      // BP0 est maintenus appuyé plus de 3 secondes
  evxBPLongUp,        // BP0 est relaché plus de 3 secondes
};


class evHandlerButton : public eventHandler_t {
  public:
    evHandlerButton(const uint8_t aEventCode, const uint8_t aPinNumber, const uint16_t aLongDelay = 1500);
    virtual void handle()  override;
    bool isDown()  {return BPDown; };

  private:
    uint8_t pinNumber;
    uint8_t evCode;
    bool    BPDown = false;
    uint16_t longDelay;
};

#ifndef __AVR_ATtiny85__
/**********************************************************
 * 
 * gestion de Serial pour generer les   evInChar et  evInString
 * 
 ***********************************************************/

class evHandlerSerial : public eventHandler_t {
  public:
    evHandlerSerial(const uint8_t inputStringSize = 20);
    //virtual void handleEvent()  override;
    virtual byte get()  override;
    String inputString = "";
    char   inputChar = '\0';
  private:
    uint8_t inputStringSizeMax = 20;
    bool stringComplete = false;
    bool stringErase = false;

};





/**********************************************************
 * 
 * gestion d'un traceur de debugage touche 'T' pour visualiser la charge CPU
 * 
 ***********************************************************/



class evHandlerDebug : public eventHandler_t {
  public:
    //evHandlerDebug();
    virtual void handle()  override;
    uint8_t trackTime = 0;
  private:
    uint16_t ev100HzMissed = 0;
    uint16_t ev10HzMissed = 0;

};

#endif
