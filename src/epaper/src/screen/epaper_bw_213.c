#include "../internal_inc/epaper_bw_213.h"
#include "math.h"
#include "../internal_inc/epaper_log.h"
#include "freertos/task.h"

#define MAX_QRCODE_VERSION_213 10   //二维码版本

/* EPD commands */
#define DRIVER_OUTPUT_CONTROL                       0x01
#define BOOSTER_SOFT_START_CONTROL                  0x0C
#define GATE_SCAN_START_POSITION                    0x0F
#define DEEP_SLEEP_MODE                             0x10
#define DATA_ENTRY_MODE_SETTING                     0x11
#define SW_RESET                                    0x12
#define TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_1                    0x21
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_DUMMY_LINE_PERIOD                       0x3A
#define SET_GATE_TIME                               0x3B
#define BORDER_WAVEFORM_CONTROL                     0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define TERMINATE_FRAME_READ_WRITE                  0xFF

#define FONT_WIDTH (36.0) 
#define REAL_MAX   (122.0 -FONT_WIDTH)
#define REAL_MIN   (0.0)
#define MAP_MAX     95    //最好定义为8的整数倍
#define MAP_MIN    (9.0)  //实际起始地址来决定的

/* Display resolution */
#define EPAPER_213_WIDTH       136 //122手动修正为136 因为这款芯片的起始地址是不显示第一个字节的（不显示坐标为0-8）。修正之后使用区间映射来实现  
#define EPAPER_213_HEIGHT      250 //这个数字不用修正，没有任何改变
#define EPAPER_213_ALLSCREEN_GRAGHBYTES	4000


// Color inverse. 1 or 0 = set or reset a bit if set a colored pixel
#define IF_INVERT_COLOR     1

// Display orientation
#define EPAPER_213_ROTATE_0            0
#define EPAPER_213_ROTATE_90           1
#define EPAPER_213_ROTATE_180          2
#define EPAPER_213_ROTATE_270          3


//Partial refresh cache size
#define PART_REFRESH_CACHE_SIZE (1024*12)


//width，height 定义需要刷新区域的大小
static volatile uint16_t epaper_213_width = 0;
static volatile uint16_t epaper_213_height = 0;
static volatile uint8_t  epaper_213_rotate = 0;
static volatile uint8_t* epaper_213_image;
static volatile uint8_t epaper_213_array_image[PART_REFRESH_CACHE_SIZE];


static float bw_213_get_interval_map(uint16_t real_val)
{
    return ((MAP_MAX - MAP_MIN)/(REAL_MAX - REAL_MIN)*(real_val -REAL_MIN) +MAP_MIN);
}

void bw_213_Wait_until_idle(void) 
{
	//保证不会在这个地方卡死
	uint8_t wait_timeout  = 60;
    while((gpio_get_level(PIN_NUM_BUSY) == 1) && (wait_timeout --))  //1==HIGH
    {      //LOW: idle, HIGH: busy
        vTaskDelay(100);
    }      
}
/************************************** init ************************************************/
void bw_213_init(void) 
{
	epaper_poweron();
	epaper_spi_init();
    epaper_pin_init();

	epaper_reset();

    bw_213_Wait_until_idle();
    epaper_SendCmd(0x12); // soft reset
    bw_213_Wait_until_idle();

    //初始化部分参数
    bw_213_part_set_image((uint8_t *)epaper_213_array_image);
    bw_213_part_set_width(0);
    bw_213_part_set_height(0);
}
/********************************* update ***************************************************/
static void  bw_213_update(void) 
{   


    epaper_SendCmd(0x20);
    bw_213_Wait_until_idle();
    vTaskDelay(10);
}
/****************************** All screen update *******************************************/
void bw_213_all_image(const uint8_t *datas) 
{

    epaper_SendCmd(0x24);   //write RAM for black(0)/white (1)

	for (int i = 0; i < EPAPER_213_ALLSCREEN_GRAGHBYTES; i++) 
    {
        epaper_SendData(datas[i]);
    }	

   	bw_213_update();		 
}


/********************************** Load Data ***********************************************/
static void  bw_213_load_data(uint8_t  data) 
{
    uint32_t i,k;
    epaper_SendCmd(0x24);   //write RAM for black(0)/white (1)
	for(k=0;k<250;k++) 
    {
        for(i=0;i<16;i++) 
        {
            epaper_SendData(data);
        }
    }
	
	bw_213_update();
}
/********************************** deep sleep **********************************************/
void  bw_213_deep_sleep(void)
{  	
	bw_213_Wait_until_idle();
    epaper_SendCmd(0x10); //enter deep sleep
    epaper_SendData(0x01); 
  	epaper_pin_deinit();
    epaper_spi_deinit();  //SPI去初始化
    
    epaper_poweroff();
}

/********************************* Display All Black ****************************************/
void  bw_213_screen_black(void) 
{
    bw_213_load_data(0x00);
}

/********************************* Display All White ****************************************/
void  bw_213_screen_white(void) 
{
    bw_213_load_data(0XFF);
}




/**
 *  @brief: private function to specify the memory area for data R/W
 */
static void bw_213_part_set_memory_area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end) 
{
    epaper_SendCmd(0x11); //data entry mode
    epaper_SendData(0x03);
	epaper_SendCmd(0x44);
    // epaper_SendCmd(SET_RAM_X_ADDRESS_START_END_POSITION);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    epaper_SendData((x_start >> 3) & 0xFF);
    epaper_SendData((x_end >> 3) & 0xFF);
    epaper_SendCmd(0x45);
    epaper_SendData(y_start & 0xFF);
    epaper_SendData((y_start >> 8) & 0xFF);
    epaper_SendData(y_end & 0xFF);
    epaper_SendData((y_end >> 8) & 0xFF);
}

/**
 *  @brief: private function to specify the start point for data R/W
 */
static void bw_213_part_set_memory_pointer(uint16_t x, uint16_t y) 
{
    epaper_SendCmd(0x4E);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    epaper_SendData((x >> 3) & 0xFF);
    epaper_SendCmd(0x4F);
    epaper_SendData(y & 0xFF);
    epaper_SendData((y >> 8) & 0xFF);
}

/**
 *  @brief: put an image buffer to the frame memory.
 *          this won't update the display.
 */
void bw_213_part_set_frame_memory(uint16_t x_start,uint16_t y_start)
{
    uint16_t x_end;
    uint16_t y_end;
    const uint8_t* image_buffer = bw_213_part_get_image();
    uint16_t image_width = bw_213_part_get_width();
    uint16_t image_height = bw_213_part_get_height();
    

    x_start = bw_213_get_interval_map(x_start);
    if (image_buffer == NULL ) 
    {
        return;
    }
    /* x_start point must be the multiple of 8 or the last 3 bits will be ignored */
    x_start &= 0xF8;
    image_width &= 0xF8;
    if (x_start + image_width >= EPAPER_213_WIDTH) 
    {
        x_end = EPAPER_213_WIDTH - 1;
    } 
    else 
    {
        x_end = x_start + image_width - 1;
    }
    if (y_start + image_height >= EPAPER_213_HEIGHT) 
    {
        y_end = EPAPER_213_HEIGHT - 1;
    } 
    else 
    {
        y_end = y_start + image_height - 1;
    }
    bw_213_part_set_memory_area(x_start, y_start, x_end, y_end);
    bw_213_part_set_memory_pointer(x_start, y_start);
    epaper_SendCmd(0x24);
    /* send the image data */
    for (int j = 0; j < y_end - y_start + 1; j++) 
    {
        for (int i = 0; i < (x_end - x_start + 1) / 8; i++) 
        {
            epaper_SendData(image_buffer[i + j * (image_width / 8)]);
        }
    }
    
}



/**
 *  @brief: clear the frame memory with the specified color.
 *          this won't update the display.
 */
void bw_213_part_clear_frame_memory(uint8_t color) 
{

    epaper_SendCmd(WRITE_RAM);
    /* send the color data */
    for (int i = 0; i < epaper_213_width / 8 * epaper_213_height; i++) 
    {
        epaper_SendData(color);
    }
}
/**
 *  @brief: update the display
 *          there are 2 memory areas embedded in the e-paper display
 *          but once this function is called,
 *          the the next action of SetFrameMemory or ClearFrame will 
 *          set the other memory area.
 */
void bw_213_part_update(void) 
{


    epaper_SendCmd(0x22);
    epaper_SendData(0xc7);
    epaper_SendCmd(0x20);
    // epaper_SendCmd(0xFF);
    bw_213_Wait_until_idle();

}


/**
 *  @brief: After this command is transmitted, the chip would enter the 
 *          deep-sleep mode to save power. 
 *          The deep sleep mode would return to standby by hardware reset. 
 *          You can use Epd::Init() to awaken
 */
void bw_213_part_Sleep() 
{
    epaper_SendCmd(DEEP_SLEEP_MODE);
    bw_213_Wait_until_idle();
}


///////////////////////////////////////////////////////////////////////////////******************************************************

static void bw_213_part_draw_absolute_pixel(uint16_t x, uint16_t y, uint8_t colored) 
{
    if ( x >= epaper_213_width || y >= epaper_213_height) 
    {
        return;
    }
    if (IF_INVERT_COLOR) 
    {
        if (colored) 
        {
            epaper_213_image[(x + y * epaper_213_width) / 8] |= 0x80 >> (x % 8);
        } 
        else 
        {
            epaper_213_image[(x + y * epaper_213_width) / 8] &= ~(0x80 >> (x % 8));
        }
    } 
    else 
    {
        if (colored) 
        {
            epaper_213_image[(x + y * epaper_213_width) / 8] &= ~(0x80 >> (x % 8));
        } else 
        {
            epaper_213_image[(x + y * epaper_213_width) / 8] |= 0x80 >> (x % 8);
        }
    }
}



//将需要刷新的区域先进行一个清理（全黑，或全白），
//这里只是对数组区域进行清理，并不是真的刷新
void bw_213_part_clear(uint8_t colored)
{
    for (uint16_t x = 0; x < epaper_213_width; x++)
    {
        for (uint16_t y = 0; y < epaper_213_height; y++)
        {
            bw_213_part_draw_absolute_pixel(x, y, colored);
        }
    }
}


/**
 *  @brief: Getters and Setters
 */
void bw_213_part_set_height(uint16_t high)
{
    epaper_213_height =  high;
}


void bw_213_part_set_width(uint16_t wid)
{
    epaper_213_width = wid % 8 ? wid + 8 - (wid % 8) : wid;
}


void bw_213_part_set_rotate(uint8_t rota)
{
    epaper_213_rotate = rota;
}

void bw_213_part_set_image(uint8_t* image)
{
    epaper_213_image =  image;
}

uint8_t* bw_213_part_get_image(void) 
{
    return (uint8_t*)epaper_213_image;
}


uint16_t bw_213_part_get_width(void) 
{
    return epaper_213_width;
}


uint16_t bw_213_part_get_height(void) 
{

    return epaper_213_height;
}


uint8_t bw_213_part_get_rotate(void) 
{
    return epaper_213_rotate;
}



/**
 *  @brief: this draws a pixel by the coordinates
 */
void bw_213_part_draw_pixel(uint16_t x, uint16_t y, uint8_t colored)
 {
    uint16_t point_temp;
    if (epaper_213_rotate == EPAPER_213_ROTATE_0) 
    {
        if( x >= epaper_213_width ||  y >= epaper_213_height)
        {
            return;
        }
        bw_213_part_draw_absolute_pixel(x, y, colored);
    }
    else if (epaper_213_rotate == EPAPER_213_ROTATE_90) 
    {
        if(x >= epaper_213_height ||  y >= epaper_213_width) 
        {
          return;
        }
        point_temp = x;
        x = epaper_213_width - y;
        y = point_temp;
        bw_213_part_draw_absolute_pixel(x, y, colored);
    } 
    else if (epaper_213_rotate == EPAPER_213_ROTATE_180) 
    {
        if(x >= epaper_213_width || y >= epaper_213_height)
        {
          return;
        }
        x = epaper_213_width - x;
        y = epaper_213_height - y;
        bw_213_part_draw_absolute_pixel(x, y, colored);
    }
    else if (epaper_213_rotate == EPAPER_213_ROTATE_270) 
    {
        if( x >= epaper_213_height || y >= epaper_213_width)
        {
          return;
        }
        point_temp = x;
        x = y;
        y = epaper_213_height - point_temp;
        bw_213_part_draw_absolute_pixel(x, y, colored);
    }
}


/**
 *  @brief: this draws a charactor on the frame buffer but not refresh
 */
void bw_213_part_draw_char_at(uint16_t x, uint16_t y, char ascii_char, font_en_t* font, uint8_t colored)
{
    uint16_t i, j;
    uint32_t char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    // EPAPER_DEBUG("ascii_char = %c char_offset = %d\r\n",ascii_char,char_offset);
    const uint8_t* ptr = &font->table[char_offset];

    for (j = 0; j < font->Height; j++) 
    {
        for (i = 0; i < font->Width; i++) 
        {
            if (*ptr & (0x80 >> (i % 8)))
            {
                bw_213_part_draw_pixel(x + i, y + j, colored);
            }
            if (i % 8 == 7)
            {
                ptr++;
            }
        }
        if (font->Width % 8 != 0) 
        {
            ptr++;
        }
    }
}


/**
*  @brief: this displays a string on the frame buffer but not refresh
* 这里的 x和y相当于偏移值。是相对于被选中区域(需要局部刷新的区域)而言。并不是相对于这个屏幕而言。
*/
void bw_213_part_draw_string_at(uint16_t x, uint16_t y, const char* text, uint8_t text_len,font_en_t* font, uint8_t colored)
{
    const char* p_text = text;
    uint8_t  counter = 0;
    uint16_t refcolumn = x;
    
    /* Send the string character by character on EPD */
    while (counter <text_len)
    {
        /* Display one character on EPD */
        bw_213_part_draw_char_at(refcolumn, y, p_text[counter], font, colored);
        /* Decrement the column position by 16 */
        refcolumn += font->Width;
        /* Point on the next character */
        counter++;
    }
}



/**
*  @brief: this draws a line on the frame buffer
*/
void bw_213_part_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t colored) 
{
    /* Bresenham algorithm */
    uint16_t dx = x1 - x0 >= 0 ? x1 - x0 : x0 - x1;
    uint16_t sx = x0 < x1 ? 1 : -1;
    uint16_t dy = y1 - y0 <= 0 ? y1 - y0 : y0 - y1;
    uint16_t sy = y0 < y1 ? 1 : -1;
    uint16_t err = dx + dy;

    while((x0 != x1) && (y0 != y1))
    {
        bw_213_part_draw_pixel(x0, y0 , colored);
        if (2 * err >= dy)
        {     
            err += dy;
            x0 += sx;
        }
        if (2 * err <= dx)
        {
            err += dx; 
            y0 += sy;
        }
    }
}



/**
*  @brief: this draws a horizontal line on the frame buffer
*/
void bw_213_part_draw_horizontal_line(uint16_t x, uint16_t y, uint16_t line_width, uint8_t colored) 
{
    uint16_t i;
    for (i = x; i < x + line_width; i++) 
    {
        bw_213_part_draw_pixel(i, y, colored);
    }
}

/**
*  @brief: this draws a vertical line on the frame buffer
*/
void bw_213_part_draw_vertical_line(uint16_t x, uint16_t y, uint16_t line_height, uint8_t colored) 
{
    uint16_t i;
    for (i = y; i < y + line_height; i++) 
    {
        bw_213_part_draw_pixel(x, i, colored);
    }
}

/**
*  @brief: this draws a rectangle
*/
void bw_213_part_draw_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t colored) 
{
    uint16_t min_x, min_y, max_x, max_y;
    min_x = x1 > x0 ? x0 : x1;
    max_x = x1 > x0 ? x1 : x0;
    min_y = y1 > y0 ? y0 : y1;
    max_y = y1 > y0 ? y1 : y0;
    
    bw_213_part_draw_horizontal_line(min_x, min_y, max_x - min_x + 1, colored);
    bw_213_part_draw_horizontal_line(min_x, max_y, max_x - min_x + 1, colored);
    bw_213_part_draw_vertical_line(min_x, min_y, max_y - min_y + 1, colored);
    bw_213_part_draw_vertical_line(max_x, min_y, max_y - min_y + 1, colored);
}

/**
*  @brief: this draws a filled rectangle
*/
void bw_213_part_draw_filled_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t colored) 
{
    uint16_t min_x, min_y, max_x, max_y;
    uint16_t i;
    min_x = x1 > x0 ? x0 : x1;
    max_x = x1 > x0 ? x1 : x0;
    min_y = y1 > y0 ? y0 : y1;
    max_y = y1 > y0 ? y1 : y0;
    
    for (i = min_x; i <= max_x; i++) 
    {
        bw_213_part_draw_vertical_line(i, min_y, max_y - min_y + 1, colored);
    }
}

/**
*  @brief: this draws a filled circle
*/
void bw_213_part_draw_filled_circle(uint16_t x, uint16_t y, uint16_t radius, uint8_t colored) 
{
    /* Bresenham algorithm */
    uint16_t x_pos = -radius;
    uint16_t y_pos = 0;
    uint16_t err = 2 - 2 * radius;
    uint16_t e2;

    do {
        bw_213_part_draw_pixel(x - x_pos, y + y_pos, colored);
        bw_213_part_draw_pixel(x + x_pos, y + y_pos, colored);
        bw_213_part_draw_pixel(x + x_pos, y - y_pos, colored);
        bw_213_part_draw_pixel(x - x_pos, y - y_pos, colored);
        bw_213_part_draw_horizontal_line(x + x_pos, y + y_pos, 2 * (-x_pos) + 1, colored);
        bw_213_part_draw_horizontal_line(x + x_pos, y - y_pos, 2 * (-x_pos) + 1, colored);
        e2 = err;
        if (e2 <= y_pos)
        {
            err += ++y_pos * 2 + 1;
            if(-x_pos == y_pos && e2 <= x_pos) 
            {
                e2 = 0;
            }
        }
        if(e2 > x_pos) 
        {
            err += ++x_pos * 2 + 1;
        }
    } while(x_pos <= 0);
}



//局部刷新内存更新函数


void bw_213_part_draw_chinese_gbk_by_str(uint16_t Xstart, uint16_t Ystart, const uint8_t * pString, font_gbk_t* font,
                        uint8_t Color_Foreground, uint8_t Color_Background)
{
    uint8_t p_text[50] ;
    uint16_t p_text_len = 0;
    uint16_t x = Xstart, y = Ystart;
    uint16_t i, j,Num = 0;

    uint8_t GBH,GBL;
    uint32_t Hp;
    const uint8_t* ptr;

    switch_to_gbk(pString, strlen((char *)pString), p_text, &p_text_len);
    uint8_t character_num =floor(p_text_len/2);//floor(strlen(p_text)/2);
    EPAPER_DEBUG("strlen((char *)pString)= %d p_text_len =  %d character_num =%d\r\n",strlen((char *)pString),p_text_len,character_num);
    
    /* Send the string character by character on EPD */
    while (character_num > 0) 
    {
        GBH = p_text[Num];
        GBL = p_text[Num+1];
        if(GBH <0xDF)
        {
            Hp=(((GBH-0xB0)*94)+GBL-0XA1)*(font->single_font_size);
            EPAPER_DEBUG("GBH %02X GBL %02X Hp %d\r\n",GBH,GBL,Hp);

            ptr = &(font->table1[Hp]);
        }
        else
        {
            Hp=(((GBH-0xDF)*94)+GBL-0XA1)*(font->single_font_size);
            EPAPER_DEBUG("GBH %02X GBL %02X Hp %d\r\n",GBH,GBL,Hp);

            ptr = &(font->table2[Hp]);
        }

        for (j = 0; j < font->Height; j++) 
        {
            for (i = 0; i < font->Width; i++) 
            {
                if (*ptr & (0x80 >> (i % 8))) 
                {
                    bw_213_part_draw_pixel(x + i, y + j, Color_Foreground);
                }
                if (i % 8 == 7) 
                {
                    ptr++;
                }
            }
            if (font->Width % 8 != 0) 
            {
                ptr++;
            }
        }
        /* Point on the next character */
        Num += 2;
        /* Decrement the column position by 16 */
        x += font->Width;
        character_num -=1;
    }
}




void bw_213_part_draw_chinese_gbk_by_num(uint16_t Xstart, uint16_t Ystart, const uint8_t * array,uint8_t array_len, font_gbk_t* font,
                        uint8_t Color_Foreground, uint8_t Color_Background)
{
    // uint8_t *p_text ;
    // uint16_t p_text_len = 0;
    uint16_t x = Xstart, y = Ystart;
    uint16_t i, j,Num = 0;

    uint8_t GBH,GBL;
    uint32_t Hp;
    const uint8_t* ptr;

    // switch_to_gbk(pString, strlen((char *)pString), p_text, &p_text_len);
    // uint8_t character_num =floor(p_text_len/2);//floor(strlen(p_text)/2);
    // EPAPER_DEBUG("strlen((char *)pString)= %d p_text_len =  %d character_num =%d\r\n",strlen((char *)pString),p_text_len,character_num);
    const uint8_t *p_text = array;
    uint8_t character_num =floor(array_len/2);
    /* Send the string character by character on EPD */
    while (character_num > 0) 
    {
        GBH = p_text[Num];
        GBL = p_text[Num+1];
        if(GBH <0xDF)
        {
            Hp=(((GBH-0xB0)*94)+GBL-0XA1)*(font->single_font_size);
            EPAPER_DEBUG("GBH %02X GBL %02X Hp %d\r\n",GBH,GBL,Hp);

            ptr = &(font->table1[Hp]);
        }
        else
        {
            Hp=(((GBH-0xDF)*94)+GBL-0XA1)*(font->single_font_size);
            EPAPER_DEBUG("GBH %02X GBL %02X Hp %d\r\n",GBH,GBL,Hp);

            ptr = &(font->table2[Hp]);
        }

        for (j = 0; j < font->Height; j++) 
        {
            for (i = 0; i < font->Width; i++) 
            {
                if (*ptr & (0x80 >> (i % 8))) 
                {
                    bw_213_part_draw_pixel(x + i, y + j, Color_Foreground);
                }
                if (i % 8 == 7) 
                {
                    ptr++;
                }
            }
            if (font->Width % 8 != 0) 
            {
                ptr++;
            }
        }
        /* Point on the next character */
        Num += 2;
        /* Decrement the column position by 16 */
        x += font->Width;
        character_num -=1;
    }
}


/*
	 bw_213_part_draw_qrcode 函数和其它的字符显示函数不同。
	 为了防止出现二维码显示不全，或者空间浪费过大，所以这里的局部刷新大小一开始就是定好的的。
	 不需要外部再调用 bw_213_part_set_width bw_213_part_set_height bw_213_part_clear等三个函数，
	 当然，如果调用了也不会报错了，只是不会有作用而已。
	 其它的用法和一些draw函数时一样的。
*/
void bw_213_part_draw_qrcode(const int x, const int y,const char *text,const int  graphics_magnification) 
{
	int border = 2;
    uint16_t x_begin =x,  y_begin = y;
    enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;
    uint8_t *qrcode, *tempBuffer;
    esp_err_t err = ESP_FAIL;

    qrcode = (uint8_t *)calloc(1, qrcodegen_BUFFER_LEN_FOR_VERSION(MAX_QRCODE_VERSION_213));

    if (!qrcode) {
        return ;
    }

    tempBuffer = (uint8_t *)calloc(1, qrcodegen_BUFFER_LEN_FOR_VERSION(MAX_QRCODE_VERSION_213));

    if (!tempBuffer) {
        free(qrcode);
        return ;
    }

    // Make and print the QR Code symbol
    bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
                                   qrcodegen_VERSION_MIN, MAX_QRCODE_VERSION_213, qrcodegen_Mask_AUTO, true);

    if (ok) {
	    err = ESP_OK;
//        printQr(qrcode);        
		int size = qrcodegen_getSize(qrcode);
	    
		bw_213_part_set_width(graphics_magnification *(size+4*border ));
		bw_213_part_set_height(graphics_magnification *(size+4*border ));
		bw_213_part_clear(1);
	    for (int y_offset = -border; y_offset < size + border; y_offset += 1) 
	    {
	        for (int x_offset = -border; x_offset < size + border; x_offset += 1) 
	        {

	            if (qrcodegen_getModule(qrcode, x_offset, y_offset)) 
	            {
	            	for(int i =0; i<graphics_magnification;i++)
	            	{
	            		for(int j =0; j<graphics_magnification; j++)
	            		{
							bw_213_part_draw_pixel(x_begin +graphics_magnification*(x_offset+border)+i ,y_begin+ graphics_magnification*(y_offset+border) +j , 1); 
	            		}
	            	}
	                
	            }
	            else
	            {
	            	for(int i =0; i<graphics_magnification;i++)
	            	{
	            		for(int j =0; j<graphics_magnification; j++)
	            		{
							bw_213_part_draw_pixel(x_begin +graphics_magnification*(x_offset+border)+i ,y_begin+ graphics_magnification*(y_offset+border) +j , 0); 
	            		}
	            	}
	            }
	        }
	    }
	}
//    printf("end \n");
    free(qrcode);
    free(tempBuffer);
    return ;

}



void bw_213_part_on(void)
{
	epaper_poweron();
	epaper_spi_init();
    epaper_pin_init();
    bw_213_Wait_until_idle();

   //初始化部分参数
    bw_213_part_set_image((uint8_t *)epaper_213_array_image);
    bw_213_part_set_width(0);
    bw_213_part_set_height(0);
}

void bw_213_part_off(void)
{
	bw_213_Wait_until_idle(); 
  	epaper_pin_deinit();
    epaper_spi_deinit();  //SPI去初始化
    epaper_poweroff();
}
void bw_213_power_sleep_hold_dis(void)
{
	epaper_power_sleep_hold_dis();
}
void bw_213_power_sleep_hold_en(void)
{
	epaper_power_sleep_hold_en();
}


void bw_213_clear(void)
{
	uint16_t w, h;
	w = (EPAPER_213_WIDTH % 8 == 0)? (EPAPER_213_WIDTH / 8 ): (EPAPER_213_WIDTH / 8 + 1);
	h = EPAPER_213_HEIGHT;
 
	epaper_SendCmd(0x24);
	for (uint16_t j = 0; j < h; j++) 
    {
		for (uint16_t i = 0; i < w; i++) 
        {
			epaper_SendData(0xff);
		}
	}
	//DISPLAY REFRESH
	bw_213_update();
}
