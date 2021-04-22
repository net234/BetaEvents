/*************************************************
 *************************************************
    handler evHandlerled.h   handle Led betaEvents to deal nicely with events programing with Arduino
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

   works with beteEvents 2.0

    V2.0  20/04/2020
    - Mise en liste chainée de modules 'events' test avec un evButton



    *************************************************/


#pragma once
#include <Arduino.h>
#include  "betaEvents.h"

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


evHandlerLed::evHandlerLed(const uint8_t aEventCode, const uint8_t aPinNumber) {
  this->pinNumber = aPinNumber;
  pinMode(aPinNumber, OUTPUT);
  this->evCode = aEventCode;
  this->setOn(false);
}

void evHandlerLed::handleEvent()  {
  if (EventManagerPtr->currentEvent.code == this->evCode) {
    switch (EventManagerPtr->currentEvent.param) {

      case evpLedOff:
        digitalWrite(this->pinNumber, !this->levelON);   // led off
        Serial.println("LEDOFF");
        break;

      case evpLedOn:
        digitalWrite(this->pinNumber, (this->percent == 0) ^ this->levelON );
        if (this->percent > 0 && this->percent < 100) {
          EventManagerPtr->pushDelayEvent(this->millisecondes, this->evCode, evpLedOn);
          EventManagerPtr->pushDelayEvent(this->millisecondes * this->percent / 100, this->evCode, evpLedOff, true);
        }
        Serial.println("LEDON");
        break;
    }
  }
}

void  evHandlerLed::setOn(const bool status) {
  setMillisec(1000, status ? 100 : 0);
  digitalWrite(this->pinNumber, status ^ this->levelON );  // make result instant if event delayed (not realy needed)
}


void  evHandlerLed::setMillisec(const uint16_t millisecondes, const uint8_t percent) {
  this->millisecondes = max(millisecondes, (uint16_t)2);
  this->percent = percent;
  EventManagerPtr->pushDelayEvent(0, this->evCode, (this->percent > 0) ? evpLedOn : evpLedOff );
}

void  evHandlerLed::setFrequence(const uint8_t frequence, const uint8_t percent) {
  this->setMillisec(1000U / frequence, percent);
}
