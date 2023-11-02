#pragma once

#define __BOARD__NANO RF__
#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");

//   Nano RF
//#define TX1  31   //!TXD        17  //!RXLED
//#define RX0  30   //!RXD        30  //!TXLED
#define D2     2 //!HI2C_SDA   WS812
#define D3     3 //!HI2C_SCL
#define D4     4 // 
#define D5     5 //
#define D6     6 //
#define D7     7 //           BP0
#define D8     8 //            Bandeau de led (PIN_WS2812) 
#define D9     9 //PB1 !RF24_CSN
#define D10    10 //PB2 !RF24_CE
#define D11    11 //PB3 !SPI_MOSI
#define D12    12 //PB4 !SPI_MISO 
#define D13    13 //PB5 !SPI_CLK   !LED_BUITIN
//---------------------

// affectation des pattes pour bandeau
#define LED0_PIN  LED_BUILTIN
#define LED_ON  HIGH

#define BP0_PIN D7
#define PIN_WS2812 D8  //Uniquement D8..D13
//---------------------

//#define USE_RVBW   Si les led sont des WS2812W
