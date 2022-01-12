#include <ESP32_sx1262_LoRaWAN.h>
#include "Arduino.h"
#include "epaper_driver.h"
#include "para_modification/usart_command.h"



/*license for Heltec ESP32 LoRaWan, quary your ChipID relevant license: http://resource.heltec.cn/search */
uint32_t  license[4] = {0x00849BFF, 0xBA832207, 0x9E2964C3, 0x929ACB5D};

/* every WOTA_CAD_CYCLE_TIME the node start 2 symbols cad dectect
 * if no cad detected, into sleep mode.
 * if cad detected, inot Rx mode.
 * if no data received in WOTA_MAX_RX_TIME, stop rx mode and start cycle cad.
 */
#define WOTA_CAD_CYCLE_TIME 1000;
#define WOTA_MAX_RX_TIME 6000;
#define WOTA_FREQ 505300000
#define WOTA_DR   DR_5
/*
 * set LoraWan_RGB to Active,the RGB active in loraWan
 * RGB red means sending;
 * RGB purple means joined done;
 * RGB blue means RxWindow1;
 * RGB yellow means RxWindow2;
 * RGB green means received done;
 */
RTC_DATA_ATTR char qr_code_arr[200] = "{\"DeviceName\":\"ESP32V2\",\"ProductId\":\"WBGEBNMT8U\",\"Signature\":\"c32b793ab455469d9a7dbc54cc7b7ed7\"}";

/* OTAA para*/
RTC_DATA_ATTR uint8_t devEui[] = { 0x22, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x01 };
RTC_DATA_ATTR uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
RTC_DATA_ATTR uint8_t appKey[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x01 };

/* ABP para*/
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

/*LoraWan channelsmask, default channels 0-7*/ 
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = CLASS_A;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 300000;

/*OTAA or ABP*/
bool overTheAirActivation = true;

/*ADR enable*/
bool loraWanAdr = true;

/* set LORAWAN_Net_Reserve ON, the node could save the network info to flash, when node reset not need to join again */
bool keepNet = true;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 2;
/*!
* Number of trials to transmit the frame, if the LoRaMAC layer did not
* receive an acknowledgment. The MAC performs a datarate adaptation,
* according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
* to the following table:
*
* Transmission nb | Data Rate
* ----------------|-----------
* 1 (first)       | DR
* 2               | DR
* 3               | max(DR-1,0)
* 4               | max(DR-1,0)
* 5               | max(DR-2,0)
* 6               | max(DR-2,0)
* 7               | max(DR-3,0)
* 8               | max(DR-3,0)
*
* Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
* the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
uint8_t confirmedNbTrials = 4;

/* Prepares the payload of the frame */
uint8_t debugLevel = 0;
static void prepareTxFrame( uint8_t port )
{
  /*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commissioning.h".
  *appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE.
  *if enabled AT, don't modify LORAWAN_APP_DATA_MAX_SIZE, it may cause system hanging or failure.
  *if disabled AT, LORAWAN_APP_DATA_MAX_SIZE can be modified, the max value is reference to lorawan region and SF.
  *for example, if use REGION_CN470, 
  *the max value for different DR can be found in MaxPayloadOfDatarateCN470 refer to DataratesCN470 and BandwidthsCN470 in "RegionCN470.h".
  */
    appDataSize = 4;
    appData[0] = 0x00;
    appData[1] = 0x01;
    appData[2] = 0x02;
    appData[3] = 0x03;
}
extern "C" void SX126xWriteRegister( uint16_t address, uint8_t value );
extern "C" uint8_t SX126xReadRegister( uint16_t address );
epaper_class epaper;
//downlink data handle function example
void downLinkDataHandle(McpsIndication_t *mcpsIndication)
{
 	 uint8_t rec_array[256];
 	 Serial.printf("+REV DATA:RXSIZE %d,PORT %d\r\n",mcpsIndication->BufferSize,mcpsIndication->Port);
  Serial.print("+REV DATA:");
	for(uint8_t i=0;i<mcpsIndication->BufferSize;i++)
	{
		Serial.printf("%02X",mcpsIndication->Buffer[i]);
		rec_array[i] = mcpsIndication->Buffer[i];
	}
	Serial.print("\r\n");
	epaper.epaper_display_data(rec_array,mcpsIndication->BufferSize);
	
}

void setup() {
  Serial.begin(115200);
//  while (!Serial);+
  usart_rec_process();
  read_config(devEui,appKey,qr_code_arr);
  Mcu.init(license);
  epaper.epaper_driver_init();
  deviceState = DEVICE_STATE_INIT;
  wota_cycle_time = WOTA_CAD_CYCLE_TIME;
  wota_max_rxtime = WOTA_MAX_RX_TIME;
  wota_dr = WOTA_DR;
  wota_freq = WOTA_FREQ;
}

void loop()
{
  switch( deviceState )
  {
    case DEVICE_STATE_INIT:
    {

      LoRaWAN.init(loraWanClass,loraWanRegion);

      break;
    }
    case DEVICE_STATE_JOIN:
    {
      LoRaWAN.join();
      break;
    }
    case DEVICE_STATE_SEND:
    {
      prepareTxFrame( appPort );
      stopWotaCad();
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE:
    {
      // Schedule next packet transmission
   
      txDutyCycleTime = appTxDutyCycle + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    }
    case DEVICE_STATE_SLEEP:
    {
      wotaCadProcess();
      LoRaWAN.sleep(loraWanClass,debugLevel);
      break;
    }
    default:
    {
      deviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}