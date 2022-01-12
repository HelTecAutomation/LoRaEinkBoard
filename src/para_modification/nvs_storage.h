#ifndef _NVS_STORAGE_H_
#define _NVS_STORAGE_H_
#include "nvs.h"


class nvs_storage
{
public:
	
	nvs_storage(void);
	~nvs_storage(void);

	size_t nvs_storage_set_string(const char* key, const char* value);
	
	size_t nvs_storage_get_string(const char* key, char* value, const size_t maxLen);
private:
	nvs_handle _handle;

};




#endif
