/*************************************************
 *************************************************
    handler evUdp.h   validation of lib betaEvents to deal nicely with events programing with Arduino
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

    V1.0  07/09/2022
    - gestion de message UDP pour communiquer en node avec des events

     *************************************************/
#pragma once
#include <Arduino.h>
#include  "EventsManager.h"
#include <WiFiUdp.h>

typedef enum   {
  // evenement recu
  //evxUdpRBCast,           // broadcast recu
  //evxUdpRUCast,            // Unicast recu
  // evenements interne de gestion
  evxBcast,                // send broadcast
  //evxNoPending,            // clear pending UDP
} tUdpEventExt;

const int UDP_MAX_SIZE = 200;  // we handle short messages


class evHandlerUdp : public eventHandler_t {
  public:
    evHandlerUdp(const uint8_t aEventCode,const uint16_t aPortNumber, String& aNodename);
    virtual void begin()  override;
    virtual void handle()  override;
    void broadcast(const String& aJsonStr);
    void unicast(const IPAddress aIPAddress);
  private:
    uint8_t evCode;
    uint16_t localPortNumber;
    WiFiUDP UDP;
    String & nodename;  // pointeur sur l'identifiant de trma nodename
    String messageUDP;  // message UDP en cours d'emission
    //bool  pendingUDP = false;   // udp less than 500ms
    time_t  lastUDP;
    uint8_t numTrameUDP = 0; // numeroteur de trame UDP
    uint8_t unicastCnt;      // compteur d'unicast a l'emission
};
