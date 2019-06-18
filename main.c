#include <msp430.h>
#include <flash.h>
#include <pins.h>
#include <timer.h>
#include <stddef.h>
#include "queue.h"
#include "main.h"

int main(void)
{
  volatile unsigned int i;
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  for (i=0; i<20000; i++)                   // Delay for crystal stabilization
  {
  }
  __enable_interrupt(); // Enable Global Interrupts

  //initialize system clock
  init_clk();

  //make sure all global variables are initialized to known value
  init_globals();

  //initalize pins
  setup_pins();

  //setup timer
  timera_init();

  //initialize spi/isd interface
  init_isd();

  //initialize flash part
  init_flash();

  //load buttons from flash
  load_button_ids();

  //initialize queue system
  queue_create(&id_queue, id_queue_array, (sizeof id_queue_array)/(sizeof id_queue_array[0]));

  //start program
  run_program();
}

void run_program(void)
{
    volatile int prog_run = 1;
    unsigned int led_toggle = 50;

    //TODO: update to only enable timer when there are valid buttons programmed
    //check to see if there are valid buttons
    //if(button_id_list.num_of_valid_ids > 0)
    start_timera();
    start_timera1();
    do
	{
	    //handle user programming inputs
	    handle_user_inputs_alt();

	    //Handle receiver inputs
	    handle_receiver_inputs_alt();

	    //play pending queue messages
	    if(!program_button_active)
	        play_from_queue();

	    if((program_button_active == 1) || button_1_programmed > 0)
	    {
	        //TODO: use timer to toggle LED

	        if(led_toggle == 0)
	        {
	            toggle_led(GPIO_STATUS_LED);
	            led_toggle = 40;
	        }
	        else
	        {
	            led_toggle--;
	        }
	    }

	    if(button_1_programmed > 0)
	    {
	        //inline_delay(0x80);
	        timer_delay(100);
	        button_1_programmed--;
	        if(button_1_programmed == 0)
	        {
	            //keep LED on
	            turn_on_led(GPIO_STATUS_LED);
	        }
	    }
	    else
	    {
	        //inline_delay(0x500);
	        timer_delay(1000);
	    }

	    //TODO: add support for power savings
        //__bis_SR_register(LPM0_bits + GIE);       // LPM0 + Enable global ints
	}
	while(prog_run);
}

void init_clk()
{
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL  = CALDCO_1MHZ;
}



void init_globals(void)
{
    toggle_led_index = 0;
    p2_gpio_int_state = 0;
    program_mode_active = 0;
    program_button_active = 0;
    button_1_programmed = 0;
    program_button_active = 0;
    program_button_target = 0;
    flash_data_struct_t button_id_list = {0};
    struct Queue id_queue = {0};
    message_length = 0;
}

void play_audio(unsigned int audio_channel)
{
    isd_set_play(audio_channel);
}

unsigned int get_audio_channel(unsigned long button_id)
{
    if(button_id==BUTTON_ID_INVALID)
        return AUDIO_CHANNEL_NONE;
    unsigned int index = 0;
    for(index=0; index<AUDIO_CHANNEL_TOTAL; index++)
    {
        if (button_id==button_id_list.button_id[index])
            return AUDIO_CHANNEL(index+1);
    }
    return AUDIO_CHANNEL_NONE;
}

unsigned int get_num_active_chan(void)
{
    return button_id_list.num_of_valid_ids;
}

void load_button_ids(void)
{
    unsigned char data_read_good = 0;
    flash_data_struct_t temp_button_id = {0};

    //initialize button list
    temp_button_id.flash_struct_ver = button_id_list.flash_struct_ver = FLASH_DATA_VERSION;
    temp_button_id.num_of_valid_ids = button_id_list.num_of_valid_ids = 0;
    temp_button_id.button_id[0] = button_id_list.button_id[0] = BUTTON_ID_INVALID;
    temp_button_id.button_id[1] = button_id_list.button_id[1] = BUTTON_ID_INVALID;
    temp_button_id.button_id[2] = button_id_list.button_id[2] = BUTTON_ID_INVALID;
    temp_button_id.button_id[3] = button_id_list.button_id[3] = BUTTON_ID_INVALID;

    //read the data from flash to RAM
    if(flash_read((char *)&temp_button_id, FLASH_BLOCK_SIZE, FLASH_BLOCK_VERSION_OFFSET) == 1)
    {
        //check for valid version
        if(temp_button_id.flash_struct_ver == FLASH_DATA_VERSION)
        {
            //valid version found, check for active channels
            //if((temp_button_id.num_of_valid_ids >= 0) && (temp_button_id.num_of_valid_ids <= AUDIO_CHANNEL_TOTAL))
            if(temp_button_id.num_of_valid_ids <= AUDIO_CHANNEL_TOTAL)
            {
                //initialize button list
                button_id_list.flash_struct_ver = temp_button_id.flash_struct_ver;
                button_id_list.num_of_valid_ids = temp_button_id.num_of_valid_ids;
                button_id_list.button_id[0] = temp_button_id.button_id[0];
                button_id_list.button_id[1] = temp_button_id.button_id[1];
                button_id_list.button_id[2] = temp_button_id.button_id[2];
                button_id_list.button_id[3] = temp_button_id.button_id[3];

                //TODO: verify checksum for each valid button

                data_read_good = 1;
            }
        }
        else //uninitialized or old version
        {
            //TODO: add recovery for legacy versions in the future
        }
    }

    //write update if read was not successful
    if(data_read_good == 0)
    {
        //intialize to latest
        if(flash_write((char *)&button_id_list, FLASH_BLOCK_SIZE, FLASH_BLOCK_VERSION_OFFSET) == 0)
        {
            //TODO: add error state
            halt();
        }
    }
}

void write_button_ids(void)
{
    if(flash_write((char *)&button_id_list, FLASH_BLOCK_SIZE, FLASH_BLOCK_VERSION_OFFSET) == 0)
    {
        //TODO: add error state (flash status LED at a certain rate)
        halt();
    }
}

void erase_button_ids(void)
{
    //initialize button list
    button_id_list.flash_struct_ver = FLASH_DATA_VERSION;
    button_id_list.num_of_valid_ids = 0;
    button_id_list.button_id[0] = 0xFF000000;
    button_id_list.button_id[1] = 0xFF000000;
    button_id_list.button_id[2] = 0xFF000000;
    button_id_list.button_id[3] = 0xFF000000;

    if(flash_write((char *)&button_id_list, FLASH_BLOCK_SIZE, FLASH_BLOCK_VERSION_OFFSET) == 0)
    {
        //TODO: add error state (flash status LED at a certain rate)
        halt();
    }

}

int add_button_id(unsigned long new_button, unsigned int index)
{
    //TODO: add support for multiple buttons
    if (index < FLASH_BUTTON_ID_LENGTH)
    {
        button_id_list.button_id[index] = new_button;
        unsigned int i = 0;
        unsigned int count = 0;
        for ( ; i < FLASH_BUTTON_ID_LENGTH; i++)
        {
            if (button_id_list.button_id[i] != BUTTON_ID_INVALID)
                count++;
        }
        button_id_list.num_of_valid_ids = count;
        write_button_ids();
        return 1;
    }
    return 0;
}

void handle_receiver_inputs_alt(void)
{
    unsigned long data_received = 0;
    if(timer_state==flag && program_button_active == 0)
    {
        stop_timera();
        data_received = timer_rcv_transmission;
        add_to_queue(data_received);
        start_timera();
    }
    else if (timer_state == flag && program_button_active)
    {
        stop_timera();
        data_received = timer_rcv_transmission;
        if (data_received != BUTTON_ID_INVALID)
        {
            add_button_id(data_received, program_button_target);
            program_button_active = 0;
            button_1_programmed = 600;
        }
    }
}

void handle_user_inputs_alt(void)
{
    unsigned int p2_gpio_cur_state = P2IN;
    unsigned int p2_gpio_debounce_state = 0;
    p2_gpio_int_state = 0;

    unsigned int prog_button_pressed[4];
    unsigned int prog_button_released[4];
    unsigned int prog_button_debounce_count[4];


    int prog_button_timer_id = 0;
    unsigned int prog_button_delta = 0;
    unsigned int count = 0;
    unsigned int button_focus_press = 0;
    unsigned int is_recording = 0;

    static unsigned int button_counter = 0;
    static unsigned int button_timer_id = 0;
    static unsigned int button_focus = 0;


    for (count = 0; count< 4; count++)
    {
        prog_button_pressed[count] = 0;
        prog_button_released[count] = 0;
        prog_button_debounce_count[count] = 0;
    }

    //inline_delay(0x300);
    timer_delay(750);

    p2_gpio_debounce_state = p2_gpio_cur_state & P2IN;
    for (count = 0; count< 4; count++)
    {
        prog_button_pressed[count] = p2_gpio_debounce_state & GPIO_BUTTON(count);
    }

    if(prog_button_pressed[0] || prog_button_pressed[1] || prog_button_pressed[2] || prog_button_pressed[3])
    {
        prog_button_timer_id = timer_begin();
        for (count=0; count<4; count++)
        {
            prog_button_debounce_count[count] = 3;
        }

        while((prog_button_pressed[0] && (prog_button_debounce_count[0]>0)) ||
              (prog_button_pressed[1] && (prog_button_debounce_count[1]>0)) ||
              (prog_button_pressed[2] && (prog_button_debounce_count[2]>0)) ||
              (prog_button_pressed[3] && (prog_button_debounce_count[3]>0)))
        {
            //inline_delay(0x30);
            timer_delay(50);

            for (count=0; count<4; count++)
            {
                if(prog_button_pressed[count] && ((P2IN & GPIO_BUTTON(count)) == 0))
                {
                    prog_button_debounce_count[count]--;
                }
                if(prog_button_pressed[count])
                {
                    button_focus_press = count;
                }
            }

            //enable recording if button held for 2s
            if (timer_check(prog_button_timer_id) > 2000 && button_counter == 0 && !is_recording)
            {
                stop_timera();
                turn_on_led(GPIO_RF_ACTIVITY_LED);
                //set_gpio_p1_high(GPIO_AUDIO_REC1_ENABLE);
                //unsigned int audio_channel = AUDIO_CHANNEL(button_focus_press+1));
                is_recording = 1;
                isd_stop();
                isd_set_rec(AUDIO_CHANNEL(button_focus_press+1));
            }
        }//while

        prog_button_delta = timer_release(prog_button_timer_id);
        for (count=0; count<4; count++)
        {
            if(prog_button_debounce_count[count] == 0)
                {
                    //program button was pressed and released
                    prog_button_released[count] = 1;
                }
        }
    }//if(prog_button_pressed[0] || prog_button_pressed[1] || prog_button_pressed[2] || prog_button_pressed[3])

    for(count = 0; count<4; count++)
    {
        if (prog_button_released[count])
        {
            if ( button_focus != count || button_counter == 0)
            {
                button_counter = 1;
            }
            else
            {
                button_counter++;
            }
            button_focus = count;
            if (!button_timer_id)
                button_timer_id = timer_begin();
        }
    }
    //TODO
    if (is_recording)
    {
        //set_gpio_p1_low(GPIO_AUDIO_REC1_ENABLE);
        isd_stop();
        turn_off_led(GPIO_RF_ACTIVITY_LED);
        start_timera();
        button_counter = 0;
    }


    //not checking for timeout / timeout has not occurred at 1.5 sec yet
    if (button_counter == 0 || timer_check(button_timer_id)<1500)
    {
    }
    //timeout occurred
    else
    {
        timer_release(button_timer_id);
        button_timer_id = 0;

        if (button_counter == 1)
        {
            button_counter = 0;
            //play audio
            if (program_mode_active == 0)
            {
                //play_audio(audio_channel[button_focus]);
                add_to_queue(button_id_list.button_id[button_focus]);
            }
            //disable program mode
            else
            {
                button_counter = 0;
                turn_off_led(GPIO_STATUS_LED);
                program_mode_active = 0;
                program_button_active = 0;
                //reset_decoder();
                start_timera();
            }
        }
        //enter program mode and record button 1
        else if (button_counter == 3 && !program_mode_active)
        {
            button_counter = 0;
            //enter program mode
            //turn_on_p1_led(GPIO_STATUS_LED);
            program_mode_active = 1;

            //stop receiver
            stop_timera();
            //turn_off_p1_led(GPIO_RF_ACTIVITY_LED);

            //enable button record
            program_button_active = 1;
            program_button_target = button_focus;
            //reset_decoder();
            //start timer
            start_timera();
        }
        //erase all buttons by pressing button 0 5 times
        else if (button_counter == 5 && button_focus==0 && !program_mode_active && !program_button_active)
        {
            //turn_on_led(GPIO_RF_ACTIVITY_LED);
            turn_on_led(GPIO_STATUS_LED);
            erase_button_ids();
            //inline_delay(0x800);
            timer_delay(2000);
            //turn_off_p1_led(GPIO_RF_ACTIVITY_LED);
            turn_on_led(GPIO_STATUS_LED);
        }
        //any other number of presses detected with timeout
        else
        {
            //button_timer = 0;
            timer_release(button_timer_id);
            button_counter= 0;
        }
    }

    //exit program mode if in program mode and not programming button 1
    if( program_mode_active && !(program_button_active) && !(button_1_programmed) )
    {
        //turn_on_led(GPIO_STATUS_LED);
        //exit program mode
        //wait for release
        turn_off_led(GPIO_STATUS_LED);
        program_mode_active = 0;
        program_button_active = 0;

        //reset_decoder();
        //start timer
        start_timera();
    }
}


void add_to_queue(unsigned long button_id)
{
    if (queue_index_of(&id_queue, button_id)<0)
        queue_enqueue(&id_queue,button_id);
}

void play_from_queue()
{
    if (id_queue.length > 0 && !(isd_is_playing()))
    {
        unsigned int audio_channel = get_audio_channel(queue_get(&id_queue,0));
        if (audio_channel != AUDIO_CHANNEL_NONE)
        {
            play_audio(audio_channel);
        }
        else
        {
            queue_dequeue(&id_queue);
        }
    }
    else if (isd_eom())
    {
        isd_stop();
        queue_dequeue(&id_queue);
    }
}

void halt()
{
    turn_on_led(GPIO_ERROR_LED);
    volatile int i = 0;
    while(!i)
        ;
    turn_off_led(GPIO_ERROR_LED);
}

//interrupts
//CCR1
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TimerA01(void)
{
    unsigned int taiv = TAIV;
    switch (timer_state)
    {
    case syn:
    {
        timer_state = idle;
        //rising, CCIxA, sync, capture, interrupt
        TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TACCTL1 = CM_0 | CCIS_0 | SCS;
        TACTL = TASSEL_2 | ID_0 | MC_2;
        break;
    }
    case read:
    {
        if (timer_rcv_index < TIMER_RCV_BIT_LEN && (taiv && TA0IV_TACCR1))
        {
            timer_push(TACCTL0 & CCI);
            TACCR1 +=TIMER_PULSE;
        }

        else if (timer_rcv_index >= TIMER_RCV_BIT_LEN)
        {
            __enable_interrupt();
            if (timer_decode())
                timer_state = flag;
            else
            {
                timer_state = idle;
                //rising, CCIxA, sync, capture, interrupt
                TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
                TACCTL1 = CM_0 | CCIS_0 | SCS;
                TACTL = TASSEL_2 | ID_0 | MC_2;
            }
        }
        break;
    }
    case flag:
    {
        TACTL = MC_0 | TACLR;
        break;
    }
    default:
    {
        timer_state = idle;
        //rising, CCIxA, sync, capture, interrupt
        TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TACCTL1 = CM_0 | CCIS_0 | SCS;
        TACTL = TASSEL_2 | ID_0 | MC_2;
        halt();
    }
    }
}

//TACCR0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerA00(void)
{
    switch (timer_state)
    {
    case idle:
    {
        TACTL = TASSEL_2 | ID_0 | MC_1 | TAIE | TACLR;
        timer_state = syn;
        TACCR0 = TIMER_SYN_MAX_LEN;
        //rising edge, CCIxA, sync, capture, interrupt
        TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TACCTL1 = CM_0 | CCIS_0 | SCS;
        turn_off_led(GPIO_RF_ACTIVITY_LED);
        break;
    }
    case syn:
    {
        timer_rcv_rate  = TACCR0;
        if (timer_rcv_rate >= TIMER_SYN_MIN_LEN)
        {
            TACTL = TASSEL_2 | ID_0 | MC_2 | TACLR | TAIE;
            timer_rcv_rate = timer_rcv_rate >> 3;
            TACCR1 = TIMER_HALF_PULSE;
            //rising, CCIxA, sync, capture, interrupt
            TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
            //no capture, CCIxA, sync, compare, interrupt
            TACCTL1 = CM_3 | CCIS_0 | SCS |/*| CAP |*/ CCIE;
            timer_rcv_index = 0;
            timer_poll_count = 0;
            timer_state = read;
        }
        else
        {
            timer_state = idle;
            //rising, CCIxA, sync, capture, interrupt
            TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
            TACCTL1 = CM_0 | CCIS_0 | SCS;
            TACTL = TASSEL_2 | ID_0 | MC_2;
        }
        turn_off_led(GPIO_RF_ACTIVITY_LED);
        break;
    }
    case read:
    {
        timer_rcv_rate = TACCR0;
        TACCR0 = 0xffff;
        TACTL |= TACLR;
        TACCR1 = TIMER_HALF_PULSE;
        break;
    }
    case flag:
    {
        TACTL = MC_0 | TACLR;
        break;
    }
    case off:
    {
        break;
    }
    default:
    {
        timer_state = idle;
        TACTL = MC_0 | TACLR;
        //rising, CCIxA, sync, capture, interrupt
        TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TACCTL1 = CM_0 | CCIS_0 | SCS;
        TACTL = TASSEL_2 | ID_0 | MC_2 | TACLR;
        halt();
    }
    }
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void TIMERA11 (void)
{
    __enable_interrupt();
    unsigned int taiv = TA1IV;
    switch (taiv)
    {
    case TA1IV_TAIFG:
    {
        unsigned int i;
        if (FLAG(timer_enable,0))
        {
            if (timer_msec[0]==0)
            {
                TA1CCTL1 = CM_0 | CCIS_2 | CCIE;
                TA1CTL |= TACLR;
                TA1CTL = TASSEL_2 | ID_0 | MC_1 | TAIE;
            }
            if (timer_msec[0])
                timer_msec[0]--;
        }

        for (i = 1; i < TIMER_RESOURCE; i++)
        {
            if (FLAG(timer_enable,i))
                timer_msec[i]++;
        }
        break;
    }
    case TA1IV_TACCR1:
    {
        CLEAR_FLAG(timer_enable,0);
        TA1CCTL1 &= ~(CCIE);
        break;
    }
    default:
    {
        halt();
    }
    }
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void ISDtxRdy (void)
{
        UCB0TXBUF = isd_tx[isd_tx_index++];
        if (isd_tx_index >= isd_cmd_len)
        {
            IE2 &= ~UCB0TXIE;
            if (!(IE2 & UCB0RXIE))
                P1OUT |= GPIO_USCI_SS;
        }
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void ISDrxRdy (void)
{
    isd_rx[isd_rx_index++] = UCB0RXBUF;
    if (isd_rx_index >= isd_cmd_len)
    {
        IE2 &= ~UCB0RXIE;
        if (!(IE2 & UCB0TXIE))
            P1OUT |= GPIO_USCI_SS;
    }
}
// //Port 2 interrupt service routine
// #pragma vector = PORT2_VECTOR
// __interrupt void Port_2(void)
// {
//     //save initial state of pin
//     p2_gpio_int_state = P2IN;
//     P2IE &= ~(GPIO_PROGRAM_BUTTON);
//     P2IFG &= ~(GPIO_PROGRAM_BUTTON);
//     //LPM0_EXIT;
// }
