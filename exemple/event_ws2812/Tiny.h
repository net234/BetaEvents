#pragma once

#define __BOARD__NANO RF__
#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");

// ATMEL ATTINY45 / ARDUINO
//
//                  +-\/-+
//    Ain0 (D 5) PB5  1|    |8  Vcc
// RX Ain3 (D 3) PB3  2|    |7  PB2 (D 2)  Ain1  data W22812

// Tx Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1  LED_LIFE
//               GND  4|    |5  PB0 (D 0) pwm0  BP0
//                  +----+

//---------------------

// affectation des pattes pour bandeau
#define LED_LIFE  PB1
#define BP0     PB0
#define LED_ON  HIGH
#define PIN_WS2812 PB2
