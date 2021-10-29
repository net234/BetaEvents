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
    V2.0  20/04/2021
    - Mise en liste chainée de modules 'events' 
      evHandlerSerial   Gestion des caracteres et des chaines provenant de Serial
      evHandlerLed      Gestion d'une led avec ou sans clignotement sur un GPIO (Multiple instance possible)
      evHandlerButton   Gestion d'un pousoir sur un GPIO (Multiple instance possible)
      evHandlerDebug    Affichage de l'occupation CPU, de la memoire libre et des evenements 100Hz 10Hz et 1Hz
    

 *************************************************/
#define BETAEVENTS_CCP

#include "betaEvents.h"
#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");


#ifdef  __AVR__
#include <avr/sleep.h>
#endif




void  EventManager::begin() {
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

  eventHandler_t** ItemPtr = &this->eventHandlerList;
  while (*ItemPtr) {
    if ( (*ItemPtr)->nextEvent() ) return (currentEvent.code);
    ItemPtr = &((*ItemPtr)->next);
  }

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
  // parse event list
  eventHandler_t** ItemPtr = &this->eventHandlerList;
  while (*ItemPtr) {
    (*ItemPtr)->handleEvent();
    ItemPtr = &((*ItemPtr)->next);
  }
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
//    case evLEDOff:
//      digitalWrite(_LEDPinNumber, !LED_PULSE_ON);   // led off
//      break;
//
//    case evLEDOn:
//      digitalWrite(_LEDPinNumber, _LEDPercent > 0 ? LED_PULSE_ON : !LED_PULSE_ON );
//      if (_LEDPercent > 0 && _LEDPercent < 100) {
//        pushDelayEvent(_LEDMillisecondes, evLEDOn);
//        pushDelayEvent(_LEDMillisecondes * _LEDPercent / 100, evLEDOff);
//      }
//      break;


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


void   EventManager::addEventHandler(eventHandler_t* aHandler) {
  eventHandler_t** ItemPtr = &this->eventHandlerList;
  while (*ItemPtr) ItemPtr = &((*ItemPtr)->next);
  *ItemPtr = aHandler;
}


void EventManager::addDelayEvent(delayEventItem_t** ItemPtr, delayEventItem_t* aItem) {
  while (*ItemPtr) ItemPtr = &((*ItemPtr)->nextItemPtr);
  *ItemPtr = aItem;
}

bool   EventManager::pushDelayEvent(const uint32_t delayMillisec, const uint8_t code, const int16_t param, const bool force) {
  if (!force) removeDelayEvent(code);
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
