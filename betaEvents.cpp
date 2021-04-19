/*************************************************
     Sketch betaEvents.ino   validation of lib betaEvents to deal nicely with events programing with Arduino
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
    V1.0 (21/11/2020)
    - Full rebuild from PH_Event V1.3.1 (15/03/2020)
    V1.1 (30/11/2020)
    - Ajout du percentCPU pour une meilleur visualisation de l'usage CPU
    V1.2 02/01/2021
    - Ajout d'une globale EventManagerPtr pour l'acces par d'autre lib et respecter l'implantation C++
    - Amelioration du iddle mode pour l'ESP8266 (WiFi sleep mode)
    V1.3 13/01/2021
    - correction pour mieux gerer les pulses dans le cas 0 ou 100 percent
   V1.4   6/3/2021
    - Inclusion TimeLib.h
    - Gestion des event en liste chainée

 *************************************************/
#define BETAEVENTS_CCP

#include "betaEvents.h"
#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");


#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)  //LEONARDO
#define LED_PULSE_ON LOW
#else
#ifdef  __AVR__
#define LED_PULSE_ON HIGH

#else
// Pour ESP c'est l'inverse
#define LED_PULSE_ON LOW
#endif
#endif

#ifdef  __AVR__
#include <avr/sleep.h>
#endif

void  EventManager::begin() {

#ifdef  USE_SERIALEVENT
  inputString.reserve(_inputStringSizeMax);

#endif
  pinMode(_LEDPinNumber, OUTPUT);
  pushEvent(evLEDOn);  // debut du clignotement
#ifdef  __AVR__
  /*
    Atmega328 seul et en sleep mode:  // 22 mA
    SLEEP_MODE_IDLE:       15 mA      //15 mA
    SLEEP_MODE_ADC:         6,5 mA    //30 mA  no timer1
    SLEEP_MODE_PWR_SAVE:    1,62 mA   //22 mA  no timer1
    SLEEP_MODE_EXT_STANDBY: 1,62 mA   //22 mA  no timer1
    SLEEP_MODE_STANDBY :    0,84 mA   //21 mA  no timer1
    SLEEP_MODE_PWR_DOWN :   0,36 mA   //21 mA  no timer1
  */
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
#endif
}

void  EventManager::setLedOn(const bool status) {
  setMillisecLED(1000, status ? 100 : 0);
  digitalWrite(_LEDPinNumber, status ? LED_PULSE_ON : !LED_PULSE_ON );
}


void  EventManager::setMillisecLED(const uint16_t millisecondes, const uint8_t percent) {
  _LEDMillisecondes = max(millisecondes, (uint16_t)10);
  _LEDPercent = percent;
  removeDelayEvent(evLEDOff);
  pushEvent( (percent > 0) ? evLEDOn : evLEDOff );
}

void  EventManager::setFrequenceLED(const uint8_t frequence, const uint8_t percent) {
  setMillisecLED(1000U / frequence, percent);
}

#ifndef _Time_h
//#ifdef  __AVR__
byte  second()  {
  return ( EventManagerPtr->timestamp % 60);
}
byte  minute()  {
  return ( (EventManagerPtr->timestamp / 60) % 60);
}
byte  hour()  {
  return ( (EventManagerPtr->timestamp / 3600) % 24);
}
//#endif
#endif

static uint32_t milliSeconds = 0;
static uint16_t delta1Hz = 0;
static uint16_t delta10Hz = 0;
static uint16_t delta100Hz = 0;
//static uint16_t delta1000Hz = 0;

//int EventManager::syncroSeconde(const int millisec) {
//  int result =  millisec - delta1Hz;
//  if (result != 0) {
//    delta1Hz = millisec;
//    delta10Hz = millisec;
//    delta100Hz = millisec;
//  }
//  return result;
//}


byte EventManager::getEvent(const bool sleepOk ) {  //  sleep = true;
  bool eventWasNill = ( currentEvent.code == evNill);
  _loopCounter++;
  // cumul du temps passé
  uint32_t delta = millis() - milliSeconds;
  if (delta) {
    milliSeconds += delta;
    parseDelayList(&(this->eventMillisList), delta);
    delta100Hz += delta;
    delta10Hz += delta;
    delta1Hz += delta;
  }
  // recuperation des events passés
  if (nextEvent()) return (currentEvent.code);

  // si SleepOk et que l'evenement precedent etait un nillEvent on frezze le CPU
  if (sleepOk  && eventWasNill) {

#ifdef  __AVR__
    sleep_mode();
#else
    // pour l'ESP8266 pas sleep simple
    // !! TODO :  faire un meilleur sleep ESP32 & ESP8266
    //while (milliSeconds == millis()) yield();
    delay(1);  // to allow wifi sleep in modem mode
#endif
    delta = millis() - milliSeconds;
    if (delta) {
      _idleMillisec += delta;
      milliSeconds += delta;
      parseDelayList(&(this->eventMillisList), delta);
      delta100Hz += delta;
      delta10Hz += delta;
      delta1Hz += delta;
    }
    // recuperation des events passés
    if (nextEvent()) return (currentEvent.code);
  }
  _evNillCounter++;
  return (currentEvent.code = evNill);
}


///////////////////////////////////////////////////////////
// get next done event
byte EventManager::nextEvent() {

  // les ev100Hz ne sont pas tous restitués
  // il sont utilisé pour les DelayCentEvent

  if (delta100Hz >= 10)  {
    currentEvent.param = (delta100Hz / 10);  // nombre d'ev100Hz d'un coup
    delta100Hz -= (currentEvent.param) * 10;
    return (currentEvent.code = ev100Hz);
  }

  // les ev10Hz ne sont pas tous restitués
  if (delta10Hz >= 100)  {
    currentEvent.param = (delta10Hz / 100);  // nombre d'ev10Hz d'un coup
    delta10Hz -= (currentEvent.param) * 100;
    return (currentEvent.code = ev10Hz);
  }

  // par contre les ev1Hz sont tous restirués meme avec du retard
  if (delta1Hz >= 1000)  {
    //    __cnt1Hz--;
    delta1Hz -= 1000;
    return (currentEvent.code = ev1Hz);
  }


#ifdef  USE_SERIALEVENT
  if (_stringComplete)   {
    _stringComplete = false;
    _stringErase = true;      // la chaine ser  a effacee au prochain caractere recu
    return (currentEvent.code = evInString);
  }

  if (Serial.available())   {
    inChar = (char)Serial.read();
    return (currentEvent.code = evInChar);
  }
#endif


  // les evenements sans delay sont geré ici
  // les delais sont gere via ev100HZ
  if (eventList) {
    eventItem_t* itemPtr = eventList->nextItemPtr;
    currentEvent = *eventList;
    delete eventList;
    eventList = itemPtr;
    return (currentEvent.code);
  }

  return (currentEvent.code = evNill);
}


void  EventManager::parseDelayList(delayEventItem_t** ItemPtr, const uint16_t delay) {
  while (*ItemPtr) {
    if ((*ItemPtr)->delay > delay ) {
      (*ItemPtr)->delay -= delay;
      ItemPtr = &((*ItemPtr)->nextItemPtr);
    } else {
      //Serial.print("done waitingdelay : ");
      //D_println((*ItemPtr)->code);
      delayEventItem_t* aDelayItemPtr = *ItemPtr;
      pushEvent(*aDelayItemPtr);
      delete aDelayItemPtr;
      *ItemPtr = (*ItemPtr)->nextItemPtr;
    }
  }
}

void  EventManager::handleEvent() {
  switch (currentEvent.code)
  {
    // gestion des evenement avec delay au 100' de seconde
    // todo  gerer des event repetitifs

    case ev100Hz: {
        parseDelayList( &(this->eventCentsList), currentEvent.param);
      }

      break;

    case ev10Hz: {
        parseDelayList( &(this->eventTenthList), currentEvent.param);
      }

      break;




    case ev1Hz: {
#ifndef _Time_h
        timestamp++;
#endif

        _percentCPU = 100 - (100UL * _idleMillisec / 1000 );


        // TODO: add ev24H with TimeLib
#ifndef _Time_h
        if (timestamp % 86400L == 0) {  // 60 * 60 * 24
          pushEvent(ev24H);  // User may take care of days
        }
#endif
        //        Serial.print("iddle="); Serial.println(_idleMillisec);
        //        Serial.print("CPU% ="); Serial.println(_percentCPU);
        //        Serial.print("_evNillCounter="); Serial.println(_evNillCounter);
        //        Serial.print("_loopCounter="); Serial.println(_loopCounter);
        //        //Serial.print("elaps="); Serial.println(elaps);
        _idleMillisec = 0;
        _evNillParsec = _evNillCounter;
        _evNillCounter = 0;
        _loopParsec = _loopCounter;
        _loopCounter = 0;

      }
      break;
    case evLEDOff:
      digitalWrite(_LEDPinNumber, !LED_PULSE_ON);   // led off
      break;

    case evLEDOn:
      digitalWrite(_LEDPinNumber, _LEDPercent > 0 ? LED_PULSE_ON : !LED_PULSE_ON );
      if (_LEDPercent > 0 && _LEDPercent < 100) {
        pushDelayEvent(_LEDMillisecondes, evLEDOn);
        pushDelayEvent(_LEDMillisecondes * _LEDPercent / 100, evLEDOff);
      }
      break;

#ifdef  USE_SERIALEVENT
    case  evInChar:
      if (_stringErase) {
        inputString = "";
        _stringErase = false;
      }
      if (isPrintable(inChar) && (inputString.length() < _inputStringSizeMax)) {
        inputString += inChar;
      };
      if (inChar == '\n' || inChar == '\r') {
        _stringComplete = (inputString.length() > 0);
      }
      break;
#endif
  }
}



bool  EventManager::pushEvent(const stdEvent_t& aevent) {
  eventItem_t** itemPtr = &(this->eventList);
  while (*itemPtr) itemPtr = &((*itemPtr)->nextItemPtr);
  *itemPtr = new eventItem_t(aevent);
  return (true);
}

bool   EventManager::pushEvent(const uint8_t codeP, const int16_t paramP) {
  eventItem_t aEvent(codeP, paramP);
  return ( pushEvent(aEvent) );
}

void EventManager::addDelayEvent(delayEventItem_t** ItemPtr, delayEventItem_t* aItem) {
  while (*ItemPtr) ItemPtr = &((*ItemPtr)->nextItemPtr);
  *ItemPtr = aItem;
}

bool   EventManager::pushDelayEvent(const uint32_t delayMillisec, const uint8_t code, const int16_t param) {
  removeDelayEvent(code);
  if (delayMillisec == 0) {
    return ( pushEvent(code, param) );
  }
  if (delayMillisec < 2000) { // moins de 2 secondes
    addDelayEvent( &(this->eventMillisList), new delayEventItem_t(delayMillisec, code, param) );
    return (true);
  }
  if (delayMillisec < 60000) { // moins d'une minute
    addDelayEvent( &(this->eventCentsList), new delayEventItem_t(delayMillisec / 10, code, param) );
    return (true);
  }

  addDelayEvent( &(this->eventTenthList), new delayEventItem_t(delayMillisec / 100, code, param) );
  return (true);
}


bool   EventManager::removeDelayEventFromList(const byte codeevent, delayEventItem_t** nextItemPtr) {
  while (*nextItemPtr) {
    if ((*nextItemPtr)->code == codeevent) {
      delayEventItem_t* aevent = *nextItemPtr;
      *nextItemPtr = (*nextItemPtr)->nextItemPtr;
      delete aevent;
      return true;
    }
    nextItemPtr = &((*nextItemPtr)->nextItemPtr);
  }
  return (false);
}

bool   EventManager::removeDelayEvent(const byte codeevent) {
  return ( removeDelayEventFromList(codeevent, &(this->eventMillisList)) ||
           removeDelayEventFromList(codeevent, &(this->eventCentsList)) ||
           removeDelayEventFromList(codeevent, &(this->eventTenthList)) );
}

//====== Sram dispo =========
#ifndef __AVR__
int EventManager::freeRam () {
  return ESP.getFreeHeap();
}
#else
int EventManager::freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif

//================ trackEvent =====================


void EventTracker::handleEvent() {
  EventManager::handleEvent();
  switch (currentEvent.code) {
    case ev1Hz:

      if (_trackTime) {

        char aBuffer[60];

        snprintf(aBuffer, 60 , " %02d:%02d:%02d,CPU=%d%%,Loop=%lu,Nill=%lu,Ram=%u", hour(), minute(), second(), _percentCPU, _loopParsec, _evNillParsec, freeRam());


        Serial.print(aBuffer);

        if (_ev100HzMissed + _ev10HzMissed) {
          sprintf(aBuffer, " Miss:%d/%d", _ev100HzMissed, _ev10HzMissed);
          Serial.print(aBuffer);
          _ev100HzMissed = 0;
          _ev10HzMissed = 0;
        }
        Serial.println();
      }

      break;

    case ev10Hz:

      _ev10HzMissed += currentEvent.param - 1;
      if (_trackTime > 1 ) {

        if (currentEvent.param > 1) {
          //        for (int N = 2; N<currentEvent.param; N++) Serial.print(' ');
          Serial.print('X');
          Serial.print(currentEvent.param - 1);
        } else {
          Serial.print('|');
        }
      }
      break;

    case ev100Hz:
      _ev100HzMissed += currentEvent.param - 1;

      if (_trackTime > 2)
      {

        if (currentEvent.param > 1) {
          //      for (int N = 3; N<currentEvent.param; N++) Serial.print(' ');
          Serial.print('x');
          Serial.print(currentEvent.param - 1);
        } else {
          Serial.print('_');
        }
      }
      break;
    case evInString:
      if (inputString.equals("T")) {


        if ( ++_trackTime > 3 ) {

          _trackTime = 0;
        }
        Serial.print("\nTrackTime=");
        Serial.println(_trackTime);
      }

      break;
  }


};
