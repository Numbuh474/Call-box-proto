#include "isd.h"


const isd_cmd_t pu =         {0x01,2,0};
const isd_cmd_t stop =       {0x02,2,0};
const isd_cmd_t reset =      {0x03,2,0};
const isd_cmd_t clr_int =    {0x04,2,0};
const isd_cmd_t rd_status =  {0x05,3,0};
const isd_cmd_t rd_play_ptr ={0x06,4,0};
const isd_cmd_t pd =         {0x07,2,0};
const isd_cmd_t rd_rec_ptr = {0x08,4,0};
const isd_cmd_t devid =      {0x09,3,0};
const isd_cmd_t play =       {0x40,2,0};
const isd_cmd_t rec =        {0x41,2,0};
const isd_cmd_t erase =      {0x42,2,0};
const isd_cmd_t g_erase =    {0x43,2,0};
const isd_cmd_t rd_apc =     {0x44,4,0};
const isd_cmd_t wr_apc1 =    {0x45,3,1};
const isd_cmd_t wr_apc2 =    {0x65,3,1};
const isd_cmd_t wr_nvcfg =   {0x46,2,0};
const isd_cmd_t ld_nvcfg =   {0x47,2,0};
const isd_cmd_t fwd =        {0x48,2,0};
const isd_cmd_t chk_mem =    {0x49,2,0};
const isd_cmd_t extclk =     {0x4a,2,0};
const isd_cmd_t set_play =   {0x80,7,2};
const isd_cmd_t set_rec =    {0x81,7,2};
const isd_cmd_t set_erase =  {0x82,7,2};
//initialize isd module
void init_isd()
{
    isd_cmd_len = 0;
    isd_rx_index = 0;
    isd_tx_index = 0;
    isd_timeout = 0;
    //master mode, 3 pin(SS controlled via software), synchronous
#ifdef __MSP430G2553__
    UCB0CTL0 = UCCKPL | UCMST | UCMODE_0 | UCSYNC;
    UCB0CTL1 = UCSSEL_2 | UCSWRST;
    //ucbclk = clk/512 (2KHz)
    UCB0BR0 = 0x00;
    UCB0BR1 = 0x01;
#endif
#ifdef __msp430fr2355_H__
    UCB0CTLW0 = UCCKPL | UCMST | UCMODE_0 | UCSYNC | UCSSEL_2 | UCSWRST;
    //ucbclk = clk/512 (2KHz)
    UCB0BRW = 0x0009;
#endif
    set_gpio_p1_high(GPIO_USCI_SS);
    //disable reset
    UCB0CTL1 &= ~(UCSWRST);
    //power up for SPI mode
    isd_transmit_validate(&pu,0,0);
    //assign memory space based on capacity of the device
    isd_transmit(&devid,0,0);
    unsigned int id = isd_read(2)>>3;
    //isd_transmit(&rd_status,0,0);
    //isd_read(0);
    switch(id) {
    case 0b11100:
    {
        isd_mem_max = ISD_MEM_17240;
        break;
    }
    case 0b11101:
    {
        isd_mem_max = ISD_MEM_17210;
        break;
    }
    case 0b11110:
    {
        isd_mem_max = ISD_MEM_17180;
        break;
    }
    case 0b11000:
    {
        isd_mem_max = ISD_MEM_17150;
        break;
    }
    case 0b11001:
    {
        isd_mem_max = ISD_MEM_17120;
        break;
    }
    case 0b11010:
    {
        isd_mem_max = ISD_MEM_1790;
        break;
    }
    case 0b10100:
    {
        isd_mem_max = ISD_MEM_1760;
        break;
    }
    case 0b10101:
    {
        isd_mem_max = ISD_MEM_1750;
        break;
    }
    case 0b10110:
    {
        isd_mem_max = ISD_MEM_1740;
        break;
    }
    case 0b10000:
    {
        isd_mem_max = ISD_MEM_1730;
        break;
    }
    default:
    {
        isd_mem_max = ISD_MEM_1760;
        //isd_mem_max = ISD_MEM_1730;
        //halt();
    }
    }//switch
    isd_mem_msg = (isd_mem_max - ISD_MEM_BEGIN)>>2;
    isd_ptr[0] = ISD_MEM_BEGIN;
    isd_ptr[1] = ISD_MEM_BEGIN+isd_mem_msg;
    isd_ptr[2] = ISD_MEM_BEGIN+(isd_mem_msg*2);
    isd_ptr[3] = ISD_MEM_BEGIN+(isd_mem_msg*3);
    //set device to end at EOM.
    //get current APC settings
    isd_transmit(&rd_apc,0,0);
    //put APC register into an int
    unsigned int apc = isd_read(3);//assign high byte of APC
    apc = (apc<<8) | isd_read(2);//assign low byte of APC
    apc |= ISD_EOM_ENABLE;//set desired data

    isd_transmit(&clr_int,0,0);
    isd_wait_ready();
    isd_transmit(&wr_apc2,apc,0);
    isd_wait_ready();
}
//wait until isd can accept another command
void isd_wait_ready()
{
    isd_timeout = 0x1000;
    do
    {
        isd_transmit(&rd_status,0,0);
        isd_timeout--;
        if (!isd_timeout)
            halt(isdTimeoutError);
    }   while(!(isd_read(2)&ISD_RDY));
}
//send a signal and ensure it was read correctly.
void isd_transmit_validate(const isd_cmd_t* command, unsigned int data, unsigned int data2)
{
    do
    {
        isd_transmit(command,data,data2);
        isd_transmit(&rd_status,0,0);
    }   while (isd_read(0)&ISD_CMD_ERR);
}
//returns true if the isd is currently recording
int isd_is_recording()
{
    isd_transmit(&rd_status,0,0);
    if (isd_read(2)&ISD_REC)
        return 1;
    return 0;
}
//returns true if the isd is currently playing
int isd_is_playing()
{
    isd_transmit(&rd_status,0,0);
    if(isd_read(2)&ISD_PLAY)
        return 1;
    return 0;
}
//returns true if the isd found an eom
int isd_eom()
{
    isd_transmit(&rd_status,0,0);
    if(isd_read(0)&ISD_EOM)
        return 1;
    return 0;
}
//returns true if the int flag on the isd is raised
int isd_interrupt()
{
    isd_transmit(&rd_status,0,0);
    if(isd_read(0)&ISD_INT)
        return 1;
    return 0;
}
//set isd to record on an audio channel
void isd_set_rec(unsigned int audio_channel)
{
    if(audio_channel > 4 || audio_channel == 0)
        return;
    audio_channel--;
    //erase whatever's currently in memory at that location
    isd_transmit(&clr_int,0,0);
    isd_wait_ready();
    isd_transmit_validate(&set_erase,isd_ptr[audio_channel], isd_ptr[audio_channel]+isd_mem_msg-1);
    isd_wait_ready();
    //record new data
    isd_transmit(&clr_int,0,0);
    isd_wait_ready();
    isd_transmit_validate(&set_rec,isd_ptr[audio_channel], isd_ptr[audio_channel]+isd_mem_msg-1);
}
//set isd to play from an audio channel.
void isd_set_play(unsigned int audio_channel)
{
    if (audio_channel > 4 || audio_channel == 0)
        return;
    audio_channel--;
    isd_transmit(&clr_int,0,0);
    isd_wait_ready();
    isd_transmit_validate(&set_play,isd_ptr[audio_channel], isd_ptr[audio_channel]+isd_mem_msg-1);

}
//stop play or recording
void isd_stop()
{

    isd_transmit(&clr_int,0,0);
    isd_wait_ready();
    isd_transmit_validate(&stop,0,0);
}
//wrapper for interrupt clear
void isd_clear_interrupt()
{
    isd_transmit(&clr_int,0,0);
}
//send a command to the isd
void isd_transmit(const isd_cmd_t * command, unsigned int data, unsigned int data2)
{
    unsigned int i = 0x1000;
    while (UCB0IE & UCRXIE)
    {
        i--;
        if (!i)
            halt(isdStallError);
    }
    if (!command)
        return;

    for (i = 0; i < 7; i++)
    {
        isd_tx[i] = 0;
        isd_rx[i] = 0;
    }
    isd_tx[0] = command->cmd | ISD_LED_ENABLE;
    if (command->fields==1)
    {
        isd_tx[1] = (char)data;
        isd_tx[2] = data>>8;
    }

    else if (command->fields==2)
    {
        isd_tx[2] = (char)data;
        isd_tx[3] = data>>8;
        isd_tx[4] = (char)data2;
        isd_tx[5] = data2>>8;
    }
    isd_cmd_len = command->length;
    isd_rx_index = 0;
    isd_tx_index = 0;
    //set line low
    P1OUT &= ~GPIO_USCI_SS;
    //begin transmission
    UCB0TXBUF = isd_tx[isd_tx_index++];
    //enable interrupt
    UCB0IE |= UCTXIE | UCRXIE;
}
//read the current row address from last transmission
unsigned int isd_decode_current_row()
{
    isd_timeout = 0x1000;
    while (UCB0IE & UCRXIE)
    {
        isd_timeout--;
        if (!isd_timeout)
            halt(isdStallError);
    }
    unsigned int result = isd_rx[1]<<3;
    result |= (isd_rx[0]&ISD_A2_0)>>5;
    return result;
}
//wait until the current transmission is done then return data at index
unsigned char isd_read(unsigned int index)
{
    isd_timeout = 0x1000;
    while (UCB0IE & UCRXIE)
    {
        isd_timeout--;
        if (!isd_timeout)
            halt(isdStallError);
    }
    return isd_rx[index];
}
