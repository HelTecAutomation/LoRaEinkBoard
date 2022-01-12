#ifndef McuSet_H
#define McuSet_H

#include <Arduino.h>
#include "SPI.h"
#include "timer.h"
#include "rtc-board.h"
#include "board-config.h"
#include "lorawan_spi.h"
//#include "debug.h"



class McuClass{
public:
  McuClass();
  void init(uint32_t * license);
  void setSPIFrequency(uint32_t frequency);
  void sleep(uint8_t classMode,uint8_t enable_wota ,uint8_t debugLevel);
  SPISettings _spiSettings;
private:

};
extern TimerEvent_t TxNextPacketTimer;

#ifdef __cplusplus
extern "C" uint8_t SpiInOut(Spi_t *obj, uint8_t outData );
extern "C" void lora_printf(const char *format, ...);
extern "C" uint64_t timercheck();
extern "C" uint64_t getID();
extern "C" void SX126xIoInit( void );
extern "C" void SX126xReset( void );
extern "C" void sx126xSleep( void );
#endif

extern McuClass Mcu;
#endif
