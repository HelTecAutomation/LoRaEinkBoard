#include "../internal_inc/epaper_spi.h"

spi_device_handle_t spi;

void epaper_SendCmd(uint8_t command)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&command;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

void epaper_SendData(const uint8_t data)
{
    esp_err_t ret;
    spi_transaction_t t;
    // if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=&data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.

}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void epaper_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

void epaper_poweron(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_POWER_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE ;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE ;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

	gpio_set_level(PIN_NUM_POWER,0);  // power on 

}

void epaper_poweroff(void)
{
	gpio_set_level(PIN_NUM_POWER,1);  // power off 
}

void epaper_power_sleep_hold_dis(void)
{
	gpio_hold_dis(PIN_NUM_POWER);
	gpio_deep_sleep_hold_dis();
}

void epaper_power_sleep_hold_en(void)
{
	gpio_hold_en(PIN_NUM_POWER );
	gpio_deep_sleep_hold_en();
}

void epaper_reset(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_RESET_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE ;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE ;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

	gpio_set_level(PIN_NUM_RESET,1);
	vTaskDelay(10);
	gpio_set_level(PIN_NUM_RESET,0);
	vTaskDelay(2);
	gpio_set_level(PIN_NUM_RESET,1);
	vTaskDelay(20);

}
void epaper_pin_init(void)
{
    // //DC init
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_DC_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE ;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_BUSY_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE ;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE ;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    
}
void epaper_spi_init(void)
{
    esp_err_t ret;
    
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=0 , //使用默认大小
    };
    spi_device_interface_config_t devcfg={
#ifdef CONFIG_LCD_OVERCLOCK
        .clock_speed_hz=26*1000*1000,           //Clock out at 26 MHz
#else
        .clock_speed_hz=10*1000*1000,           //Clock out at 10 MHz
#endif
        .mode=0,                                //SPI mode 0
        .spics_io_num=PIN_NUM_CS,               //CS pin
        .queue_size=7,//7,                          //We want to be able to queue 7 transactions at a time
        .pre_cb=epaper_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(EPAPER_HOST, &buscfg,0);// SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(EPAPER_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}


void epaper_spi_deinit(void)
{
    spi_bus_remove_device(spi);
    spi_bus_free(EPAPER_HOST);
}

void epaper_pin_deinit(void)
{
    gpio_reset_pin(PIN_NUM_DC);
    gpio_reset_pin(PIN_NUM_BUSY);
    gpio_reset_pin(PIN_NUM_RESET);
    gpio_reset_pin(PIN_NUM_POWER);
}


