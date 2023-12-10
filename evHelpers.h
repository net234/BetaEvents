//Helpers
// generic helpers out of evManager library
#pragma once
#include <arduino.h>

#ifndef time_t
#if  defined(__AVR__)

#define time_t unsigned long
#endif
#endif


#ifdef DEBUG_ON 

// D_println(variable); permet d'afficher le nom de variable suivit de sa valeur
#define T_println(x) Serial.println(F(#x));
#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");
#define TD_println(x,y) Serial.print(F(#x " => '")); Serial.print(y); Serial.println("'");

//idem sans retour chariot
#define D_print(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.print("', ");
#define TD_print(x,y) Serial.print(F(#x " => '")); Serial.print(y); Serial.print("', ");

//idem mais en HEXA
#define X_println(x) Serial.print(F(#x " => '0x")); Serial.print(x,HEX); Serial.println("'");


#else

#define T_println(...)  ;
#define D_print(...)    while(0) {  };
#define D_println(...)  while(0) {  };
#define TD_print(...)   ;
#define TD_println(...)  while(0) {  };
#define X_println(...)    while(0) {  };

#endif

#define T1_println(x) Serial.println(F(#x));
#define D1_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");
#define D1_print(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.print("', ");
#define TD1_println(x,y) Serial.print(F(#x " => '")); Serial.print(y); Serial.println("'");
#define TD1_print(x,y) Serial.print(F(#x " => '")); Serial.print(y); Serial.print("', ");
#define X1_println(x) Serial.print(F(#x " => '0x")); Serial.print(x,HEX); Serial.println("'");


String niceDisplayDelay(time_t aDelay);
String Digit2_str(const uint16_t value);
//void   helperReset();
//int    helperFreeRam();

// extrait un element terminé par aKey de la chaine aString
// si aKey est absent toute la chaine est extaite
String grabFromStringUntil(String & aString, const char aKey);
String grabFromStringUntil(String & aString, const String aKey);
