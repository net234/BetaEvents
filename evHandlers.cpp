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
    V2.0.1  26/10/2021
      corections evHandlerLed sur le true/false
    V2.2  27/10/2021
       more arduino like lib with self built in instance
    V2.2a  11/11/2021
       add begin in evHandles

    V2.3    09/03/2022   isolation of evHandler for compatibility with dual core ESP32

    *************************************************/
#include "evHandlers.h"


/**********************************************************

   gestion d'un output generique

 ***********************************************************/

evHandlerOutput::evHandlerOutput(const uint8_t aEventCode, const uint8_t aPinNumber, const bool aStateON)
  : pinNumber(aPinNumber), stateON(aStateON), evCode(aEventCode) {
  pinMode(aPinNumber, OUTPUT);
};

void evHandlerOutput::setOn(const bool status) {
  state = status;
  digitalWrite(pinNumber, (not status) xor stateON);  
}




/**********************************************************

   gestion d'une Led sur un port   clignotement en frequence ou en millisecondes

 ***********************************************************/

evHandlerLed::evHandlerLed(const uint8_t aEventCode, const uint8_t aPinNumber, const bool revert, const uint8_t frequence)
  : evHandlerOutput(aEventCode,aPinNumber,revert) {
  setFrequence(frequence);
};

void evHandlerLed::handle() {
  if (evManager.code == evCode) {
    switch (evManager.ext) {
      case evxLedOff:
        digitalWrite(pinNumber, not levelON);  // led off
        break;

      case evxLedOn:
        digitalWrite(pinNumber, (percent == 0) xor levelON);
        if (percent > 0 && percent < 100) {
          evManager.delayedPush(millisecondes, evCode, evxLedOn);
          evManager.delayedPush(millisecondes * percent / 100, evCode, evxLedOff, true);
        }
        break;
    }
  }
}

void evHandlerLed::setOn(const bool status) {
  setMillisec(1000, status ? 100 : 0);
  digitalWrite(pinNumber, (not status) xor levelON);  // make result instant needed  outside event loop
}


void evHandlerLed::setMillisec(const uint16_t aMillisecondes, const uint8_t aPercent) {
  millisecondes = max(aMillisecondes, (uint16_t)2);
  percent = aPercent;
  evManager.delayedPush(0, evCode, (percent > 0) ? evxLedOn : evxLedOff);
}

void evHandlerLed::setFrequence(const uint8_t frequence, const uint8_t percent) {
  if (frequence == 0) {
    setOn(false);
    return;
  }
  setMillisec(1000U / frequence, percent);
}

void evHandlerLed::pulse(const uint32_t aDelay) {  // pulse d'allumage simple
  if (aDelay == 0) {
    setOn(false);
    return;
  }
  setOn(true);
  evManager.delayedPush(aDelay, evCode, evxLedOff);
}
/**********************************************************

   gestion d'un poussoir sur un port   genere evBPDown, evBPUp, evBPLongDown, evBPLongUp

 ***********************************************************/



evHandlerButton::evHandlerButton(const uint8_t aEventCode, const uint8_t aPinNumber, const uint16_t aLongDelay)
  : pinNumber(aPinNumber), evCode(aEventCode), longDelay(aLongDelay) {
  pinMode(pinNumber, INPUT_PULLUP);
};


void evHandlerButton::handle() {
  if (evManager.code == ev10Hz) {
    if (BPDown != (digitalRead(pinNumber) == LOW)) {  // changement d'etat BP0
      BPDown = !BPDown;
      if (BPDown) {
        evManager.push(evCode, evxBPDown);
        evManager.delayedPush(longDelay, evCode, evxBPLongDown);  // arme un event BP0 long down
      } else {
        evManager.push(evCode, evxBPUp);
        evManager.delayedPush(longDelay, evCode, evxBPLongUp);  // arme un event BP0 long up
      }
    }
  }
}

#ifndef __AVR_ATtiny85__
/**********************************************************

   gestion de Serial pour generer les   evInChar et  evInString

 ***********************************************************/

evHandlerSerial::evHandlerSerial(const uint32_t aSerialSpeed, const uint8_t inputStringSize)
  : serialSpeed(aSerialSpeed),
    inputStringSizeMax(inputStringSize) {
  inputString.reserve(inputStringSize);
  //evManager.(this);
}

void evHandlerSerial::begin() {
  Serial.begin(serialSpeed);
}

byte evHandlerSerial::get() {
  if (stringComplete) {
    stringComplete = false;
    stringErase = true;  // la chaine sera effacee au prochain caractere recu
    evManager.StringPtr = &inputString;
    return (evManager.code = evInString);
  }
  if (Serial.available()) {
    inputChar = Serial.read();
    if (stringErase) {
      inputString = "";
      stringErase = false;
    }
    if (isPrintable(inputChar) && (inputString.length() <= inputStringSizeMax)) {
      inputString += inputChar;
    };
    if (inputChar == '\n' || inputChar == '\r') {
      stringComplete = (inputString.length() > 0);
    }
    evManager.charExt = inputChar;
    return (evManager.code = evInChar);
  }
  return (evNill);
}


/**********************************************************

   gestion d'un traceur de debugage touche 'T' pour visualiser la charge CPU

 ***********************************************************/


void evHandlerDebug::handle() {
  switch (evManager.code) {
    case ev1Hz:

      if (trackTime) {
        Serial.print(Digit2_str(hour()));
        Serial.print(':');
        Serial.print(Digit2_str(minute()));
        Serial.print(':');
        Serial.print(Digit2_str(second()));
        Serial.print(F(",CPU="));
        Serial.print(evManager._percentCPU);
        Serial.print('%');
        if (trackTime < 2) {

          Serial.print(F(",Loop="));
          Serial.print(evManager._loopParsec);
          Serial.print(F(",Nill="));
          Serial.print(evManager._evNillParsec);
          Serial.print(F(",Ram="));
          Serial.print(evManager.freeRam());
#ifdef ESP8266
          Serial.print(F(",Frag="));
          Serial.print(ESP.getHeapFragmentation());
          Serial.print(F("%,MaxMem="));
          Serial.print(ESP.getMaxFreeBlockSize());
#endif
        }
        if (ev100HzMissed + ev10HzMissed) {
          Serial.print(F(" Miss:"));
          Serial.print(ev100HzMissed);
          Serial.print('/');
          Serial.print(ev10HzMissed);
          ev100HzMissed = 0;
          ev10HzMissed = 0;
        }
        Serial.println();
      }

      break;

    case ev10Hz:

      ev10HzMissed += evManager.intExt - 1;
      if (trackTime > 1) {

        if (evManager.intExt > 1) {
          //        for (int N = 2; N<currentEvent.param; N++) Serial.print(' ');
          Serial.print('X');
          Serial.print(evManager.intExt - 1);
        } else {
          Serial.print('|');
        }
      }
      break;

    case ev100Hz:
      ev100HzMissed += evManager.intExt - 1;

      if (trackTime > 2) {

        if (evManager.intExt > 1) {
          //      for (int N = 3; N<currentEvent.param; N++) Serial.print(' ');
          Serial.print('x');
          Serial.print(evManager.intExt - 1);
        } else {
          Serial.print('_');
        }
      }
      break;
    case evInString:
      if (evManager.StringPtr->equals("T")) {
        if (++(trackTime) > 3) {

          trackTime = 0;
        }
        Serial.print("\nTrackTime=");
        Serial.println(trackTime);
      }

      break;
  }
};

#endif