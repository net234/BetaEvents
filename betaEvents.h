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



 *************************************************/

#pragma once
#include "Arduino.h"

#define   USE_SERIALEVENT       // remove this if you need standard Serial.read 

#define   MAX_WAITING_EVENT       20    // size of event buffer

#define   MAX_WAITING_DELAYEVENT  10   // size of delayed event buffer


#ifndef  LED_BUILTIN
#if defined(ESP32)
# define LED_BUILTIN 33   //ESP32-cam
#elif
# define LED_BUILTIN 13
#endif
#endif


enum tEventCode {
  evNill = 0,      // No event
  ev100Hz,         // tick 100HZ    non cumulative (see betaEvent.h)
  ev10Hz,          // tick 10HZ     non cumulative (see betaEvent.h)
  ev1Hz,           // un tick 1HZ   cumulative (see betaEvent.h)
  ev24H,           // 24H when timestamp pass over 24H
  evLEDOn,         // Kife
  evLEDOff,
  evInChar,
  evInString,
};


// 2 byte structure for event
struct stdEvent  {
  uint8_t code = evNill;       // code of the event
  int16_t param = 0;           // parameter for the event
};

struct delayedEvent : stdEvent {
  int32_t delay;         // delay in millisecondes;
};

class EventManager
{
  public:

    EventManager(const byte ledpinnumber = LED_BUILTIN, const byte inputStringSizeMax = 30) {  // constructeur
      currentEvent.code = evNill;
      _waitingEventIndex = 0;

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
    bool   pushEvent(const byte code, const int param = 0);
    bool   pushEvent(stdEvent* aevent);
    bool   pushDelayEvent(const uint32_t delayMillisec, const byte code, const int param = 0);
    bool   pushDelayEvent(const uint32_t delayMillisec, stdEvent* aevent );
    void   setFreqenceLED(const byte frequence, const byte percent = 10); // frequence de la led
    void   setMillisecLED(const int millisecondes, const byte percent = 10); // frequence de la led
    //    int    syncroSeconde(const int millisec = 0);
    byte   second() const;
    byte   minute() const;
    byte   hour()   const;
    stdEvent currentEvent;

#ifdef  USE_SERIALEVENT
    char  inChar = '\0';
    String inputString = "";
#endif
    int freeRam();
    unsigned long   timestamp = 0;   //timestamp en seconde  (around 49 jours)

  protected:

    unsigned long      _loopCounter = 0;
    unsigned long      _evNillCounter = 0;
    unsigned long      _loopParsec = 0;
    unsigned long      _evNillParsec = 0;
    byte      _LEDPinNumber;         // Pin de la led de vie
    byte      _LEDPercent;           // durée de l'allumage en %
    unsigned int _LEDMillisecondes;  // durée du cycle de clignotement en Millisecondes (max 64 secondes)
    unsigned int _idleMillisec = 0;  // CPU millisecondes en pause
    byte       _percentCPU = 0;
    // liste des evenements en attente
    byte       _waitingEventIndex = 0;
    stdEvent  _waitingEvent[MAX_WAITING_EVENT];
    
    // liste des evenements sous delay en attente
    byte       _waitingDelayEventIndex = 0;
    delayedEvent _waitingDelayEvent[MAX_WAITING_DELAYEVENT];
    

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
