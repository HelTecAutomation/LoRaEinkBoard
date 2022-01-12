/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EPAPER_FONTS_H
#define __EPAPER_FONTS_H

/* Max size of bitmap will based on a font24 (17x24) */
// #define MAX_HEIGHT_FONT         24
// #define MAX_WIDTH_FONT          17
// #define OFFSET_BITMAP           54

/*×î´ó×ÖÌåÎ¢ÈíÑÅºÚ24 (32x41) */
#define MAX_HEIGHT_FONT         41
#define MAX_WIDTH_FONT          32
#define OFFSET_BITMAP     

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>



typedef struct
{    
  const uint8_t *table1;
  const uint8_t *table2;
  uint16_t single_font_size;
  
  uint16_t Width;
  uint16_t Height;
  
}font_gbk_t;

typedef struct 
{    
  const uint8_t *table;
  uint16_t Width;
  uint16_t Height;
  
} font_en_t;


extern font_en_t font_19x35_en ;
extern font_gbk_t font_gbk;
int8_t switch_to_gbk(const uint8_t* pszBufIn, uint16_t nBufInLen, uint8_t* pszBufOut, uint16_t* pnBufOutLen);

#ifdef __cplusplus
}
#endif
  
#endif /* __FONTS_H */
 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
