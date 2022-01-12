#ifndef E_PAPER_SPI_H_
#define E_PAPER_SPI_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#define EPAPER_HOST    HSPI_HOST

//#define OLD_VERSION
#ifdef  OLD_VERSION

#define PIN_NUM_MISO -1 // Do not use this pin
#define PIN_NUM_MOSI 2
#define PIN_NUM_CLK  15 //
#define PIN_NUM_CS   13
#define PIN_NUM_DC   12

#define PIN_NUM_BUSY 25
#define PIN_NUM_RESET 26

#define PIN_NUM_POWER   21//on//off

#else
#define PIN_NUM_MISO -1 // Do not use this pin
#define PIN_NUM_MOSI 32
#define PIN_NUM_CLK  33 //
#define PIN_NUM_CS   25
#define PIN_NUM_DC   26

#define PIN_NUM_BUSY 14
#define PIN_NUM_RESET 27

#define PIN_NUM_POWER   21//on//off
#endif


#define GPIO_DC_PIN_SEL     (1ULL<<PIN_NUM_DC) //| 1ULL<<PIN_NUM_RESET)  
#define GPIO_RESET_PIN_SEL  (1ULL<<PIN_NUM_RESET)
#define GPIO_POWER_PIN_SEL  (1ULL<<PIN_NUM_POWER) //| 1ULL<<PIN_NUM_RESET)  


#define GPIO_BUSY_PIN_SEL   (1ULL<<PIN_NUM_BUSY) 

void epaper_SendCmd(uint8_t command);
void epaper_SendData(const uint8_t data);
void epaper_pin_init(void);
void epaper_spi_init(void);
void epaper_spi_deinit(void);
void epaper_pin_deinit(void);
void epaper_poweron(void);
void epaper_poweroff(void);
void epaper_reset(void);
void epaper_power_sleep_hold_en(void);
void epaper_power_sleep_hold_dis(void);


#ifdef __cplusplus
}
#endif

#endif
