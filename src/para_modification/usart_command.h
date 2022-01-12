#ifndef _USART_COMMAND_H_
#define _USART_COMMAND_H_


void usart_rec_process(void);
void read_config(uint8_t * dev_eui,uint8_t * app_key,char * qrcode_str);


#endif
