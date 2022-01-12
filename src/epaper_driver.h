#ifndef _EPAPER_DRIVER_H_
#define _EPAPER_DRIVER_H_
#include "epaper/inc/epaper.h"

class epaper_class
{
public:
	void epaper_driver_init(void);
	void epaper_display_data(uint8_t *rec_array, const uint8_t rec_array_len);
	void epaper_power_deep_sleep_hold_en(void);
	void epaper_power_deep_sleep_hold_dis(void);

private:
	
};
extern RTC_DATA_ATTR char qr_code_arr[];


#endif
