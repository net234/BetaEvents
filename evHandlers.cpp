/*************************************************
 *************************************************
    handler evHandlers.cpp   validation of lib betaEvents to deal nicely with events programing with Arduino
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
    - Mise en liste chainée de modules 'events' 
      evHandlerSerial   Gestion des caracteres et des chaines provenant de Serial
      evHandlerLed      Gestion d'une led avec ou sans clignotement sur un GPIO (Multiple instance possible)
      evHandlerButton   Gestion d'un pousoir sur un GPIO (Multiple instance possible)
      evHandlerDebug    Affichage de l'occupation CPU, de la memoire libre et des evenements 100Hz 10Hz et 1Hz

    *************************************************/ 
#include  "evHandlers.h"


/**********************************************************
 * 
 * gestion de Serial pour generer les   evInChar et  evInString
 * 
 ***********************************************************/

evHandlerSerial::evHandlerSerial(const uint32_t speed) {
  Serial.begin(speed);  // par defaut 115200
  EventManagerPtr->addGetEvent(this);
}

byte evHandlerSerial::getEvent()  {
  if (this->stringComplete)   {
    this->stringComplete = false;
    this->stringErase = true;      // la chaine sera effacee au prochain caractere recu
    EventManagerPtr->currentEvent.aStringPtr = &this->inputString;
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
    EventManagerPtr->currentEvent.aChar = this->inChar;
    return (EventManagerPtr->currentEvent.code = evInChar);
  }
  return (evNill);
}


/**********************************************************
 * 
 * gestion d'une Led sur un port   clignotement en frequence ou en millisecondes
 * 
 ***********************************************************/



evHandlerLed::evHandlerLed(const uint8_t aEventCode, const uint8_t aPinNumber) {
  this->pinNumber = aPinNumber;
  pinMode(aPinNumber, OUTPUT);
  this->evCode = aEventCode;
  this->setOn(false);
}

void evHandlerLed::handleEvent()  {
  if (EventManagerPtr->currentEvent.code == this->evCode) {
    switch (EventManagerPtr->currentEvent.ext) {

      case evxLedOff:
        digitalWrite(this->pinNumber, !this->levelON);   // led off
        break;

      case evxLedOn:
        digitalWrite(this->pinNumber, (this->percent == 0) ^ this->levelON );
        if (this->percent > 0 && this->percent < 100) {
          EventManagerPtr->pushDelayEvent(this->millisecondes, this->evCode, evxLedOn);
          EventManagerPtr->pushDelayEvent(this->millisecondes * this->percent / 100, this->evCode, evxLedOff, true);
        }
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
  EventManagerPtr->pushDelayEvent(0, this->evCode, (this->percent > 0) ? evxLedOn : evxLedOff );
}

void  evHandlerLed::setFrequence(const uint8_t frequence, const uint8_t percent) {
  this->setMillisec(1000U / frequence, percent);
}



/**********************************************************
 * 
 * gestion d'un poussoir sur un port   genere evBPDown, evBPUp, evBPLongDown, evBPLongUp
 * 
 ***********************************************************/



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
        EventManagerPtr->pushEvent(this->evCode, evxBPDown);
        EventManagerPtr->pushDelayEvent(2000, this->evCode, evxBPLongDown); // arme un event BP0 long down
      } else {
        EventManagerPtr->pushEvent(this->evCode, evxBPUp);
        EventManagerPtr->pushDelayEvent(1000, this->evCode, evxBPLongUp); // arme un event BP0 long up
      }
    }
  }
}


/**********************************************************
 * 
 * gestion d'un traceur de debugage touche 'T' pour visualiser la charge CPU
 * 
 ***********************************************************/


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

      this->ev10HzMissed += EventManagerPtr->currentEvent.aInt - 1;
      if (this->trackTime > 1 ) {

        if (EventManagerPtr->currentEvent.aInt > 1) {
          //        for (int N = 2; N<currentEvent.param; N++) Serial.print(' ');
          Serial.print('X');
          Serial.print(EventManagerPtr->currentEvent.aInt - 1);
        } else {
          Serial.print('|');
        }
      }
      break;

    case ev100Hz:
      this->ev100HzMissed += EventManagerPtr->currentEvent.aInt - 1;

      if (this->trackTime > 2)
      {

        if (EventManagerPtr->currentEvent.aInt > 1) {
          //      for (int N = 3; N<currentEvent.param; N++) Serial.print(' ');
          Serial.print('x');
          Serial.print(EventManagerPtr->currentEvent.aInt - 1);
        } else {
          Serial.print('_');
        }
      }
      break;
    case evInString:
      if (EventManagerPtr->currentEvent.aStringPtr->equals("T")) {
     //case evInChar: 
     //  if (EventManagerPtr->currentEvent.aChar == 'T') {
        if ( ++(this->trackTime) > 3 ) {

          this->trackTime = 0;
        }
        Serial.print("\nTrackTime=");
        Serial.println(this->trackTime);
      }

      break;
  }
};
