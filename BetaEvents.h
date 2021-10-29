/*************************************************
     Header betaEvents.h   helper yo use a Event system with EventManager
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
    - V2.1 27/10/2020

 *************************************************/
#pragma once


#include "EventsManager.h"
// Events Manager build an instance called "Events" who care about events

//// definition des evBP0 et evLed0 si ceux si sont absent
//#ifndef evBP0
//#define evBP0 80
//#warning "evBP0 should be define by user > 100"
//#endif
//
//#ifndef evLed0
//#define evLed0 81
//#warning "evLed0 should be define by user > 100"
//#endif
//#endif
//
//
//
// definition GPIO sur D5 pour BP0 si celuici n'est pas defini
#ifndef BP0_PIN
#if  defined(__AVR__)
#define BP0_PIN 5
#elif defined(ESP8266) || defined(ESP32)
#define BP0_PIN D5 // D2
#endif
#endif

//definition GPIO sur LED_BUILTIN pour LED0 si il n'est pas defini par l'utilisateur
#ifndef LED0_PIN
#define LED0_PIN LED_BUILTIN
#endif

// reverted led on AVR UNO and NANO
#if  defined(__AVR__)
  const bool Led0Revert = true;
#else
  const bool Led0Revert = false;
#endif

// instance eventsManager
//EventManager Events;

// instance poussoir
evHandlerButton BP0(evBP0, BP0_PIN);

// instance LED


// led clignotante a 1Hz 
evHandlerLed    Led0(evLed0, LED0_PIN, Led0Revert , 1);

// instance Serial
evHandlerSerial Keyboard;

// instance debugger
evHandlerDebug  Debug;
