/*************************************************
    EventsManager.h   validation of lib betaEvents to deal nicely with events programing with Arduino
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
    - ajout setLedOn(true/false)
    V1.3.1 23/01/2021
  	- correction setLedOn pour un resultat immediat
    V1.4   6/3/2021
    - Inclusion TimeLib.h
    - Gestion des event en liste chainée
     V2.0  20/04/2021
    - Mise en liste chainée de modules 'events'
      evHandlerSerial   Gestion des caracteres et des chaines provenant de Serial
      evHandlerLed      Gestion d'une led avec ou sans clignotement sur un GPIO (Multiple instance possible)
      evHandlerButton   Gestion d'un pousoir sur un GPIO (Multiple instance possible)
      evHandlerDebug    Affichage de l'occupation CPU, de la memoire libre et des evenements 100Hz 10Hz et 1Hz

     V2.1  05/06/2021
       split EventManger and BetaEvents

 *************************************************/
#pragma message "compile EventManager.h"
#pragma once
#include <Arduino.h>


// betaEvent handle a minimal time system to get for seconds() minutes() or hours()
//#ifndef  __AVR__
//#include <TimeLib.h>          // uncomment this if you prefer to use arduino TimeLib.h  (it will use little more ram and flash)
//#endif


class EventManager;
#ifdef BETAEVENTS_CCP
EventManager* EventManagerPtr; // allow other lib to access the specific instance of the user Sketch

#else
extern EventManager* EventManagerPtr;
#ifndef _Time_h
extern byte   second() ;
extern byte   minute() ;
extern byte   hour()   ;
#endif

#endif

enum tEventCode {
  evNill = 0,      // No event  about 1 every milisecond but do not use them for delay Use pushDelayEvent(delay,event)
  ev100Hz,         // tick 100HZ    non cumulative (see betaEvent.h)
  ev10Hz,          // tick 10HZ     non cumulative (see betaEvent.h)
  ev1Hz,           // un tick 1HZ   cumulative (see betaEvent.h)
  ev24H,           // 24H when timestamp pass over 24H
  evInChar,
  evInString,
  evPB0,
  evLED0
  //  evWEB = 20,
  //  evUser = 100,
};


// Base structure for event
struct stdEvent_t  {
  //  stdEvent_t(const uint8_t code = evNill, const int8_t ext = 0) : code(code), ext(ext) {}
  //  stdEvent_t(const uint8_t code = evNill) : code(code), aInt(0) {};
  stdEvent_t(const uint8_t code = evNill, const int aInt = 0) : code(code), aInt(aInt) {};
  //  stdEvent_t(const uint8_t code = evNill, const uint8_t ext ) : code(code), ext(ext) {};
  //  stdEvent_t(const uint8_t code = evNill, const char aChar) : code(code), aChar(aChar) {};

  stdEvent_t(const stdEvent_t& stdevent) : code(stdevent.code), ext(stdevent.ext) {}
  union   {
    uint8_t ext;        // extCode of the event
    char    aChar;
    int     aInt;
    String* aStringPtr;
  };
  uint8_t code;       // code of the event
};

// Base structure for an EventItem in an EventList
struct eventItem_t : public stdEvent_t {
  eventItem_t(const uint8_t code = evNill, const uint8_t ext = 0) : stdEvent_t(code, ext), nextItemPtr(nullptr) {}
  eventItem_t(const stdEvent_t& stdEvent) : stdEvent_t(stdEvent), nextItemPtr(nullptr) {}
  //  eventItem_t(const uint8_t codeP,const int16_t paramP) : stdEvent_t(codeP,paramP),nextItemPtr(nullptr) {}
  eventItem_t* nextItemPtr;
};



struct delayEventItem_t : public stdEvent_t {
  uint16_t delay;         // delay millis cents or thenth;
  delayEventItem_t(const uint32_t delay, const uint8_t code, const int8_t ext = 0) : stdEvent_t(code, ext), delay(delay), nextItemPtr(nullptr) {}
  delayEventItem_t(const delayEventItem_t& stdEvent) : stdEvent_t(stdEvent) , delay(delay), nextItemPtr(nullptr) {}
  delayEventItem_t*  nextItemPtr;

};

// base pour un eventHandler (gestionaire avec un handleEvent);
class eventHandler_t
{
  public:
    eventHandler_t *next;  // handle suivant
    eventHandler_t();
    virtual void handleEvent()  {};
    virtual byte getEvent()   {
      return evNill;
    };
};

#include "evHandlers.h"


class EventManager
{
  public:

    EventManager() {  // constructeur
#ifdef  EventManagerInstance        
#error "EventManager already intancied"
#else
#define EventManagerInstance
#endif
//      if (EventManagerPtr != NULL) {
 //       Serial.print(F("Error: Only one instance for EventManager (BetaEvents)"));
 //       while (true) delay(100);
 //      }
      EventManagerPtr = this;
      currentEvent.code = evNill;
    }
    void   begin();
    byte   getEvent(const bool sleep = true);
    void   handleEvent();

    bool   removeDelayEvent(const byte codeevent);
    bool   pushEvent(const stdEvent_t& eventPtr);
    bool   pushEvent(const uint8_t code, const int16_t param = 0);
    bool   pushDelayEvent(const uint32_t delayMillisec, const uint8_t code, const int16_t param = 0, const bool force = false);
    //    int    syncroSeconde(const int millisec = 0);
#ifndef _Time_h
    //#ifdef  __AVR__
    friend byte   second() ;
    friend byte   minute() ;
    friend byte   hour()   ;
    //#endif
#endif
    stdEvent_t currentEvent;

    int freeRam();
#ifndef _Time_h
    uint32_t   timestamp = 0;   //timestamp en seconde  (more than 100 years)
#endif

    void   addHandleEvent(eventHandler_t* eventHandlerPtr);
    void   addGetEvent(eventHandler_t* eventHandlerPtr);
  private:
    byte   nextEvent();  // Recherche du prochain event disponible
    void   parseDelayList(delayEventItem_t** ItemPtr, const uint16_t delay);
    void   addDelayEvent(delayEventItem_t** ItemPtr, delayEventItem_t* aItem);
    bool   removeDelayEventFromList(const byte codeevent, delayEventItem_t** nextItemPtr);

  public:
    unsigned long      _loopCounter = 0;
    unsigned long      _loopParsec = 0;
    unsigned long      _evNillParsec = 0;
    byte       _percentCPU = 0;
  private:
    unsigned long      _evNillCounter = 0;
    uint16_t           _idleMillisec = 0;  // CPU millisecondes en pause
    eventItem_t* eventList = nullptr;
    delayEventItem_t* eventMillisList = nullptr;
    delayEventItem_t* eventCentsList = nullptr;
    delayEventItem_t* eventTenthList = nullptr;
    eventHandler_t*   handleEventList = nullptr;
    eventHandler_t*   getEventList = nullptr;
};
