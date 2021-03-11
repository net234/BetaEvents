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
    - ajout setLedOn(true/false)
    V1.3.1 23/01/2021
	- correction setLedOn pour un resultat immediat
 Le croquis utilise 9476 octets (30%) de l'espace de stockage de programmes. Le maximum est de 30720 octets.
Les variables globales utilisent 493 octets (24%) de mémoire dynamique, ce qui laisse 1555 octets pour les variables locales. Le maximum est de 2048 octets.

    V1.4   6/3/2021
Le croquis utilise 9632 octets (31%) de l'espace de stockage de programmes. Le maximum est de 30720 octets.
Les variables globales utilisent 484 octets (23%) de mémoire dynamique, ce qui laisse 1564 octets pour les variables locales. Le maximum est de 2048 octets.
Le croquis utilise 9582 octets (31%) de l'espace de stockage de programmes. Le maximum est de 30720 octets.
Les variables globales utilisent 381 octets (18%) de mémoire dynamique, ce qui laisse 1667 octets pour les variables locales. Le maximum est de 2048 octets.
Le croquis utilise 9434 octets (30%) de l'espace de stockage de programmes. Le maximum est de 30720 octets.
Les variables globales utilisent 367 octets (17%) de mémoire dynamique, ce qui laisse 1681 octets pour les variables locales. Le maximum est de 2048 octets.

Le croquis utilise 9544 octets (31%) de l'espace de stockage de programmes. Le maximum est de 30720 octets.
Les variables globales utilisent 365 octets (17%) de mémoire dynamique, ce qui laisse 1683 octets pour les variables locales. Le maximum est de 2048 octets.

Le croquis utilise 9804 octets (31%) de l'espace de stockage de programmes. Le maximum est de 30720 octets.
Les variables globales utilisent 365 octets (17%) de mémoire dynamique, ce qui laisse 1683 octets pour les variables locales.

Le croquis utilise 9982 octets (32%) de l'espace de stockage de programmes. Le maximum est de 30720 octets.
Les variables globales utilisent 367 octets (17%) de mémoire dynamique, ce qui laisse 1681 octets pour les variables locales. Le maximum est de 2048 octets.


    Inclusion TimeLib.h
    Gestion des event en liste chainée

 *************************************************/

#pragma once
#include "Arduino.h"

// betaEvent handle a minimal time system to get for seconds() minutes() or hours()
//#include <TimeLib.h>          // uncomment this if you prefer to use arduino TimeLib.h  (it will use little more ram and flash)

#define   USE_SERIALEVENT       // comment this if you need standard Serial.read

#ifndef  LED_BUILTIN
#if defined(ESP32)
#define LED_BUILTIN 33   //ESP32-cam
#elif
#define LED_BUILTIN 13
#endif
#endif

class EventManager;
#ifdef BETAEVENTS_CCP
EventManager* EventManagerPtr; // allow other lib to access the specific instance of the user Sketch
#else
extern EventManager* EventManagerPtr;
#endif

enum tEventCode {
  evNill = 0,      // No event  about 1 every milisecond but do not use them for delay Use pushDelayEvent(delay,event)
  ev1000Hz,         // tick 1000HZ    non cumulative (see betaEvent.h)
  ev100Hz,         // tick 100HZ    non cumulative (see betaEvent.h)
  ev10Hz,          // tick 10HZ     non cumulative (see betaEvent.h)
  ev1Hz,           // un tick 1HZ   cumulative (see betaEvent.h)
  ev24H,           // 24H when timestamp pass over 24H
  evLEDOn,         // 
  evLEDOff,
  evInChar,
  evInString,
evWEB = 20,
evUser = 100,
};


// Base structure for event
struct stdEvent_t  {
//  stdEvent_t* clone() const { Serial.print("S"); return new stdEvent_t(*this); }
  //stdEvent_t() : code(evNill) , param(0) {}
  stdEvent_t(const uint8_t code= evNill,const int16_t param=0) : code(code), param(param) {}
  stdEvent_t(const stdEvent_t& stdevent) : code(stdevent.code), param(stdevent.param) {} 
  uint8_t code;       // code of the event
  int16_t param;      // parameter for the event
};

struct eventItem_t : stdEvent_t {
  eventItem_t(const uint8_t code= evNill,const int16_t param=0) : stdEvent_t(code,param),nextItemPtr(nullptr) {}
  eventItem_t(const stdEvent_t& stdEvent) : stdEvent_t(stdEvent),nextItemPtr(nullptr) {}
//  eventItem_t(const uint8_t codeP,const int16_t paramP) : stdEvent_t(codeP,paramP),nextItemPtr(nullptr) {}
  eventItem_t* nextItemPtr;
};



struct delayEventItem_t : stdEvent_t {
//  delayedEvent_t* clone() const { Serial.print("D"); return new delayedEvent_t(*this);  }
//  delayEventItem_t() : stdEvent_t(),nextItemPtr(nullptr) {}
  delayEventItem_t(const uint32_t delay, const uint8_t code,const int16_t param = 0) : stdEvent_t(code,param),delay(delay),nextItemPtr(nullptr) {}
  delayEventItem_t(const delayEventItem_t& stdEvent) : stdEvent_t(stdEvent) , delay(delay), nextItemPtr(nullptr) {}
  delayEventItem_t*  nextItemPtr;
  int32_t delay;         // delay in millisecondes;
};

class EventManager
{
  public:

    EventManager(const byte ledpinnumber = LED_BUILTIN, const byte inputStringSizeMax = 30) {  // constructeur
      if (EventManagerPtr != NULL) {
        Serial.print(F("Error: Only one instance for EventManager (BetaEvents)"));
        while (true) delay(100);
      }
      EventManagerPtr = this;
      currentEvent.code = evNill;
//      _waitingEventIndex = 0;

      _LEDPinNumber = ledpinnumber;
      _LEDMillisecondes = 1000;
      _LEDPercent = 10;


#ifdef USE_SERIALEVENT
      _inputStringSizeMax = inputStringSizeMax;
#endif
    }
    void   begin();
    byte   getEvent(const bool sleep = true);
    void   handleEvent();
    bool   removeDelayEvent(const byte codeevent);
    bool   pushEvent(const stdEvent_t& eventPtr);
    bool   pushEvent(const uint8_t code, const int16_t param = 0);
    bool   pushDelayEvent(const uint32_t delayMillisec, const byte code, const int param = 0);
//    bool   pushDelayEvent(const uint32_t delayMillisec, stdEvent_t &eventPtr );
    void   setLedOn(const bool status = true);
    void   setFrequenceLED(const uint8_t frequence, const uint8_t percent = 10); // frequence de la led
    void   setMillisecLED(const uint16_t millisecondes, const uint8_t percent = 10); // frequence de la led
    //    int    syncroSeconde(const int millisec = 0);
#ifndef _Time_h
    friend byte   second() ;
    friend byte   minute() ;
    friend byte   hour()   ;
#endif
    stdEvent_t currentEvent;

#ifdef  USE_SERIALEVENT
    char  inChar = '\0';
    String inputString = "";
#endif
    int freeRam();
#ifndef _Time_h
    uint32_t   timestamp = 0;   //timestamp en seconde  (more than 100 years)
#endif
  private:
   byte   nextEvent();
  protected:

    unsigned long      _loopCounter = 0;
    unsigned long      _evNillCounter = 0;
    unsigned long      _loopParsec = 0;
    unsigned long      _evNillParsec = 0;
    byte      _LEDPinNumber;         // Pin de la led de vie
    byte      _LEDPercent;           // durée de l'allumage en %
    uint16_t _LEDMillisecondes;  // durée du cycle de clignotement en Millisecondes (max 64 secondes)
    uint16_t _idleMillisec = 0;  // CPU millisecondes en pause
    byte       _percentCPU = 0;
    // liste des evenements en attente
//    byte       _waitingEventIndex = 0;
//    stdEvent  _waitingEvent[MAX_WAITING_EVENT];
    eventItem_t* eventList = nullptr;
    // liste des evenements sous delay en attente
//    byte       _waitingDelayEventIndex = 0;
//    delayedEvent_t _waitingDelayEvent[MAX_WAITING_DELAYEVENT];
    delayEventItem_t* delayEventList = nullptr;

#ifdef  USE_SERIALEVENT
    byte _inputStringSizeMax = 1;
    bool _stringComplete = false;
    bool _stringErase = false;
#endif

};


class EventTracker : public EventManager
{
  public:
    EventTracker(const byte aPinNumber = LED_BUILTIN, const byte inputStringSizeMax = 30) : EventManager{aPinNumber, inputStringSizeMax}  {};
    void handleEvent();
  protected:
    byte _trackTime = 0;
    int _ev100HzMissed = 0;
    int _ev10HzMissed = 0;
};
