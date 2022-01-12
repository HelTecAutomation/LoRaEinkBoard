/*
 * HelTec Automation(TM) LoRaWAN 1.0.2 OTAA example use OTAA, CLASS A
 *
 * Function summary:
 *
 * - use internal RTC(150KHz);
 *
 * - Include stop mode and deep sleep mode;
 *
 * - 15S data send cycle;
 *
 * - Informations output via serial(115200);
 *
 * - Only ESP32 + LoRa series boards can use this library, need a license
 *   to make the code run(check you license here: http://www.heltec.cn/search/);
 *
 * You can change some definition in "Commissioning.h" and "LoRaMac-definitions.h"
 *
 * HelTec AutoMation, Chengdu, China.
 * 成都惠利特自动化科技有限公司
 * https://heltec.org
 * support@heltec.cn
 *
 *this project also release in GitHub:
 *https://github.com/HelTecAutomation/ESP32_LoRaWAN
*/

#include <ESP32_sx1262_LoRaWAN.h>
#include "Arduino.h"


#define RF_FREQUENCY                                470000000 // Hz

#define TX_OUTPUT_POWER                             15        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
void enter_sleep(void);
typedef enum
{
    STATUS_LOWPOWER,
    STATUS_RX,
    STATUS_TX
}States_t;


int16_t txNumber;
States_t state;
bool sleepMode = false;
int16_t Rssi,rxSize;

uint32_t  license[4] = {0xD5397DF0, 0x8573F814, 0x7A38C73D, 0x48E68607};
extern "C" void SX126xWriteRegister( uint16_t address, uint8_t value );
extern "C" uint8_t SX126xReadRegister( uint16_t address );
// Add your initialization code here
void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Mcu.init(license);
    txNumber=0;
    Rssi=0;

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxDone = OnRxDone;

    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
    state=STATUS_TX;
    pinMode(0,INPUT_PULLUP);
    attachInterrupt(0, enter_sleep, FALLING);

}


void loop()
{
  switch(state)
  {
    case STATUS_TX:
      delay(1000);
      txNumber++;
        sprintf(txpacket,"%s","hello");
        sprintf(txpacket+strlen(txpacket),"%d",txNumber);
        sprintf(txpacket+strlen(txpacket),"%s"," Rssi : ");
        sprintf(txpacket+strlen(txpacket),"%d",Rssi);

        Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

        Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
        state=STATUS_LOWPOWER;
        break;
    case STATUS_RX:
      Serial.println("into RX mode");
        Radio.Rx( 0 );
        state=STATUS_LOWPOWER;
        break;
    case STATUS_LOWPOWER:
        LoRaWAN.sleep(CLASS_C,0);
        break;
        default:
            break;
  }
}

void OnTxDone( void )
{
  Serial.print("TX done......");
  state=STATUS_RX;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.print("TX Timeout......");
    state=STATUS_TX;
}
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    Radio.Sleep( );

    Serial.printf("\r\nreceived packet \"%s\" with Rssi %d , length %d\r\n",rxpacket,Rssi,rxSize);
    Serial.println("wait to send next packet");

    state=STATUS_TX;
}

void enter_sleep(void)
{
    Radio.Sleep( );
    pinMode(32,ANALOG);
    pinMode(33,ANALOG);
    pinMode(27,ANALOG);
    pinMode(26,ANALOG);
    pinMode(27,ANALOG);
    pinMode(14,ANALOG);


    //touch mode
    pinMode(2,ANALOG);
    pinMode(12,ANALOG);
    pinMode(13,ANALOG);
    pinMode(15,ANALOG);
    pinMode(15,ANALOG);


    pinMode(RADIO_NSS,INPUT_PULLUP);
    pinMode(LORA_MISO,ANALOG);
    pinMode(RADIO_DIO_1,ANALOG);
    pinMode(LORA_MOSI,ANALOG);
    pinMode(LORA_CLK,ANALOG);
    pinMode(RADIO_RESET,ANALOG);
    pinMode(RADIO_BUSY,ANALOG);
    // esp_sleep_enable_timer_wakeup((6*60000*(uint64_t)1000);
    esp_deep_sleep_start();

}