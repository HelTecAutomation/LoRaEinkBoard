#ifndef _EPAPER_BW_213_H
#define _EPAPER_BW_213_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "epaper_spi.h"
#include "epaper_fonts.h"
#include "epaper_qrcodegen.h"

void bw_213_init(void);

void bw_213_deep_sleep(void);
void bw_213_screen_black(void);
void bw_213_screen_white(void);
void bw_213_screen_Red(void);
void bw_213_all_image(const uint8_t *data1);
void bw_213_clear(void);



//以下是局部刷新数据转换相关函数
void bw_213_part_clear(uint8_t colored);
void bw_213_part_set_height(uint16_t high);
void bw_213_part_set_width(uint16_t wid);
void bw_213_part_set_rotate(uint8_t rota);
void bw_213_part_set_image(uint8_t* img);
uint8_t* bw_213_part_get_image(void);
uint16_t bw_213_part_get_width(void);
uint16_t bw_213_part_get_height(void);
uint8_t bw_213_part_get_rotate(void);
void bw_213_part_draw_pixel(uint16_t x, uint16_t y, uint8_t colored);
void bw_213_part_draw_char_at(uint16_t x, uint16_t y, char ascii_char, font_en_t* font, uint8_t colored);
void bw_213_part_draw_string_at(uint16_t x, uint16_t y, const char* text, uint8_t text_len,font_en_t* font, uint8_t colored);
void bw_213_part_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t colored);
void bw_213_part_draw_horizontal_line(uint16_t x, uint16_t y, uint16_t line_width, uint8_t colored);
void bw_213_part_draw_vertical_line(uint16_t x, uint16_t y, uint16_t line_height, uint8_t colored);
void bw_213_part_draw_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t colored);
void bw_213_part_draw_filled_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t colored);
void bw_213_part_draw_filled_circle(uint16_t x, uint16_t y, uint16_t radius, uint8_t colored);
void bw_213_part_draw_qrcode(const int x, const int y,const char *text,const int  graphics_magnification);


//局部刷新内存更新函数

void bw_213_part_set_frame_memory(uint16_t x_start,uint16_t y_start);
void bw_213_part_display_part_base_image(const uint8_t* frame_buffer);
void bw_213_part_update(void);
void bw_213_part_on(void);
void bw_213_part_off(void);



void bw_213_part_draw_chinese_gbk_by_str(uint16_t Xstart, uint16_t Ystart, const uint8_t * pString, font_gbk_t* font,
                        uint8_t Color_Foreground, uint8_t Color_Background);
                    
void bw_213_part_draw_chinese_gbk_by_num(uint16_t Xstart, uint16_t Ystart, const uint8_t * array,uint8_t array_len, font_gbk_t* font,
                        uint8_t Color_Foreground, uint8_t Color_Background);
void bw_213_power_sleep_hold_dis(void);
void bw_213_power_sleep_hold_en(void);


#ifdef __cplusplus
}
#endif
#endif
