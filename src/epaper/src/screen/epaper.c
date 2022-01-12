
#include "../../inc/epaper.h"

void epaper_init(e_paper_function_t *epaper_refresh_function)
{
#ifdef EPAPER_BW_213

	//黑白 2.13 250x122
	const e_paper_global_function_t bw_213_global_fun={
	    bw_213_init,
	    bw_213_clear,
	    bw_213_screen_white,
	    bw_213_screen_black,
	    bw_213_deep_sleep,
	    bw_213_all_image,
	    bw_213_power_sleep_hold_en,
	    bw_213_power_sleep_hold_dis,
	};


	const e_paper_part_function_t bw_213_part_fun={
	 bw_213_part_clear,
	 bw_213_part_draw_chinese_gbk_by_num,
	 bw_213_part_draw_string_at,
	 bw_213_part_draw_qrcode,
	 
	 bw_213_part_set_height,
	 bw_213_part_set_width,
	 bw_213_part_set_rotate,
	 bw_213_part_get_width,
	 bw_213_part_get_height,
	 bw_213_part_get_rotate,
	 bw_213_part_set_frame_memory,
	 bw_213_part_update,
	 bw_213_part_on, 
	 bw_213_part_off,
	};

	epaper_refresh_function->global_refresh_function = bw_213_global_fun;
	epaper_refresh_function->part_refresh_function = bw_213_part_fun;
	epaper_refresh_function->width = 122;
	epaper_refresh_function->high = 250;
}
#endif