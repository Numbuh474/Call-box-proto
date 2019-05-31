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
  //initialize system clock
  init_clk();

  //make sure all global variables are initialized to known value
  init_globals();

  //initalize pins
  setup_pins();

  //setup timer
  timera_init();

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
    //done on invocation of start_timera();
    /*{
        //enable capture interrupt
        CCTL0 |= CCIE;
    }*/
    __enable_interrupt(); // Enable Global Interrupts
    start_timera();
    //add_to_queue(0);
    do
	{
	    //handle user programming inputs
	    handle_user_inputs_alt();

	    //Handle receiver inputs
	    handle_receiver_inputs_alt();

	    if(!program_button_active)
	        play_from_queue();

	    if((program_button_active == 1) || button_1_programmed > 0)
	    {
	        //TODO: user timer to toggle LED
	        if(led_toggle == 0)
	        {
	            toggle_p1_led(GPIO_STATUS_LED);
	            led_toggle = 50;
	        }
	        else
	        {
	            led_toggle--;
	        }
	    }

	    if(button_1_programmed > 0)
	    {
	        inline_delay(0x80);
	        button_1_programmed--;
	        if(button_1_programmed == 0)
	        {
	            //keep LED on
	            turn_on_p1_led(GPIO_STATUS_LED);
	        }
	    }
	    else
	    {
	        inline_delay(0x500);
	    }

	    //TODO: add support for power savings
        //__bis_SR_register(LPM0_bits + GIE);       // LPM0 + Enable global ints
	}
	while(prog_run);
}

void init_clk()
{
    //if (CALBC1_1MHZ != 0xFF)   //check for calibration constant
    //constant definition in msp430xxx.h, should fail to compile if
    //microprocessor does not support
    //{
        DCOCTL = 0;
        BCSCTL1 = CALBC1_1MHZ;
        DCOCTL  = CALDCO_1MHZ;
   // }
}

void setup_pins(void)
{
     // set P1 GPIOs
    P1SEL &= ~(GPIO_AUDIO_CHAN1_ENABLE | GPIO_STATUS_LED | GPIO_RF_ACTIVITY_LED | GPIO_AUDIO_REC1_ENABLE);
    // intialize output pins
    P1OUT &= ~(GPIO_AUDIO_CHAN1_ENABLE | GPIO_STATUS_LED | GPIO_RF_ACTIVITY_LED | GPIO_AUDIO_REC1_ENABLE);
    // set up output pins
    P1DIR |= GPIO_AUDIO_CHAN1_ENABLE | GPIO_STATUS_LED | GPIO_RF_ACTIVITY_LED | GPIO_AUDIO_REC1_ENABLE;
    // intialize output pins
    P1OUT &= ~(GPIO_AUDIO_CHAN1_ENABLE | GPIO_STATUS_LED | GPIO_RF_ACTIVITY_LED | GPIO_AUDIO_REC1_ENABLE);
    //set P1 input pins
    P1DIR &= ~(GPIO_RF_INPUT);
    //set GPIO_RF_INPUT to TA0
    P1SEL = GPIO_RF_INPUT;

    // set P2 GPIOs
    P2SEL &= ~(GPIO_PROGRAM_BUTTON | GPIO_PROGRAM_BUTTON_1 | GPIO_ERASE_BUTTONS | GPIO_CHAN1_REC_BUTTON);
    // set up output pins
    //P2DIR |= ;
    // intialize output pins
    //P2OUT &= ~();
    //set input pins
    P2DIR &= ~(GPIO_PROGRAM_BUTTON | GPIO_PROGRAM_BUTTON_1 | GPIO_ERASE_BUTTONS | GPIO_CHAN1_REC_BUTTON);
    //set pull downs
    P2REN &= ~(GPIO_PROGRAM_BUTTON | GPIO_PROGRAM_BUTTON_1 | GPIO_ERASE_BUTTONS | GPIO_CHAN1_REC_BUTTON);
    //setup interrupt
    //P2IES &= ~(GPIO_PROGRAM_BUTTON); // rising Edge 0 -> 1
    //P2IE |= (GPIO_PROGRAM_BUTTON);
    //P2IFG &= ~(GPIO_PROGRAM_BUTTON);

    //disable port 3 for now
    P3DIR = 0xFF;
    P3OUT &= 0x00;

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

}
void play_audio(unsigned int audio_channel)
{
    unsigned int audio_gpio = 0xFF;

    if(audio_channel == AUDIO_CHANNEL_1)
    {
        audio_gpio = GPIO_AUDIO_CHAN1_ENABLE;
    }

    if(audio_gpio != 0xFF)
    {
        set_gpio_p1_high(audio_gpio);

        inline_delay(0x300);

        set_gpio_p1_low(audio_gpio);
    }
}

void inline_delay(unsigned int delay_cycle)
{
    volatile unsigned int i;

    // delay for a while
    for (i = delay_cycle; i > 0; i--);
}

unsigned int get_audio_channel(unsigned long button_id)
{
    unsigned int ret_channel = AUDIO_CHANNEL_NONE;
    unsigned int index = 0;

    for(index=0; index<AUDIO_CHANNEL_TOTAL; index++)
    {
        //TODO: refactory button_id_list to be an array
        if((index == 0) && (button_id == button_id_list.button_id[0]))
        {
            ret_channel = AUDIO_CHANNEL_1;
        }
        else if((index == 1) && (button_id == button_id_list.button_id[1]))
        {
            ret_channel = AUDIO_CHANNEL_2;
        }
        else if((index == 2) && (button_id == button_id_list.button_id[2]))
        {
            ret_channel = AUDIO_CHANNEL_3;
        }
        else if((index == 3) && (button_id == button_id_list.button_id[3]))
        {
            ret_channel = AUDIO_CHANNEL_4;
        }
    }
    return ret_channel;
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
    temp_button_id.button_id[0] = button_id_list.button_id[0] = 0xFF000000;
    temp_button_id.button_id[1] = button_id_list.button_id[1] = 0xFF000000;
    temp_button_id.button_id[2] = button_id_list.button_id[2] = 0xFF000000;
    temp_button_id.button_id[3] = button_id_list.button_id[3] = 0xFF000000;

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
            //TODO: add error state (flash status LED at a certain rate)
        }
    }
}

void write_button_ids(void)
{
    if(flash_write((char *)&button_id_list, FLASH_BLOCK_SIZE, FLASH_BLOCK_VERSION_OFFSET) == 0)
    {
        //TODO: add error state (flash status LED at a certain rate)
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
    unsigned int prog_button_delta [4];
    unsigned int count = 0;

    static unsigned int button_counter = 0;
    static unsigned int button_timer = 0;
    static unsigned int button_focus = 0;

    for (count = 0; count< 4; count++)
    {
        prog_button_pressed[count] = 0;
        prog_button_released[count] = 0;
        prog_button_debounce_count[count] = 0;
        prog_button_delta[count] = 0;
    }

    inline_delay(0x300);

    p2_gpio_debounce_state = p2_gpio_cur_state & P2IN;

    prog_button_pressed[0] = p2_gpio_debounce_state & GPIO_PROGRAM_BUTTON;
    prog_button_pressed[1] = p2_gpio_debounce_state & GPIO_PROGRAM_BUTTON_1;
    prog_button_pressed[2] = p2_gpio_debounce_state & GPIO_ERASE_BUTTONS;
    prog_button_pressed[3] = p2_gpio_debounce_state & GPIO_CHAN1_REC_BUTTON;

    if(prog_button_pressed[0] || prog_button_pressed[1] || prog_button_pressed[2] || prog_button_pressed[3])
    {
        for (count=0; count<4; count++)
        {
            prog_button_debounce_count[count] = 3;
        }

        while((prog_button_pressed[0] && (prog_button_debounce_count[0]>0)) ||
              (prog_button_pressed[1] && (prog_button_debounce_count[1]>0)) ||
              (prog_button_pressed[2] && (prog_button_debounce_count[2]>0)) ||
              (prog_button_pressed[3] && (prog_button_debounce_count[3]>0)))
        {
            inline_delay(0x30);

            if(prog_button_pressed[0] && ((P2IN & GPIO_PROGRAM_BUTTON) == 0))
            {
                prog_button_debounce_count[0]--;
            }
            if(prog_button_pressed[1] && ((P2IN & GPIO_PROGRAM_BUTTON_1) == 0))
            {
                prog_button_debounce_count[1]--;
            }
            if(prog_button_pressed[2] && ((P2IN & GPIO_ERASE_BUTTONS) == 0))
            {
                prog_button_debounce_count[2]--;
            }
            if(prog_button_pressed[3] && ((P2IN & GPIO_CHAN1_REC_BUTTON) == 0))
            {
                prog_button_debounce_count[3]--;
            }
            for (count=0; count<4; count++)
            {
                if(prog_button_pressed[count] && prog_button_delta[count] != ~(unsigned int)0)
                {
                    prog_button_delta[count]++;
                    //enable recording if button 0 held for 5000mut
                    if (prog_button_delta[0] > 5000 && button_counter == 0)
                    {
                        turn_on_p1_led(GPIO_RF_ACTIVITY_LED);
                        set_gpio_p1_high(GPIO_AUDIO_REC1_ENABLE);
                    }
                }
            }
        }
        for (count=0; count<4; count++)
        {
            if(prog_button_debounce_count[count] == 0)
                {
                    //program button was pressed and released
                    prog_button_released[count] = 1;
                }
        }

    }
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
            button_timer = 0;
            //disable recording if button 0 pressed once for 5000mut
            if (prog_button_delta[count] > 5000 )//&& button_counter == 1)
            {
                set_gpio_p1_low(GPIO_AUDIO_REC1_ENABLE);
                turn_off_p1_led(GPIO_RF_ACTIVITY_LED);
                button_counter = 0;
                button_timer = 0;
            }
        }
    }

    //not checking for timeout
    if (button_counter == 0)
    {
        //do nothing here
    }
    //checking for timeout at 50mut
    else if (button_timer <= 50)
    {
        //no timeout
        button_timer++;
    }
    //timeout occurred
    else //if (button_timer > 50)
    {
        button_timer = 0;

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
                turn_off_p1_led(GPIO_STATUS_LED);
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
            turn_on_p1_led(GPIO_STATUS_LED);
            program_mode_active = 1;

            //stop receiver
            stop_timera();
            turn_off_p1_led(GPIO_RF_ACTIVITY_LED);

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
            //turn_on_p1_led(GPIO_RF_ACTIVITY_LED);
            turn_on_p1_led(GPIO_STATUS_LED);
            erase_button_ids();
            inline_delay(0x800);
            //turn_off_p1_led(GPIO_RF_ACTIVITY_LED);
            turn_on_p1_led(GPIO_STATUS_LED);
        }
        //any other number of presses detected with timeout
        else
        {
            button_timer = 0;
            button_counter= 0;
        }
    }

    //exit program mode if in program mode and not programming button 1
    if( program_mode_active && !(program_button_active) && !(button_1_programmed) )
    {
        turn_on_p1_led(GPIO_STATUS_LED);
        //for(count = 20; count > 0; count--)
        //    inline_delay(0x4000);
        //exit program mode
        //wait for release
        turn_off_p1_led(GPIO_STATUS_LED);
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
    //TODO:
    static unsigned long timer = 0;

    if (id_queue.length > 0 && timer==0)
    {
        unsigned int audio_channel = get_audio_channel(queue_get(&id_queue,0));
        if (audio_channel != AUDIO_CHANNEL_NONE)
        {
            play_audio(audio_channel);
            timer = 150;
        }
        else
            queue_dequeue(&id_queue);
    }
    else if(timer > 0)
    {
        timer--;
        if (timer==0)
        {
            turn_on_p1_led(GPIO_STATUS_LED);
            inline_delay(0x600);
            turn_off_p1_led(GPIO_STATUS_LED);
            queue_dequeue(&id_queue);
        }
    }
}



//interrupts
//CCR1
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TimerA1(void)
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
        else if (timer_decode() && timer_rcv_index >= TIMER_RCV_BIT_LEN)
            timer_state = flag;
        else
        {
            timer_state = idle;
            //rising, CCIxA, sync, capture, interrupt
            TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
            TACCTL1 = CM_0 | CCIS_0 | SCS;
            TACTL = TASSEL_2 | ID_0 | MC_2;
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
        break;
    }
    }
}

//TACCR0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerA0(void)
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
        turn_off_p1_led(GPIO_RF_ACTIVITY_LED);
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
        turn_off_p1_led(GPIO_RF_ACTIVITY_LED);
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

    default:
    {
        timer_state = idle;
        TACTL = MC_0 | TACLR;
        //rising, CCIxA, sync, capture, interrupt
        TACCTL0 = CM_1 | CCIS_0 | SCS | CAP | CCIE;
        TACCTL1 = CM_0 | CCIS_0 | SCS;
        TACTL = TASSEL_2 | ID_0 | MC_2 | TACLR;
    }
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
