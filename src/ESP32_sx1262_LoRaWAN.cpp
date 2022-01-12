#include <ESP32_sx1262_LoRaWAN.h>
#include <Arduino.h>
#include <rom/rtc.h>


#if defined( REGION_EU868 )
#include "loramac/region/RegionEU868.h"
#elif defined( REGION_EU433 )
#include "loramac/region/RegionEU433.h"
#elif defined( REGION_KR920 )
#include "loramac/region/RegionKR920.h"
#elif defined( REGION_AS923) || defined( REGION_AS923_AS1) || defined( REGION_AS923_AS2)
#include "loramac/region/RegionAS923.h"
#endif

#ifdef CLASS_A_WOTA
	RTC_DATA_ATTR static uint16_t SendingCadSwitchingCycle =0;
#endif


#define POWERON_RESET 1  //硬件复位标识（上电复位标识）
#define SW_CPU_RESET  12  //软件复位CPU标识


/*loraWan default Dr when adr disabled*/
#ifdef REGION_US915
int8_t defaultDrForNoAdr = 3;
#else
int8_t defaultDrForNoAdr = 5;
#endif

/*AT mode, auto into low power mode*/
bool autoLPM = true;

/*loraWan current Dr when adr disabled*/
int8_t currentDrForNoAdr;

/*!
 * User application data size
 */
uint8_t appDataSize = 4;

/*!
 * User application data
 */
uint8_t appData[LORAWAN_APP_DATA_MAX_SIZE];


/*!
 * Defines the application data transmission duty cycle
 */
uint32_t txDutyCycleTime ;

/*!
 * Timer to handle the application data transmission duty cycle
 */
TimerEvent_t TxNextPacketTimer;

/*!
 * PassthroughMode mode enable/disable. don't modify it here. 
 * when use PassthroughMode, set it true in app.ino , Reference the example PassthroughMode.ino 
 */
bool passthroughMode = false;

/*!
 * when use PassthroughMode, Mode_LoraWan to set use lora or lorawan mode . don't modify it here. 
 * it is used to set mode lora/lorawan in PassthroughMode.
 */
bool modeLoraWan = true;

/*!
 * Indicates if a new packet can be sent
 */
static bool nextTx = true;


enum eDeviceState_LoraWan deviceState;


/*!
 * \brief   Prepares the payload of the frame
 *
 * \retval  [0: frame could be send, 1: error]
 */
bool SendFrame( void )
{
	lwan_dev_params_update();
	
	McpsReq_t mcpsReq;
	LoRaMacTxInfo_t txInfo;
	LORAWANLOG;
	if( LoRaMacQueryTxPossible( appDataSize, &txInfo ) != LORAMAC_STATUS_OK )
	{
		// Send empty frame in order to flush MAC commands
		printf("payload length error ...\r\n");
		mcpsReq.Type = MCPS_UNCONFIRMED;
		mcpsReq.Req.Unconfirmed.fBuffer = NULL;
		mcpsReq.Req.Unconfirmed.fBufferSize = 0;
		mcpsReq.Req.Unconfirmed.Datarate = currentDrForNoAdr;
		//return false;
	}
	else
	{
		if( isTxConfirmed == false )
		{
			printf("unconfirmed uplink sending ...\r\n");
			mcpsReq.Type = MCPS_UNCONFIRMED;
			mcpsReq.Req.Unconfirmed.fPort = appPort;
			mcpsReq.Req.Unconfirmed.fBuffer = appData;
			mcpsReq.Req.Unconfirmed.fBufferSize = appDataSize;
			mcpsReq.Req.Unconfirmed.Datarate = currentDrForNoAdr;
		}
		else
		{
			printf("confirmed uplink sending ...\r\n");
			mcpsReq.Type = MCPS_CONFIRMED;
			mcpsReq.Req.Confirmed.fPort = appPort;
			mcpsReq.Req.Confirmed.fBuffer = appData;
			mcpsReq.Req.Confirmed.fBufferSize = appDataSize;
			mcpsReq.Req.Confirmed.NbTrials = confirmedNbTrials;
			mcpsReq.Req.Confirmed.Datarate = currentDrForNoAdr;
		}
	}

	if( LoRaMacMcpsRequest( &mcpsReq ) == LORAMAC_STATUS_OK )
	{
		return false;
	}
	return true;
}

/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static void OnTxNextPacketTimerEvent( void )
{
	MibRequestConfirm_t mibReq;
	LoRaMacStatus_t status;

	TimerStop( &TxNextPacketTimer );

	mibReq.Type = MIB_NETWORK_JOINED;
	status = LoRaMacMibGetRequestConfirm( &mibReq );

	if( status == LORAMAC_STATUS_OK )
	{
		if( mibReq.Param.IsNetworkJoined == true )
		{
			deviceState = DEVICE_STATE_SEND;
			nextTx = true;
		}
		else
		{
			// Network not joined yet. Try to join again
			MlmeReq_t mlmeReq;
			mlmeReq.Type = MLME_JOIN;
			mlmeReq.Req.Join.DevEui = devEui;
			mlmeReq.Req.Join.AppEui = appEui;
			mlmeReq.Req.Join.AppKey = appKey;
			mlmeReq.Req.Join.NbTrials = 1;

			if( LoRaMacMlmeRequest( &mlmeReq ) == LORAMAC_STATUS_OK )
			{
				deviceState = DEVICE_STATE_SLEEP;
			}
			else
			{
				deviceState = DEVICE_STATE_CYCLE;
			}
		}
	}
}

/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] mcpsConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
	if( mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
	{
		switch( mcpsConfirm->McpsRequest )
		{
			case MCPS_UNCONFIRMED:
			{
				// Check Datarate
				// Check TxPower
				break;
			}
			case MCPS_CONFIRMED:
			{
				// Check Datarate
				// Check TxPower
				// Check AckReceived
				// Check NbTrials
				break;
			}
			case MCPS_PROPRIETARY:
			{
				break;
			}
			default:
				break;
		}
	}
	nextTx = true;
}





void __attribute__((weak)) downLinkAckHandle()
{
	//printf("ack received\r\n");
}

void __attribute__((weak)) downLinkDataHandle(McpsIndication_t *mcpsIndication)
{
	printf("+REV DATA:%s,RXSIZE %d,PORT %d\r\n",mcpsIndication->RxSlot?"RXWIN2":"RXWIN1",mcpsIndication->BufferSize,mcpsIndication->Port);
	printf("+REV DATA:");
	for(uint8_t i=0;i<mcpsIndication->BufferSize;i++)
	{
		printf("%02X",mcpsIndication->Buffer[i]);
	}
	printf("\r\n");
}

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
int revrssi;
static void McpsIndication( McpsIndication_t *mcpsIndication )
{
	if( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
	{
		return;
	}
#if defined(CubeCell_BoardPlus)||defined(CubeCell_GPS)
	ifDisplayAck=1;
	revrssi=mcpsIndication->Rssi;
#endif
#if (LoraWan_RGB==1)
	turnOnRGB(COLOR_RECEIVED, 200);
	turnOffRGB();
#endif
	LORAWANLOG;
	printf( "received ");
	switch( mcpsIndication->McpsIndication )
	{
		case MCPS_UNCONFIRMED:
		{
			printf( "unconfirmed ");
			break;
		}
		case MCPS_CONFIRMED:
		{
			printf( "confirmed ");
			OnTxNextPacketTimerEvent( );
			break;
		}
		case MCPS_PROPRIETARY:
		{
			printf( "proprietary ");
			break;
		}
		case MCPS_MULTICAST:
		{
			printf( "multicast ");
			break;
		}
		default:
			break;
	}
	printf( "downlink: rssi = %d, snr = %d, datarate = %d\r\n", mcpsIndication->Rssi, (int)mcpsIndication->Snr,(int)mcpsIndication->RxDoneDatarate);

	if(mcpsIndication->AckReceived)
	{
		downLinkAckHandle();
	}

	if( mcpsIndication->RxData == true )
	{
		downLinkDataHandle(mcpsIndication);
	}

	if( mcpsIndication->FramePending == true )
	{
		// The server signals that it has pending data to be sent.
		// We schedule an uplink as soon as possible to flush the server.
		OnTxNextPacketTimerEvent( );
	}

	delay(10);
}


void __attribute__((weak)) dev_time_updated()
{
	printf("device time updated\r\n");
}

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] mlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
	switch( mlmeConfirm->MlmeRequest )
	{
		case MLME_JOIN:
		{
			if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
			{

#if (LoraWan_RGB==1)
				turnOnRGB(COLOR_JOINED,500);
				turnOffRGB();
#endif
#if defined(CubeCell_BoardPlus)||defined(CubeCell_GPS)
				if(isDispayOn)
				{
					LoRaWAN.displayJoined();
				}
#endif
				LORAWANLOG;
				printf("joined\r\n");
				
				//in PassthroughMode,do nothing while joined
				if(passthroughMode == false)
				{
					// Status is OK, node has joined the network
					deviceState = DEVICE_STATE_SEND;
				}
			}
			else
			{
				uint32_t rejoin_delay = 30000;
				printf("join failed, join again at 30s later\r\n");
				delay(5);
				TimerSetValue( &TxNextPacketTimer, rejoin_delay );
				TimerStart( &TxNextPacketTimer );
			}
			break;
		}
		case MLME_LINK_CHECK:
		{
			if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
			{
				// Check DemodMargin
				// Check NbGateways
			}
			break;
		}
		case MLME_DEVICE_TIME:
		{
			if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
			{
				dev_time_updated();
			}
			break;
		}
		default:
			break;
	}
	nextTx = true;
}

/*!
 * \brief   MLME-Indication event function
 *
 * \param   [IN] mlmeIndication - Pointer to the indication structure.
 */
static void MlmeIndication( MlmeIndication_t *mlmeIndication )
{
	switch( mlmeIndication->MlmeIndication )
	{
		case MLME_SCHEDULE_UPLINK:
		{// The MAC signals that we shall provide an uplink as soon as possible
			OnTxNextPacketTimerEvent( );
			break;
		}
		default:
			break;
	}
}


void lwan_dev_params_update( void )
{
#if defined( REGION_EU868 )
	LoRaMacChannelAdd( 3, ( ChannelParams_t )EU868_LC4 );
	LoRaMacChannelAdd( 4, ( ChannelParams_t )EU868_LC5 );
	LoRaMacChannelAdd( 5, ( ChannelParams_t )EU868_LC6 );
	LoRaMacChannelAdd( 6, ( ChannelParams_t )EU868_LC7 );
	LoRaMacChannelAdd( 7, ( ChannelParams_t )EU868_LC8 );
#elif defined( REGION_EU433 )
	LoRaMacChannelAdd( 3, ( ChannelParams_t )EU433_LC4 );
	LoRaMacChannelAdd( 4, ( ChannelParams_t )EU433_LC5 );
	LoRaMacChannelAdd( 5, ( ChannelParams_t )EU433_LC6 );
	LoRaMacChannelAdd( 6, ( ChannelParams_t )EU433_LC7 );
	LoRaMacChannelAdd( 7, ( ChannelParams_t )EU433_LC8 );
#elif defined( REGION_KR920 )
	LoRaMacChannelAdd( 3, ( ChannelParams_t )KR920_LC4 );
	LoRaMacChannelAdd( 4, ( ChannelParams_t )KR920_LC5 );
	LoRaMacChannelAdd( 5, ( ChannelParams_t )KR920_LC6 );
	LoRaMacChannelAdd( 6, ( ChannelParams_t )KR920_LC7 );
	LoRaMacChannelAdd( 7, ( ChannelParams_t )KR920_LC8 );
#elif defined( REGION_AS923 ) || defined( REGION_AS923_AS1 ) || defined( REGION_AS923_AS2 )
	LoRaMacChannelAdd( 2, ( ChannelParams_t )AS923_LC3 );
	LoRaMacChannelAdd( 3, ( ChannelParams_t )AS923_LC4 );
	LoRaMacChannelAdd( 4, ( ChannelParams_t )AS923_LC5 );
	LoRaMacChannelAdd( 5, ( ChannelParams_t )AS923_LC6 );
	LoRaMacChannelAdd( 6, ( ChannelParams_t )AS923_LC7 );
	LoRaMacChannelAdd( 7, ( ChannelParams_t )AS923_LC8 );
#endif

	MibRequestConfirm_t mibReq;

	mibReq.Type = MIB_CHANNELS_DEFAULT_MASK;
	mibReq.Param.ChannelsMask = userChannelsMask;
	LoRaMacMibSetRequestConfirm(&mibReq);

	mibReq.Type = MIB_CHANNELS_MASK;
	mibReq.Param.ChannelsMask = userChannelsMask;
	LoRaMacMibSetRequestConfirm(&mibReq);
}



LoRaMacPrimitives_t LoRaMacPrimitive;
LoRaMacCallback_t LoRaMacCallback;

void LoRaWanClass::generateDeveuiByChipID()
{
	uint32_t uniqueId[2];
#if defined(__asr6601__)
	system_get_chip_id(uniqueId);
#elif defined(__asr650x__)
	CyGetUniqueId(uniqueId);
#elif defined(ESP_PLATFORM)
	uint64_t id = getID();
	uniqueId[0]=(uint32_t)(id>>32);
	uniqueId[1]=(uint32_t)id;
#endif
	for(int i=0;i<8;i++)
	{
		if(i<4)
			devEui[i] = (uniqueId[1]>>(8*(3-i)))&0xFF;
		else
			devEui[i] = (uniqueId[0]>>(8*(7-i)))&0xFF;
	}
}


void LoRaWanClass::init(DeviceClass_t lorawanClass,LoRaMacRegion_t region)
{
	if((rtc_get_reset_reason(0)==POWERON_RESET) ||(rtc_get_reset_reason(0)==SW_CPU_RESET)
		||(rtc_get_reset_reason(1)==POWERON_RESET) ||(rtc_get_reset_reason(1)==SW_CPU_RESET))
	{
		Serial.println();
		LORAWANLOG;
		Serial.print("LoRaWAN ");
		switch(region)
		{
			case LORAMAC_REGION_AS923_AS1:
				Serial.print("AS923(AS1:922.0-923.4MHz)");
				break;
			case LORAMAC_REGION_AS923_AS2:
				Serial.print("AS923(AS2:923.2-924.6MHz)");
				break;
			case LORAMAC_REGION_AU915:
				Serial.print("AU915");
				break;
			case LORAMAC_REGION_CN470:
				Serial.print("CN470");
				break;
			case LORAMAC_REGION_CN779:
				Serial.print("CN779");
				break;
			case LORAMAC_REGION_EU433:
				Serial.print("EU433");
				break;
			case LORAMAC_REGION_EU868:
				Serial.print("EU868");
				break;
			case LORAMAC_REGION_KR920:
				Serial.print("KR920");
				break;
			case LORAMAC_REGION_IN865:
				Serial.print("IN865");
				break;
			case LORAMAC_REGION_US915:
				Serial.print("US915");
				break;
			case LORAMAC_REGION_US915_HYBRID:
				Serial.print("US915_HYBRID ");
				break;
			default:
				break;
		}
		Serial.printf(" Class %X start!\r\n\r\n",loraWanClass+10);
	}
	if(region == LORAMAC_REGION_AS923_AS1 || region == LORAMAC_REGION_AS923_AS2)
		region = LORAMAC_REGION_AS923;
	MibRequestConfirm_t mibReq;

	LoRaMacPrimitive.MacMcpsConfirm = McpsConfirm;
	LoRaMacPrimitive.MacMcpsIndication = McpsIndication;
	LoRaMacPrimitive.MacMlmeConfirm = MlmeConfirm;
	LoRaMacPrimitive.MacMlmeIndication = MlmeIndication;
	LoRaMacCallback.GetBatteryLevel = BoardGetBatteryLevel;
	LoRaMacCallback.GetTemperatureLevel = NULL;

	LoRaMacInitialization( &LoRaMacPrimitive, &LoRaMacCallback,region);
	TimerStop( &TxNextPacketTimer );

	TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent );

    if(IsLoRaMacNetworkJoined==false)
    {
	mibReq.Type = MIB_ADR;
	mibReq.Param.AdrEnable = loraWanAdr;
	LoRaMacMibSetRequestConfirm( &mibReq );

	mibReq.Type = MIB_PUBLIC_NETWORK;
	mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
	LoRaMacMibSetRequestConfirm( &mibReq );

	lwan_dev_params_update();

	mibReq.Type = MIB_DEVICE_CLASS;
	LoRaMacMibGetRequestConfirm( &mibReq );
	
	if(loraWanClass != mibReq.Param.Class)
	{
		mibReq.Param.Class = loraWanClass;
		LoRaMacMibSetRequestConfirm( &mibReq );
	}

	deviceState = DEVICE_STATE_JOIN;
	}
    else
    {
#ifdef CLASS_A_WOTA
		if(SendingCadSwitchingCycle <=0)
		{
			deviceState = DEVICE_STATE_SEND;
		}
		else
		{
			deviceState = DEVICE_STATE_SLEEP;
			SendingCadSwitchingCycle -=1;
//			printf("SendingCadSwitchingCycle = %d\r\n",SendingCadSwitchingCycle);
		}
#else
		deviceState = DEVICE_STATE_SEND;
#endif
  	  
    }
}


void LoRaWanClass::join()
{
	if( overTheAirActivation )
	{
		LORAWANLOG;
		Serial.println("joining...");
		MlmeReq_t mlmeReq;
		
		mlmeReq.Type = MLME_JOIN;

		mlmeReq.Req.Join.DevEui = devEui;
		mlmeReq.Req.Join.AppEui = appEui;
		mlmeReq.Req.Join.AppKey = appKey;
		mlmeReq.Req.Join.NbTrials = 1;

		if( LoRaMacMlmeRequest( &mlmeReq ) == LORAMAC_STATUS_OK )
		{
			deviceState = DEVICE_STATE_SLEEP;
		}
		else
		{
			deviceState = DEVICE_STATE_CYCLE;
		}
	}
	else
	{
		MibRequestConfirm_t mibReq;

		mibReq.Type = MIB_NET_ID;
		mibReq.Param.NetID = LORAWAN_NETWORK_ID;
		LoRaMacMibSetRequestConfirm( &mibReq );

		mibReq.Type = MIB_DEV_ADDR;
		mibReq.Param.DevAddr = devAddr;
		LoRaMacMibSetRequestConfirm( &mibReq );

		mibReq.Type = MIB_NWK_SKEY;
		mibReq.Param.NwkSKey = nwkSKey;
		LoRaMacMibSetRequestConfirm( &mibReq );

		mibReq.Type = MIB_APP_SKEY;
		mibReq.Param.AppSKey = appSKey;
		LoRaMacMibSetRequestConfirm( &mibReq );

		mibReq.Type = MIB_NETWORK_JOINED;
		mibReq.Param.IsNetworkJoined = true;
		LoRaMacMibSetRequestConfirm( &mibReq );
		
		deviceState = DEVICE_STATE_SEND;
	}
}

void LoRaWanClass::send()
{
	if( nextTx == true )
	{
		MibRequestConfirm_t mibReq;
		mibReq.Type = MIB_DEVICE_CLASS;
		LoRaMacMibGetRequestConfirm( &mibReq );

		if(loraWanClass != mibReq.Param.Class)
		{
			mibReq.Param.Class = loraWanClass;
			LoRaMacMibSetRequestConfirm( &mibReq );
		}
		nextTx = SendFrame( );
	}
}

void LoRaWanClass::cycle(uint32_t dutyCycle)
{
	TimerSetValue( &TxNextPacketTimer, dutyCycle );
	TimerStart( &TxNextPacketTimer );
#ifdef CLASS_A_WOTA
	SendingCadSwitchingCycle = dutyCycle/wota_cycle_time +1;
#endif
}

void LoRaWanClass::sleep(DeviceClass_t classMode,uint8_t debugLevel)
{
	Radio.IrqProcess( );
	
#ifdef CLASS_A_WOTA
	Mcu.sleep(classMode,1,debugLevel);
#else
	Mcu.sleep(classMode,0,debugLevel);
#endif
}
void LoRaWanClass::setDataRateForNoADR(int8_t dataRate)
{
	defaultDrForNoAdr = dataRate;
}


LoRaWanClass LoRaWAN;










