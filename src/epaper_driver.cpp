#include "epaper_driver.h"
#include <rom/rtc.h>
#include "Arduino.h"
#include <string.h>

RTC_DATA_ATTR static e_paper_function_t epaper_refresh_function;

#define POWERON_RESET 1  //硬件复位标识（上电复位标识）
#define SW_CPU_RESET  12  //软件复位CPU标识

#define OPEN_SCREEN_TEXT_X_START 0
#define OPEN_SCREEN_TEXT_Y_START 0
#define COMPANY_X_START          0
#define COMPANY_Y_START          0
#define TITLE_X_START            40
#define TITLE_Y_START            0
#define NAME_X_START             80
#define NAME_Y_START             0


#define EPAPER_DATA_MAX_NUM 36  //Number of parameters.

typedef enum
{
	open_screen_text =1,
	company,
	title,
	name
}select_flag_t;

typedef struct 
{
	select_flag_t select_flag;
	uint16_t x_start;
	uint16_t y_start;
	uint8_t angle;
	uint8_t foreground_color;
	uint8_t background_color;
	uint8_t data_len;
	uint8_t data[EPAPER_DATA_MAX_NUM];
}epaper_data_t;

void en_cn_mix_cache(	epaper_data_t epaper_data);

void epaper_class::epaper_driver_init(void)
{
	
	epaper_init(&epaper_refresh_function);
	
	if((rtc_get_reset_reason(0)==POWERON_RESET) ||(rtc_get_reset_reason(0)==SW_CPU_RESET)
      ||(rtc_get_reset_reason(1)==POWERON_RESET) ||(rtc_get_reset_reason(1)==SW_CPU_RESET))
  	{
		epaper_refresh_function.global_refresh_function.epaper_init();
		vTaskDelay(10);
		epaper_refresh_function.global_refresh_function.epaper_all_image(gImage_213image_bw);
		vTaskDelay(20);

		epaper_refresh_function.part_refresh_function.epaper_part_draw_qrcode(0,0,qr_code_arr,3);

		epaper_refresh_function.part_refresh_function.epaper_part_set_frame_memory(0, 0);
		epaper_refresh_function.part_refresh_function.epaper_part_update();
		vTaskDelay(20);
		epaper_refresh_function.global_refresh_function.epaper_deep_sleep();
	}
	else
	{
		//do nothing
	}

}
void epaper_class::epaper_display_data(uint8_t *rec_array, const uint8_t rec_array_len)
{
	uint8_t data_num =0;
	uint8_t data_offset = 0;
	epaper_data_t epaper_data;

	
	epaper_refresh_function.global_refresh_function.epaper_init();
	vTaskDelay(5);
	epaper_refresh_function.global_refresh_function.epaper_clear();
	vTaskDelay(5);
	while(data_offset <rec_array_len )
	{
		memset(&epaper_data,0,sizeof(epaper_data_t));
		epaper_data.select_flag = (select_flag_t)rec_array[data_offset++];
		epaper_data.data_len = rec_array[data_offset++];
		for(data_num = 0;data_num <epaper_data.data_len;data_num++)
		{
			epaper_data.data[data_num] = rec_array[data_offset+data_num];		
		}
		data_offset += data_num;
		
		switch(epaper_data.select_flag)
		{

			case open_screen_text:
			{
				epaper_data.angle = ROTATE_270;
				epaper_data.x_start = OPEN_SCREEN_TEXT_X_START;
				epaper_data.y_start = OPEN_SCREEN_TEXT_Y_START;
				epaper_data.foreground_color = EPAPER_BLACK;
				epaper_data.background_color = EPAPER_WHITE;
				en_cn_mix_cache(epaper_data);
				break;
			};
			case company:
			{
				epaper_data.angle = ROTATE_270;
				epaper_data.x_start = COMPANY_X_START;
				epaper_data.y_start = COMPANY_Y_START;
				epaper_data.foreground_color = EPAPER_BLACK;
				epaper_data.background_color = EPAPER_WHITE;
				
				en_cn_mix_cache(epaper_data);
				break;
			};
			case title:
			{
				epaper_data.angle = ROTATE_270;
				epaper_data.x_start = TITLE_X_START;
				epaper_data.y_start = TITLE_Y_START;
				epaper_data.foreground_color = EPAPER_BLACK;
				epaper_data.background_color = EPAPER_WHITE;
				
				en_cn_mix_cache(epaper_data);

				break;
			};
			case name:
			{
				epaper_data.angle = ROTATE_270;
				epaper_data.x_start = NAME_X_START;
				epaper_data.y_start = NAME_Y_START;
				epaper_data.foreground_color = EPAPER_BLACK;
				epaper_data.background_color = EPAPER_WHITE;
				
				en_cn_mix_cache(epaper_data);

				break;
			};
			default:
			break;
		}
	}
	epaper_refresh_function.part_refresh_function.epaper_part_update();
	vTaskDelay(20);
	epaper_refresh_function.global_refresh_function.epaper_deep_sleep();
}





void epaper_class::epaper_power_deep_sleep_hold_en(void)
{
	epaper_refresh_function.global_refresh_function.epaper_power_deep_sleep_hold_en();
}
void epaper_class::epaper_power_deep_sleep_hold_dis(void)
{
	epaper_refresh_function.global_refresh_function.epaper_power_deep_sleep_hold_dis();
}


void en_cn_mix_cache(	epaper_data_t epaper_data)
{
	uint8_t data_num =epaper_data.data_len; 
	uint8_t take_out_num = 0;
	uint8_t  display_data[2]={0,0};
	uint16_t x_offset = 0;
	uint16_t y_offset = 0;
//	epaper_refresh_function.part_refresh_function.epaper_part_on();
	vTaskDelay(2);
	while(data_num >take_out_num)
	{
		if(epaper_data.data[take_out_num] &(0x01 <<7)) //通过最高位来判断，是英文还是中文数据
		{
			//最高位是1， 代表是中文数据
			if((epaper_data.data[take_out_num] < 0xB0) || (epaper_data.data[take_out_num]> 0xF7))
			{
				//跳过未取模区域
				take_out_num+=2;
				continue;
			}
			epaper_refresh_function.part_refresh_function.epaper_part_set_width(font_gbk.Height);
			epaper_refresh_function.part_refresh_function.epaper_part_set_height(font_gbk.Width);
			epaper_refresh_function.part_refresh_function.epaper_part_set_rotate(epaper_data.angle);				
			epaper_refresh_function.part_refresh_function.epaper_part_clear(epaper_data.background_color);

			display_data[0] = epaper_data.data[take_out_num];
			display_data[1] = epaper_data.data[take_out_num+1];

			epaper_refresh_function.part_refresh_function.epaper_part_draw_chinese_gbk_by_num(0,0,display_data,2,&font_gbk,epaper_data.foreground_color,epaper_data.foreground_color);
			epaper_refresh_function.part_refresh_function.epaper_part_set_frame_memory(epaper_data.x_start +x_offset,epaper_refresh_function.high -epaper_data.y_start- y_offset -font_gbk.Width);
			y_offset += font_gbk.Width;
			/*Wrap*/
			if((epaper_data.y_start +y_offset) > (epaper_refresh_function.high - font_gbk.Width))
			{
				/*font_gbk.Height+6 It is to prevent covering, and to make beautiful corrections. 42*/
				x_offset += font_gbk.Height+6;
				y_offset = 0 ;
				epaper_data.y_start = 0;
			}
			take_out_num+=2;
		}
		else
		{
			//最高位不是1， 代表是英文数据
			if((epaper_data.data[take_out_num] < 0x20) || (epaper_data.data[take_out_num]> 0x7F))
			{
				//跳过未取模区域
				take_out_num+=1;
				continue;
			}
			epaper_refresh_function.part_refresh_function.epaper_part_set_width(font_19x35_en.Height);
			epaper_refresh_function.part_refresh_function.epaper_part_set_height(font_19x35_en.Width);
			epaper_refresh_function.part_refresh_function.epaper_part_set_rotate(epaper_data.angle);

			display_data[0] = epaper_data.data[take_out_num];
			epaper_refresh_function.part_refresh_function.epaper_part_clear(epaper_data.background_color);
			epaper_refresh_function.part_refresh_function.epaper_part_draw_string_at(0,0,(const char *)display_data,1,&font_19x35_en,epaper_data.foreground_color);
			epaper_refresh_function.part_refresh_function.epaper_part_set_frame_memory(epaper_data.x_start +x_offset, epaper_refresh_function.high-epaper_data.y_start-y_offset- font_19x35_en.Width);
			x_offset += 0;
			y_offset += font_19x35_en.Width;
			/*Wrap*/
			if((epaper_data.y_start +y_offset) > (epaper_refresh_function.high - font_19x35_en.Width))
			{
				x_offset += font_19x35_en.Height;
				y_offset = 0 ;
				epaper_data.y_start = 0;
			}
			
			take_out_num +=1;
		}

	}

//	epaper_refresh_function.part_refresh_function.epaper_part_update();
//	vTaskDelay(20);
//	epaper_refresh_function.part_refresh_function.epaper_part_off();
}







