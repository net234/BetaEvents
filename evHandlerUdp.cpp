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

#include  "evHandlerUdp.h"

evHandlerUdp::evHandlerUdp(const uint8_t aEventCode, const uint16_t aPortNumber,  String& aNodename) :
  evCode(aEventCode),
  localPortNumber(aPortNumber),
  nodename(aNodename) {
  messageUDP.reserve(UDP_MAX_SIZE);
}

const  IPAddress broadcastIP(255, 255, 255, 255);

void evHandlerUdp::begin() {
  UDP.begin(localPortNumber);
}

void evHandlerUdp::handle() {
  if (evManager.code  == evCode) {

    // broadcst out = send unicast  inicastCnt  fois

    switch (evManager.ext) {
      case evxBcast: {
          if (unicastCnt == 0) return;
          // send udp after 200ms of silence
          if (millis() - lastUDP > 200) {
            unicast(broadcastIP);
            --unicastCnt;
          } // else D_println(unicastCnt);
          if (unicastCnt > 0) evManager.delayedPush(50, evCode, evxBcast);
        }
        break;
    }
    return;
  }

  // check for reception
  if (evManager.code != evNill) return;
  int packetSize = UDP.parsePacket();
  if (packetSize == 0) return;

  Serial.print("Received packet UDP");
//  Serial.printf("Received packet of size % d from % s: % d\n    (to % s: % d, free heap = % d B)\n",
//                packetSize,
//                UDP.remoteIP().toString().c_str(), UDP.remotePort(),
//                UDP.destinationIP().toString().c_str(), UDP.localPort(),
//                ESP.getFreeHeap());

  char udpPacketBuffer[UDP_MAX_SIZE + 1]; //buffer to hold incoming packet,
  int size = UDP.read(udpPacketBuffer, UDP_MAX_SIZE);

  // read the packet into packetBufffer
  if (packetSize > UDP_MAX_SIZE) {
    Serial.printf("UDP too big ");
    return;
  }

  //TODO: clean this   cleanup line feed
  if (size > 0 && udpPacketBuffer[size - 1] == '\n') size--;
  udpPacketBuffer[size] = 0;

  String aStr = udpPacketBuffer;
 
  lastUDP = millis();

  // filtrage des trame repetitive

   String bStr = grabFromStringUntil(aStr, '\t'); // EVENT xxxx
   rxHeader = grabFromStringUntil(bStr, ' '); // header
   rxNode = grabFromStringUntil(aStr, '\t'); // node

     // UdpId is a mix of remote IP and EVENT number
    IPAddress  aUdpId = UDP.remoteIP();
    aUdpId[0] = bStr.toInt();

    if (aUdpId == lastUdpId) {
      Serial.println(F("Doublon UDP"));
      return;
    }
    //Todo : filtrer les 5 dernier UdpID ?
    lastUdpId = aUdpId;


    bcast = ( UDP.destinationIP() == broadcastIP );
    rxJson = aStr;
//    D_print(rxHeader);
//    D_print(rxNode);
//    D_println(aStr);

    evManager.push(evCode,evxUdpRxMessage);

  //  // Broadcast
  //  if  ( MyUDP.destinationIP() == broadcastIP ) {
  //    // it is a reception broadcast
  //    String bStr = grabFromStringUntil(aStr, '\t');
  //    //aStr => 'cardreader  BetaPorte_2B  cardid  1626D989  user  Pierre H'
  //    String cStr = "";
  //    if ( bStr.equals(F("cardreader")) ) {
  //      messageUDP += "      ";
  //      cStr += grabFromStringUntil(aStr, '\t'); // event nodename
  //      bStr = grabFromStringUntil(aStr, '\t'); // 'cardid'
  //      bStr = grabFromStringUntil(aStr, '\t');  // cardid (value)
  //      bStr = grabFromStringUntil(aStr, '\t');  // 'user'
  //      cStr += " : ";
  //      cStr += aStr;
  //      D_println(cStr);
  //
  //      if (messageUDP.indexOf(cStr) < 0) {
  //        messageUDP += "    ";
  //        messageUDP += cStr;
  //      }
  //      Events.delayedPush(500, evNewStatus);
  //      Events.delayedPush(3 * 60 * 1000, evEraseUdp);
  //      return;
  //    }
  //  }


}


void evHandlerUdp::broadcast(const String & aJsonStr) {
  Serial.print("Send broadcast ");
  D_println(aJsonStr);
  messageUDP = aJsonStr;
  unicastCnt = 5;
  if (++numTrameUDP == 0) numTrameUDP++;
  Events.push(evCode, evxBcast);
}

void evHandlerUdp::unicast(const IPAddress aAddress) {
  Serial.println("Send unicast ");
  String message = F("EVENT ");
  message += numTrameUDP;
  message += '\t';
  message += nodename;
  message += '\t';
  message += messageUDP;
  message += '\n';
  if ( !UDP.beginPacket(broadcastIP, localPortNumber) ) return ;
  UDP.write(message.c_str(), message.length());
  UDP.endPacket();
}
/*******



  String message = F("event\tbetaBrite\talive");

  message += '\n';

  if ( !MyUDP.beginPacket(broadcastIP, localUdpPort) ) return false;
  MyUDP.write(message.c_str(), message.length());

  MyUDP.endPacket();

  delay(100);

  if ( !MyUDP.beginPacket(broadcastIP, localUdpPort) ) return false;
  MyUDP.write(message.c_str(), message.length());

  MyUDP.endPacket();

  delay(100);
  if ( !MyUDP.beginPacket(broadcastIP, localUdpPort) ) return false;
  MyUDP.write(message.c_str(), message.length());

  MyUDP.endPacket();

  Serial.print(message);
  return true;
  }



  handleUdpPacket();        // handle UDP connection other betaporte


*/













//
//// port d'ecoute UDP
//const unsigned int localUdpPort = 23423;      // local port to listen on
////Objet UDP pour la liaison avec la console
//WiFiUDP MyUDP;
