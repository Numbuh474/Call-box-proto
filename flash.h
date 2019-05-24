#include <msp430.h>

//Flash definitions
#define FLASH_BLOCK_ADDRESS 0x1000
#define FLASH_BLOCK_VERSION_SIZE 0x02
#define FLASH_BUTTON_ID_SIZE 0x04
#define FLASH_BLOCK_VERSION_OFFSET 0x00
#define FLASH_BLOCK_VALID_IDS      0x02
#define FLASH_BUTTON_ID_1_OFFET    0x04
#define FLASH_BUTTON_ID_2_OFFET    0x08
#define FLASH_BUTTON_ID_3_OFFET    0x0C
#define FLASH_BUTTON_ID_4_OFFET    0x10
#define FLASH_BLOCK_SIZE (FLASH_BUTTON_ID_4_OFFET + FLASH_BUTTON_ID_SIZE)

#define FLASH_DATA_VERSION 0x01
#define FLASH_DATA_VERSION_UNKNOWN 0xFFFF



void init_flash(void);
unsigned char flash_write(char *data_ptr, unsigned int data_size, unsigned int offset);
unsigned char flash_read(char *data_ret_ptr, unsigned int data_size, unsigned int offset);
char flash_read_byte(unsigned int offset);
