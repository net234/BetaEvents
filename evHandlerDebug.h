/*************************************************
 *************************************************
    handler evHandlerDebug.h   validation of lib betaEvents to deal nicely with events programing with Arduino
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



class evHandlerDebug : public eventHandler_t {
  public:
    //evHandlerDebug();
    virtual void handleEvent()  override;

  private:
    uint16_t ev100HzMissed = 0;
    uint16_t ev10HzMissed = 0;
    uint8_t trackTime = 0;
};

void evHandlerDebug::handleEvent() {
  switch (EventManagerPtr->currentEvent.code) {
    case ev1Hz:

      if (this->trackTime) {

        char aBuffer[60];

        snprintf(aBuffer, 60 , " %02d:%02d:%02d,CPU=%d%%,Loop=%lu,Nill=%lu,Ram=%u", hour(), minute(), second(), EventManagerPtr->_percentCPU, EventManagerPtr->_loopParsec, EventManagerPtr->_evNillParsec, EventManagerPtr->freeRam());


        Serial.print(aBuffer);

        if (this->ev100HzMissed + this->ev10HzMissed) {
          sprintf(aBuffer, " Miss:%d/%d", this->ev100HzMissed, this->ev10HzMissed);
          Serial.print(aBuffer);
          this->ev100HzMissed = 0;
          this->ev10HzMissed = 0;
        }
        Serial.println();
      }

      break;

    case ev10Hz:

      this->ev10HzMissed += EventManagerPtr->currentEvent.param - 1;
      if (this->trackTime > 1 ) {

        if (EventManagerPtr->currentEvent.param > 1) {
          //        for (int N = 2; N<currentEvent.param; N++) Serial.print(' ');
          Serial.print('X');
          Serial.print(EventManagerPtr->currentEvent.param - 1);
        } else {
          Serial.print('|');
        }
      }
      break;

    case ev100Hz:
      this->ev100HzMissed += EventManagerPtr->currentEvent.param - 1;

      if (this->trackTime > 2)
      {

        if (EventManagerPtr->currentEvent.param > 1) {
          //      for (int N = 3; N<currentEvent.param; N++) Serial.print(' ');
          Serial.print('x');
          Serial.print(EventManagerPtr->currentEvent.param - 1);
        } else {
          Serial.print('_');
        }
      }
      break;
   // case evInString:
      //if (EventManagerPtr->inputString.equals("T")) {
     case evInChar: 
       if (EventManagerPtr->currentEvent.param == 'T') {
        if ( ++(this->trackTime) > 3 ) {

          this->trackTime = 0;
        }
        Serial.print("\nTrackTime=");
        Serial.println(this->trackTime);
      }

      break;
  }
};
