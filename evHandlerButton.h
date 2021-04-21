/*************************************************
 *************************************************
    hadler evHandlerButton.h   validation of lib betaEvents to deal nicely with events programing with Arduino
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

  private:
    uint8_t pinNumber;
    uint8_t evCode;
    bool    BPDown = true;

};


evHandlerButton::evHandlerButton(const uint8_t aEventCode, const uint8_t aPinNumber) {
  this->pinNumber = aPinNumber;
  pinMode(aPinNumber, INPUT_PULLUP);
  this->evCode = aEventCode;
}

void evHandlerButton::handleEvent()  {
  if (EventManagerPtr->currentEvent.code == ev10Hz) {
    if ( this->BPDown != (digitalRead(this->pinNumber) == LOW)) { // changement d'etat BP0
      this->BPDown = !this->BPDown;
      if (this->BPDown) {
        EventManagerPtr->pushEvent(this->evCode, evBPDown);
        EventManagerPtr->pushDelayEvent(3000, this->evCode, evBPLongDown); // arme un event BP0 long down
      } else {
        EventManagerPtr->pushEvent(this->evCode, evBPUp);
        EventManagerPtr->pushDelayEvent(1000, this->evCode, evBPLongUp); // arme un event BP0 long up
      }
    }
  }
}
