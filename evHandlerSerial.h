/*************************************************
 *************************************************
    handler evHandlerSerial.h   validation of lib betaEvents to deal nicely with events programing with Arduino
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

    V2.0  20/04/2021
    - Mise en liste chainée de modules 'events' test avec un evButton



    *************************************************/


#pragma once
#include <Arduino.h>
#include  "betaEvents.h"


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


evHandlerSerial::evHandlerSerial(const uint32_t speed) {
  Serial.begin(speed);
}

byte evHandlerSerial::nextEvent()  {
  if (this->stringComplete)   {
    this->stringComplete = false;
    this->stringErase = true;      // la chaine sera effacee au prochain caractere recu
    return (EventManagerPtr->currentEvent.code = evInString);
  }
  if (Serial.available())   {
    this->inChar = Serial.read();
    if (this->stringErase) {
      this->inputString = "";
      this->stringErase = false;
    }
    if (isPrintable(this->inChar) && (this->inputString.length() < this->inputStringSizeMax)) {
      this->inputString += this->inChar;
    };
    if (this->inChar == '\n' || this->inChar == '\r') {
      this->stringComplete = (this->inputString.length() > 0);
    }
    EventManagerPtr->currentEvent.param = this->inChar;
    return (EventManagerPtr->currentEvent.code = evInChar);
  }
  return (evNill);
}
