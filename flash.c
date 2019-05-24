#include <msp430.h>
#include <flash.h>


void init_flash(void)
{
    // Configure Flash Timing Generator with MCLK/3
    FCTL2 = FWKEY + FSSEL0 + FN1; 
}

unsigned char flash_write(char *data_ptr, unsigned int data_size, unsigned int offset)
{
    char *flash_data_partition_ptr;
    unsigned int index;
    unsigned char ret_val = 0;
    
    if((data_ptr != 0) && (data_size != 0) && ((data_size + offset) < 64))
    {
        //set flash module address
        flash_data_partition_ptr = (char *) (FLASH_BLOCK_ADDRESS + offset);

        //first erase segment
        FCTL1 = FWKEY + ERASE;
        FCTL3 = FWKEY;
        *flash_data_partition_ptr = 0;
        
        //set flash part for write mode
        FCTL1 = FWKEY + WRT;
        
        //write data to the block
        for (index=0; index<data_size; index++)
        {
            *flash_data_partition_ptr++ = data_ptr[index];
        }
        
        //clear write bit and lock segment
        FCTL1 = FWKEY;
        FCTL3 = FWKEY + LOCK;
        
        ret_val = 1;
    }
    return ret_val;
}

unsigned char flash_read(char *data_ret_ptr, unsigned int data_size, unsigned int offset)
{
    char *flash_data_partition_ptr;
    unsigned int index;
    unsigned char ret_val = 0;
    
     //verify pointer is not null and size range
    if((data_ret_ptr != 0) && (data_size > 0) && ((data_size + offset) < 64))
    {
        //set flash module address
        flash_data_partition_ptr = (char *) (FLASH_BLOCK_ADDRESS + offset);
        
        //read data 
        for (index=0; index<data_size; index++)
        {
            data_ret_ptr[index] = *flash_data_partition_ptr++;
        }
        
        ret_val = 1;
    }
    return ret_val;
}

char flash_read_byte(unsigned int offset)
{
    char *flash_data_partition_ptr = (char *) (FLASH_BLOCK_ADDRESS + offset);
    
    char ret_byte = *flash_data_partition_ptr;
    
    return ret_byte;
}

