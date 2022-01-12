#include "Arduino.h"
#include "usart_command.h"
#include "nvs_storage.h"
#include <rom/rtc.h>

/*
	dev_eui=2232330000888801
	app_key=88888888888888888888888888886601
	qrcode_info={"DeviceName":"ESP32V2","ProductId":"WBGEBNMT8U","Signature":"a1457642cd514aa187d3e620a8542752"}
*/

/*
    1/115200 =8.6 us
*/
#define POWERON_RESET 1  //硬件复位标识（上电复位标识）
#define SW_CPU_RESET  12  //软件复位CPU标识

#define REC_TIMEOUT 1000
#define DELIMITER '='

#define DEV_EUI_KEY "dev_eui"
#define APP_KEY_KEY "app_key"
#define QRCODE_INFO "qrcode_info"

static int info_storage( const char *str, const char * substr );
static int str_to_array(const char* str, uint8_t* arr, uint16_t size);


void usart_rec_process(void)
{
	char buf[256];	
	char *ptr =NULL;
	uint8_t rec_count = 0;
	uint16_t rec_timeout =REC_TIMEOUT;

	uint16_t enter_timeout =10000; //10s
	uint8_t enter_flag =0;
	
	//只有首次启动的时候才会进入判断界面
	if((rtc_get_reset_reason(0)==POWERON_RESET) ||(rtc_get_reset_reason(0)==SW_CPU_RESET)
		||(rtc_get_reset_reason(1)==POWERON_RESET) ||(rtc_get_reset_reason(1)==SW_CPU_RESET))
	{
		delay(1000);
		while(Serial.available())
		{
			Serial.read(); //清除串口缓存,防止误触发
			delay(1);
		}
		printf("Within 10 seconds, send any data to the serial port and enter the configuration mode.\r\n");
		while(enter_timeout--)
		{
			if(Serial.available() > 0)
			{
				enter_flag =1;
				while(Serial.available())
				{
					Serial.read(); //将第一次数据清空
					delay(1);  
				}
				printf("Please enter the data in the following format!\rOnly enter one at a time!\r");
				printf("dev_eui=2232330000888801\rapp_key=88888888888888888888888888886601\r");
				printf("qrcode_info={\"DeviceName\":\"ESP32V2\",\"ProductId\":\"WBGEBNMT8U\",\"Signature\":\"a1457642cd514aa187d3e620a8542752\"}\r\n");
				break;
			}
			delay(1);
		}
	
		while(enter_flag) 
		{
			if (Serial.available() > 0) 
			{
				// read the incoming byte:
				buf[rec_count ++] = Serial.read();
				rec_timeout = REC_TIMEOUT;
			}
			else
			{
				if(rec_timeout > 0)
				{
					rec_timeout --;
					delayMicroseconds(5);  //10 us
				}
			}

			if(rec_count>0 && rec_timeout ==0)
			{  
				//停止接收，开始处理数据
				buf[rec_count]= '\0';

				Serial.print(buf);
				if(info_storage(buf,DEV_EUI_KEY)>=0)
				{

				}
				else if(info_storage(buf,APP_KEY_KEY)>=0)
				{

				}
				else if(info_storage(buf,QRCODE_INFO)>=0)
				{

				}
				else
				{
					Serial.println("Please check the format for invalid information!!!");
				}
				rec_count = 0;
			}

		}
	}

}


static int info_storage( const char *str, const char * substr )
{
	char *ptr =NULL;	
	nvs_storage nvs_process;
	size_t len =0;
	ptr = strstr(  str,substr );
	if(ptr != NULL)
	{
		ptr = strchr(  ptr,DELIMITER);
		if(ptr!= NULL)
		{
			ptr++; //Cross equals sign
			len = nvs_process.nvs_storage_set_string(substr, ptr);
			if(len >0)
			{
//				Serial.printf("successful%d\r\n",strlen(ptr));
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

void read_config(uint8_t * dev_eui,uint8_t * app_key,char * qrcode_str)
{
	if((rtc_get_reset_reason(0)==POWERON_RESET) ||(rtc_get_reset_reason(0)==SW_CPU_RESET)
		||(rtc_get_reset_reason(1)==POWERON_RESET) ||(rtc_get_reset_reason(1)==SW_CPU_RESET))
	{
		const uint8_t dev_eui_len = 8;
		const uint8_t app_key_len = 16;
		char buf[256];
		size_t buf_szie = 256;
		size_t rec_buf_size = 0;
		nvs_storage nvs_process;
		
		rec_buf_size = nvs_process.nvs_storage_get_string(DEV_EUI_KEY, buf,buf_szie);
		if(rec_buf_size !=0)
		{
			str_to_array(buf,dev_eui,dev_eui_len*2);
		}
		printf("%s\r",buf);
		
		rec_buf_size = nvs_process.nvs_storage_get_string(APP_KEY_KEY, buf,buf_szie);
		if(rec_buf_size !=0)
		{
			str_to_array(buf,app_key,app_key_len*2);
		}
		printf("%s\r",buf);

		rec_buf_size = nvs_process.nvs_storage_get_string(QRCODE_INFO, buf,buf_szie);
		if(rec_buf_size !=0)
		{
			strcpy(qrcode_str,buf);
		}
		printf("%s\r\n",qrcode_str);
	}
}

static int str_to_array(const char* str, uint8_t* arr, uint16_t size)
{
	uint8_t temp =0;
	uint8_t data =0;
	uint16_t count = 0;
	char *str_temp = (char *)str;
	if ((size % 2) != 0)
	{
		printf("config Parameter error, please enter a hexadecimal number\r\n");
		return -1;
	}
	while (count < size)
	{
		count++;
		switch (*str_temp++)
		{
		case '0':
			temp = 0x00;
			break;
		case '1':
			temp = 0x01;
			break;
		case '2':
			temp = 0x02;
			break;
		case '3':
			temp = 0x03;
			break;
		case '4':
			temp = 0x04;
			break;
		case '5':
			temp = 0x05;
			break;
		case '6':
			temp = 0x06;
			break;
		case '7':
			temp = 0x07;
			break;
		case '8':
			temp = 0x08;
			break;
		case '9':
			temp = 0x09;
			break;
		case 'a':
		case 'A':
			temp = 0x0a;
			break;
		case 'b':
		case 'B':
			temp = 0x0b;
			break;
		case 'c':
		case 'C':
			temp = 0x0c;
			break;
		case 'd':
		case 'D':
			temp = 0x0d;
			break;
		case 'e':
		case 'E':
			temp = 0x0e;
			break;
		case 'f':
		case 'F':
			temp = 0x0f;
			break;
		default:
			return -1;
		}
		if (count % 2 == 0)
		{
			arr[(count / 2) - 1] = data + temp;
		}
		else
		{
			data = temp << 4;
		}
	}
	return 0;
}


