#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include <string.h>
#include "nvs_storage.h"
#include "Arduino.h"


#define NAMESPACE_NAME "storage"

const char * nvs_errors[] = { "OTHER", "NOT_INITIALIZED", "NOT_FOUND", "TYPE_MISMATCH", "READ_ONLY", "NOT_ENOUGH_SPACE", "INVALID_NAME", "INVALID_HANDLE", "REMOVE_FAILED", "KEY_TOO_LONG", "PAGE_FULL", "INVALID_STATE", "INVALID_LENGTH"};
#define nvs_error(e) (((e)>ESP_ERR_NVS_BASE)?nvs_errors[(e)&~(ESP_ERR_NVS_BASE)]:nvs_errors[0])

nvs_storage::nvs_storage(void)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    err = nvs_open(NAMESPACE_NAME, NVS_READWRITE, &_handle);
	if (err != ESP_OK)
	{
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return ;
	}

}
nvs_storage::~nvs_storage(void)
{
	nvs_close(_handle);
}



size_t nvs_storage::nvs_storage_get_string(const char* key, char* value, const size_t maxLen)
{
    size_t len = 0;
   	esp_err_t err;
    if( !key || !value || !maxLen)
    {
        return 0;
    }
    err = nvs_get_str(_handle, key, NULL, &len);
    if(err)
    {
        log_e("nvs_get_str len fail: %s %s", key, nvs_error(err));
        return 0;
    }
    if(len > maxLen)
    {
        log_e("not enough space in value: %u < %u", maxLen, len);
        return 0;
    }
    err = nvs_get_str(_handle, key, value, &len);
    if(err)
    {
        log_e("nvs_get_str fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return len;
}

size_t nvs_storage::nvs_storage_set_string(const char* key, const char* value)
{
    if(!key || !value )
    {
        return 0;
    }
    esp_err_t err = nvs_set_str(_handle, key, value);
    if(err)
    {
        log_e("nvs_set_str fail: %s %s", key, nvs_error(err));
        return 0;
    }
    err = nvs_commit(_handle);
    if(err)
    {
        log_e("nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }
    return strlen(value);
}



