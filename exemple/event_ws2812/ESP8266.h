// definition de IO pour beta four

#define __BOARD__ESP8266__

#define LED_ON LOW


/*
//Pin out NODEMCU ESP8266
#define D0  16    //!LED_BUILTIN
#define D1  5     //       I2C_SCL
#define D2  4     //       I2C_SDA
#define D3  0     //!FLASH    BEEP_PIN
#define D4  2     //!LED2     PN532_POWER_PIN

#define D5  14    //!SPI_CLK    DOORLOCK_PIN Entrée porte verouillée
#define D6  12    //!SPI_MISO   GACHE_PIN
#define D7  13    //!SPI_MOSI   BP0_PIN
#define D8  15    //!BOOT_STS            
*/
#define BP0_PIN   D3                 //  High to Low = will wleep in, 5 minutes 
#define LED0_PIN  LED_BUILTIN   //   By default Led0 is on LED_BUILTIN you can change it 16 on ESP

//#define I2C_SDA  D2
//#define I2C_SCL  D1
//#define LED_ON  HIGH

#define WS2812_PIN D8  //Uniquement D8..D13



/*
#define GPIO_ENSET      0x60000310
#define GPIO_OUTSET     0x60000304
#define GPIO_OUTCLR     0x60000308
#define IOMUX_GPIO2     0x60000838

Setting the pin HIGH:

GPOS = (1 << PIN_OUT);
Setting the pin LOW:

GPOC = (1 << PIN_OUT);

extern void ICACHE_RAM_ATTR __digitalWrite(uint8_t pin, uint8_t val) {
  pwm_stop_pin(pin);
  if(pin < 16){
    if(val) GPOS = (1 << pin);
    else GPOC = (1 << pin);
  } else if(pin == 16){
    if(val) GP16O |= 1;
    else GP16O &= ~1;
  }
}

extern int ICACHE_RAM_ATTR __digitalRead(uint8_t pin) {
  pwm_stop_pin(pin);
  if(pin < 16){
    return GPIP(pin);
  } else if(pin == 16){
    return GP16I & 0x01;
  }
  return 0;
}
#define myPin 15
#define myPinBit (1<<15)

void setup() {
  pinMode(myPin,OUTPUT);
}

void loop() {
  WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + 4, myPinBit );
  delay(1000);                      
  WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + 8, myPinBit );
  delay(1000);                      
}






*/
