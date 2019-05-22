#include <msp430.h>
#include <stddef.h>
//flash structure
typedef struct
{
    unsigned int flash_struct_ver;
    unsigned int num_of_valid_ids;
    unsigned long button_id[4];
} flash_data_struct_t;

//Flash definitions
#define FLASH_BLOCK_ADDRESS 0x1000
#define FLASH_BLOCK_VERSION_SIZE    sizeof(int)//0x02
#define FLASH_BUTTON_ID_SIZE        sizeof(long)//0x04
#define FLASH_BLOCK_VERSION_OFFSET  offsetof(flash_data_struct_t,flash_struct_ver)//0x00
#define FLASH_BLOCK_VALID_IDS       offsetof(flash_data_struct_t,num_of_valid_ids)//0x02
#define FLASH_BUTTON_ID_1_OFFET     offsetof(flash_data_struct_t,button_id[0])//0x04
#define FLASH_BUTTON_ID_2_OFFET     offsetof(flash_data_struct_t,button_id[1])//0x08
#define FLASH_BUTTON_ID_3_OFFET     offsetof(flash_data_struct_t,button_id[2])//0x0C
#define FLASH_BUTTON_ID_4_OFFET     offsetof(flash_data_struct_t,button_id[3])//0x10
#define FLASH_BLOCK_SIZE            sizeof(flash_data_struct_t)//(FLASH_BUTTON_ID_4_OFFET + FLASH_BUTTON_ID_SIZE)

#define FLASH_DATA_VERSION 0x01
#define FLASH_DATA_VERSION_UNKNOWN 0xFFFF

#define FLASH_BUTTON_ID_LENGTH      4


void init_flash(void);
unsigned char flash_write(char *data_ptr, unsigned int data_size, unsigned int offset);
unsigned char flash_read(char *data_ret_ptr, unsigned int data_size, unsigned int offset);
char flash_read_byte(unsigned int offset);
