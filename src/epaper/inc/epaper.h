#ifndef E_PAPER_H_
#define E_PAPER_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "../src/internal_inc/epaper_fonts.h"
#include "../src/internal_inc/epaper_picture.h"
#include "../src/internal_inc/epaper_bw_213.h"
#include "../src/internal_inc/epaper_log.h"

#define EPAPER_WHITE          0x01
#define EPAPER_BLACK          0x00

#define ROTATE_0    0
#define ROTATE_90   1
#define ROTATE_180  2
#define ROTATE_270  3


#define EPAPER_BW_213
// #define EPAPER_BW_290
// #define EPAPER_BW_154


typedef struct e_paper_global_function
{
    void (*epaper_init)(void);
    void (*epaper_clear)(void);
    void (*epaper_screen_White)(void);
    void (*epaper_screen_black)(void);
    void (*epaper_deep_sleep)(void);
    void (*epaper_all_image)(const uint8_t *data1);
    void (*epaper_power_deep_sleep_hold_en)(void);
	void (*epaper_power_deep_sleep_hold_dis)(void);

}e_paper_global_function_t;

typedef struct e_paper_part_function
{

//以下是局部刷新数据转换相关函数
void (*epaper_part_clear)(uint8_t colored);

void (*epaper_part_draw_chinese_gbk_by_num)(uint16_t Xstart, uint16_t Ystart, const uint8_t * array,uint8_t array_len, font_gbk_t* font,
                        uint8_t Color_Foreground, uint8_t Color_Background);



void (*epaper_part_draw_string_at)(uint16_t x, uint16_t y, const char* text, uint8_t text_len,font_en_t* font, uint8_t colored);

void (*epaper_part_draw_qrcode)(const int x, const int y,const char *text,const int  graphics_magnification);

void (*epaper_part_set_height)(uint16_t high);
void (*epaper_part_set_width)(uint16_t wid);
void (*epaper_part_set_rotate)(uint8_t rota);
uint16_t (*epaper_part_get_width)(void);
uint16_t (*epaper_part_get_height)(void);
uint8_t (*epaper_part_get_rotate)(void);

//局部刷新内存更新函数
void (*epaper_part_set_frame_memory)(uint16_t x_start,uint16_t y_start);
void (*epaper_part_update)(void);
void (*epaper_part_on)(void);
void (*epaper_part_off)(void);


}e_paper_part_function_t;


typedef struct e_paper_function
{
    e_paper_global_function_t global_refresh_function;
    e_paper_part_function_t  part_refresh_function;
    uint16_t width;
    uint16_t high;
}e_paper_function_t;



void epaper_init(e_paper_function_t *epaper_refresh_function);

#ifdef __cplusplus
}
#endif


#endif
