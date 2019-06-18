#ifndef ISD_H_
#define ISD_H_

#include "main.h"
#define ISD_SE_TOTAL    4
#define ISD_SE(x)       4*x
#define ISD_MEM_BEGIN   0x10
#define ISD_MEM_1730    0xff
#define ISD_MEM_1740    0x14f
#define ISD_MEM_1750    0x19f
#define ISD_MEM_1760    0x1ef
#define ISD_MEM_1790    0x2df
#define ISD_MEM_17120   0x3cf
#define ISD_MEM_17150   0x4bf
#define ISD_MEM_17180   0x5af
#define ISD_MEM_17210   0x69f
#define ISD_MEM_17240   0x78f

#define ISD_CMD_ERR     BIT0
#define ISD_FULL        BIT1
#define ISD_PU          BIT2
#define ISD_EOM         BIT3
#define ISD_INT         BIT4
#define ISD_A2_0        0b11100000
#define ISD_REC         BIT3
#define ISD_PLAY        BIT2
#define ISD_ERASE       BIT1
#define ISD_RDY         BIT0
#define ISD_LED_EN      BIT4

typedef struct
{
    unsigned char cmd;//the SPI command to send
    unsigned char length;//the length in bytes of the entire command
    unsigned char fields;//the number of variables transmitted
}isd_cmd_t;

//TODO: isd_rx may need to be changed to prevent ovf.
unsigned char isd_rx [7];
unsigned char isd_tx [7];
unsigned int isd_ptr[4];

unsigned int isd_mem_max;
unsigned int isd_mem_msg;
volatile unsigned int isd_rx_index;
volatile unsigned int isd_tx_index;
volatile unsigned char isd_cmd_len;


void isd_transmit(const isd_cmd_t* command, unsigned int data, unsigned int data2);
void isd_transmit_validate(const isd_cmd_t* command, unsigned int data, unsigned int data2);
void isd_wait_ready();
void isd_set_play(unsigned int audio_channel);
void isd_set_rec(unsigned int audio_channel);
int isd_is_playing();
int isd_is_recording();
unsigned char isd_read(unsigned int index);
void isd_stop();
unsigned int isd_decode_current_row();
void init_isd();



#endif /* ISD_H_ */
