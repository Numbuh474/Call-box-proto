#include <flash.h>

#ifdef __MSP430G2553
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
#endif

#ifdef __msp430fr2355_H__
void init_flash()
{
    //password & program write protection
    SYSCFG0 = FRWPPW | PFWP;
    //enable reset on misread, enable power, enable power on lpm
    FRCTL0 = FRCTLPW | NWAITS_7;
    GCCTL0 = UBDRSTEN | FRPWR | FRLPMPWR;
    FRCTL0_H = NWAITS_7;
}
//changes the value at data_ret_ptr to the memory stored at offset. Returns 1 on success
unsigned char flash_read (char * data_ret_ptr, unsigned int data_size, unsigned int offset)
{
    if (data_size + offset >= INFO_LENGTH)
        return 0;

    char * memory = (char *)INFO_START;
    unsigned int i;
    for (i = 0; i<data_size; i++)
    {
        data_ret_ptr[i] = memory[i];
    }
    return 1;
}
//returns a byte from FRAM storage
char flash_read_byte (unsigned int offset)
{
    char * result = (char*)(FRAM_START + offset);
    return *result;
}
//saves the data at data_ptr to memory at offset. returns 1 on success
unsigned char flash_write(char * data_ptr, unsigned int data_size, unsigned int offset)
{
    //password, maximum wait states
    //FRCTL0 = FRCTLPW | NWAITS_7;
    if (data_size + offset >= INFO_LENGTH)
        return 0;

    char * memory = (char *)INFO_START;
    unsigned int i;
    for (i = 0; i<data_size; i++)
    {
        memory[i] = data_ptr[i];
    }
    return 1;
    //FRCTL0 = NWAITS_7;
}
#endif
