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

 *************************************************/
#define BETAEVENTS_CCP

#include "betaEvents.h"
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
  setMillisecLED(1000,status ? 100 : 0);
  digitalWrite(_LEDPinNumber, status ? LED_PULSE_ON : !LED_PULSE_ON );
}


void  EventManager::setMillisecLED(const uint16_t millisecondes, const uint8_t percent) {
  _LEDMillisecondes = max(millisecondes, (uint16_t)10);
  _LEDPercent = percent;
  removeDelayEvent(evLEDOff);
pushEvent( (percent > 0) ? evLEDOn : evLEDOff );
}

void  EventManager::setFrequenceLED(const uint8_t frequence, const uint8_t percent) {
  setMillisecLED(1000U/frequence,percent);
}

#ifndef _Time_h
byte  second()  {
  return ( EventManagerPtr->timestamp % 60);
}
byte  minute()  {
  return ( (EventManagerPtr->timestamp / 60) % 60);
}
byte  hour()  {
  return ( (EventManagerPtr->timestamp / 3600) % 24);
}
#endif

static unsigned long milliSeconds = 0;
static unsigned int delta1Hz = 0;
static unsigned int delta10Hz = 0;
static unsigned int delta100Hz = 0;

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

  unsigned long delta = millis() - milliSeconds;

  if (delta) {
    milliSeconds += delta;
    delta1Hz += delta;
    delta10Hz += delta;
    delta100Hz += delta;

    // les ev100Hz ne sont pas tous restitués
    // il sont utilisé pour les DelayedEvent
    if (delta100Hz >= 10)
    {
      currentEvent.param = (delta100Hz / 10);  // nombre d'ev100Hz d'un coup
      delta100Hz -= (currentEvent.param) * 10;
      return (currentEvent.code = ev100Hz);
    }


    // les ev10Hz ne sont pas tous restitués
    if (delta10Hz >= 100)
    {
      currentEvent.param = (delta10Hz / 100);  // nombre d'ev10Hz d'un coup
      delta10Hz -= (currentEvent.param) * 100;
      return (currentEvent.code = ev10Hz);
    }
    // par contre les ev1Hz sont tous restirués meme avec du retard
    if (delta1Hz >= 1000)
    {
      //    __cnt1Hz--;
      delta1Hz -= 1000;
      return (currentEvent.code = ev1Hz);
    }
  }

#ifdef  USE_SERIALEVENT
  if (_stringComplete)
  {
    _stringComplete = false;
    _stringErase = true;      // la chaine ser  a effacee au prochain caractere recu
    return (currentEvent.code = evInString);
  }

  if (Serial.available())
  {
    inChar = (char)Serial.read();
    return (currentEvent.code = evInChar);
  }
#endif


  // les evenements sans delay sont geré ici
  // les delais sont gere via ev100HZ
  if (_waitingEventIndex != 0) {
    currentEvent = _waitingEvent[0];

    for (byte N = 0; N < _waitingEventIndex; N++) {
      _waitingEvent[N] = _waitingEvent[N + 1];
    }
    _waitingEvent[--_waitingEventIndex].code = evNill;
    return (currentEvent.code);
  }


  // si SleepOk et que l'evenement precedent etait un nillEvent on frezze le CPU
  if (millis() == milliSeconds && sleepOk  && eventWasNill) {

#ifdef  __AVR__
    sleep_mode();
#else
    // pour l'ESP8266 pas sleep simple
    // !! TODO :  faire un meilleur sleep ESP32 & ESP8266
    //while (milliSeconds == millis()) yield();
    delay(1);  // to allow wifi sleep in modem mode

#endif
    _idleMillisec += ( millis() - milliSeconds);
  }

  _evNillCounter++;

  return (currentEvent.code = evNill);
}






void  EventManager::handleEvent() {
  switch (currentEvent.code)
  {


    // gestion des evenement avec delay
    // todo  gerer des event repetitifs
    case ev100Hz: {
        //      Serial.print("waitingdelay : ");
        //          Serial.println(_waitingDelayEventIndex);
        // on scan les _waitintDelayEvent pour les passer en _waitintEvent
        byte N = 0;
        while (N < _waitingDelayEventIndex) {
          //        Serial.print("delay : ");
          //        Serial.println(_waitingDelayEvent[N].delay);
          if (_waitingDelayEvent[N].delay > currentEvent.param) {
            _waitingDelayEvent[N].delay -= currentEvent.param;
            N++;
          } else {
            //                      Serial.print("Exec delay Event ");
            //                     Serial.println(_waitingDelayEvent[N].codeEvent);
            pushEvent(&_waitingDelayEvent[N]);
            removeDelayEvent(_waitingDelayEvent[N].code);
          }
        }
      }

      break;



    case evLEDOff:
      digitalWrite(_LEDPinNumber, !LED_PULSE_ON);   // led off
      break;

    case evLEDOn:
      digitalWrite(_LEDPinNumber, LED_PULSE_ON);digitalWrite(_LEDPinNumber, LED_PULSE_ON);   // led on
      if (_LEDPercent > 0 && _LEDPercent < 100) pushDelayEvent(_LEDMillisecondes, evLEDOn);
      if (_LEDPercent < 100) pushDelayEvent(_LEDMillisecondes * _LEDPercent / 100, evLEDOff);
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

bool   EventManager::removeDelayEvent(const byte codeevent) {
  byte N = 0;
  while (N < _waitingDelayEventIndex) {
    if (_waitingDelayEvent[N].code == codeevent ) {
      //     Serial.print("Remove Delay Event ");
      //     Serial.println(codeevent);

      for (byte N2 = N; N2 < _waitingDelayEventIndex; N2++) {
        _waitingDelayEvent[N2] = _waitingDelayEvent[N2 + 1];
      }
      _waitingDelayEvent[--_waitingDelayEventIndex].code = evNill;
      _waitingDelayEvent[_waitingDelayEventIndex].param = 0;
    } else {
      N++;
    }
  }
}


bool  EventManager::pushEvent(stdEvent* aevent) {
  if (_waitingEventIndex >= MAX_WAITING_EVENT) {
    return (false);
  }
  _waitingEvent[_waitingEventIndex++] = *aevent;
  return (true);

}

bool   EventManager::pushEvent(const byte code, const int param) {
  stdEvent aEvent;
  aEvent.code = code;
  aEvent.param = param;
  return ( pushEvent(&aEvent) );
}


bool   EventManager::pushDelayEvent(const uint32_t delayMillisec, const byte code, const int param) {
  delayedEvent aEvent;
  aEvent.code = code;
  aEvent.param = param;

  removeDelayEvent(code);

  if (delayMillisec == 0) {
    return ( pushEvent(&aEvent) );
  }



  if (_waitingDelayEventIndex >= MAX_WAITING_DELAYEVENT) {
    return (false);
  }

  aEvent.delay = delayMillisec / 10;
  if (aEvent.delay == 0 )  aEvent.delay = 1;


  _waitingDelayEvent[_waitingDelayEventIndex++] = aEvent;
  //  Serial.print("Pushdelay ");
  //  Serial.print(aEvent.codeEvent);
  //  Serial.print(" dans ");
  //  Serial.print(aEvent.delay);
  //  Serial.println(" msec.");

  return (true);
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
