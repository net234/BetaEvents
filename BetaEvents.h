/*************************************************
     Header betaEvents.h   validation of lib betaEvents to deal nicely with events programing with Arduino
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


 *************************************************/

#include "EventsManager.h"

// definition des evBP0 et evLed0 si ceux si sont absent
#ifndef evBP0
#define evBP0 80
#pragma message "evBP0 should be define by user > 100"
#endif

#ifndef evLed0
#define evLed0 81  // should be define by user > 100
#endif


// definition GPIO sur D2 pour BP0 si celuici n'est pas defini
#ifndef pinBP0
#if  defined(__AVR__)
#define pinBP0 2  // D2
#elif defined(ESP8266) || defined(ESP32)
#define pinBP0 D2 // D2
#endif
#endif

//definition GPIO sur LED_BUILTIN pour LED0 si il n'est pas defini par l'utilisateur
#ifndef pinLed0
#define pinLed0 LED_BUILTIN
#endif

// instance eventsManager
EventManager MyEvents;

// instance poussoir
evHandlerButton MyBP0(evBP0, pinBP0);

// instance LED
evHandlerLed    MyLed0(evLed0, pinLed0);

// instance Serial
evHandlerSerial MyKeyboard;

// instance debugger
evHandlerDebug MyDebug;
