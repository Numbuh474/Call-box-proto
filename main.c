#include <msp430.h>
#include <flash.h>
#include <pins.h>
#include <timer.h>

#define MINIMUM_DECODE_DELTA 52
#define DECODE_ARRAY_SIZE 160

//Audio Channel Defines
#define AUDIO_CHANNEL_NONE 0
#define AUDIO_CHANNEL_1 1
#define AUDIO_CHANNEL_2 2
#define AUDIO_CHANNEL_3 3
#define AUDIO_CHANNEL_4 4

#define AUDIO_CHANNEL_TOTAL 4

#define BUTTON_ID_INVALID 0xFF000000

//GPIO definitions
#define GPIO_RF_ACTIVITY_LED     BIT0  //P1.0
#define GPIO_RF_INPUT            BIT1  //P1.1
#define GPIO_AUDIO_REC1_ENABLE   BIT4  //P1.4
#define GPIO_STATUS_LED          BIT6  //P1.6
#define GPIO_AUDIO_CHAN1_ENABLE  BIT7  //P1.7
#define GPIO_PROGRAM_BUTTON      BIT0  //P2.0
#define GPIO_PROGRAM_BUTTON_1    BIT1  //P2.1
#define GPIO_ERASE_BUTTONS       BIT2  //P2.2
#define GPIO_CHAN1_REC_BUTTON    BIT4  //P2.4

//flash structure
typedef struct
{
    unsigned int flash_struct_ver;
    unsigned int num_of_valid_ids;
    unsigned long button_id_1;
    unsigned long button_id_2;
    unsigned long button_id_3;
    unsigned long button_id_4;
} flash_data_struct_t;

volatile unsigned int new_cap=0;
volatile unsigned int old_cap=0;
volatile unsigned int cap_diff=0;

unsigned int decode_array[DECODE_ARRAY_SIZE] = {0};
unsigned int decode_array_head = 0;
unsigned int decode_last_timestamp = 0;
unsigned int decode_data_available = 0;
unsigned int toggle_led_index = 0;
unsigned int p2_gpio_int_state = 0;
unsigned int program_mode_active = 0;
unsigned int program_button_1_active = 0;
unsigned int button_1_programmed = 0;

flash_data_struct_t button_id_list = {0}; 

void setup_pins(void);
unsigned int add_decode_transition(unsigned int new_timestamp);
unsigned long get_call_button_id(void);
unsigned int get_next_decoded_bit(unsigned int decode_index);
void reset_decoder(void);
unsigned long call_button_received(void);
void run_program(void);
void play_audio(unsigned int audio_channel);
void inline_delay(unsigned int delay_cycle);
unsigned int get_audio_channel(unsigned long button_id);
unsigned int get_num_active_chan(void);
void load_button_ids(void);
void write_button_ids(void);
void erase_button_ids(void);
void add_button_id(unsigned long new_button);
void handle_receiver_inputs(void);
void handle_user_inputs(void);
void init_globals(void);
unsigned long get_multiple_call_button_ids(unsigned int num_of_ids);


int main(void)
{
  volatile unsigned int i;
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  for (i=0; i<20000; i++)                   // Delay for crystal stabilization
  {
  }
  
  //make sure all global variables are initialized to known value
  init_globals();
  
  //initalize pins
  setup_pins();

  //setup timer
  timer_a_init();
  
  //initialize flash part
  init_flash();
  
  //load buttons from flash
  load_button_ids();

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
    {
        //enable capture interrupt
        CCTL0 |= CCIE;
    }
    __enable_interrupt(); // Enable Global Interrupts

    do
	{
	    //handle user programming inputs
	    handle_user_inputs();

	    //Handle receiver inputs
	    handle_receiver_inputs();
	    
	    
	    if((program_button_1_active == 1) || button_1_programmed > 0)
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
    new_cap=0;
    old_cap=0;
    cap_diff=0;
    
    decode_array_head = 0;
    decode_last_timestamp = 0;
    decode_data_available = 0;
    toggle_led_index = 0;
    p2_gpio_int_state = 0;
    program_mode_active = 0;
    program_button_1_active = 0;
    button_1_programmed = 0;
}

unsigned long call_button_received(void)
{
    unsigned long data_received = 0;
    
    if( decode_data_available > 0)
    {
        data_received = get_call_button_id();
        decode_data_available = 0;
        reset_decoder();
    }
    return data_received;
}

unsigned int add_decode_transition(unsigned int new_timestamp)
{
    unsigned int stop_timer = 0;
    if((new_timestamp >= 275) && (new_timestamp <= 10500))
    {
        if(decode_array_head >= DECODE_ARRAY_SIZE)
        {
            decode_array_head = 0;
        }
        else
        {
            decode_array[decode_array_head] = new_timestamp;
            decode_array_head++;
        }
        if(decode_array_head == MINIMUM_DECODE_DELTA*3)
        {
            decode_data_available = 1;
            CCTL0 &= ~(CCIE);
            stop_timer = 1;
        }
    }
    return stop_timer;
    
}

unsigned long get_call_button_id(void)
{
    unsigned int current_head = decode_array_head;
    unsigned int i = 0;
    unsigned long button_id = 0;
    unsigned int button_bits_added = 0;
    
    unsigned int ret_bit = 0;
    unsigned int start_bit_found = 0;
    
    if(current_head >= 48)
    {
        //get button id
        while((i < current_head) && (i < 199))
        {
            ret_bit = get_next_decoded_bit(i);
            if((start_bit_found == 0) && (ret_bit == 0x02)) //found start bit
            {
                start_bit_found = 1;
                i++;
            }
            else if((start_bit_found == 1) && (ret_bit == 0x02)) //new start bit, reset number
            {
                button_id = 0;
                button_bits_added = 0;
                i++;
            }
            else if((button_bits_added < 24) && (ret_bit != 0xFF)) //a valid bit found
            {
                button_id = button_id << 1;
                button_id |= (0x0001 & ret_bit);
                button_bits_added++;
                i += 2;
            }
            else
            {
                i++;
            }
            if(button_bits_added == 24)
            {
                //found all bits, break
                break;
            }
        }
    }
    return button_id;
}

unsigned long get_multiple_call_button_ids(unsigned int num_of_ids)
{
    unsigned int current_head = decode_array_head;
    unsigned int i = 0;
    unsigned long button_id = 0;
    unsigned long button_id_1 = 0;
    unsigned long button_id_2 = 0;
    unsigned long button_id_3 = 0;
    unsigned int button_bits_added = 0;
    unsigned int button_count = 0;
    
    unsigned int ret_bit = 0;
    unsigned int start_bit_found = 0;
    
    if(current_head >= (num_of_ids*MINIMUM_DECODE_DELTA))
    {
        while(button_count < num_of_ids)
        {
            //get button id
            while((i < current_head) && (i < DECODE_ARRAY_SIZE-1))
            {
                ret_bit = get_next_decoded_bit(i);
                if((start_bit_found == 0) && (ret_bit == 0x02)) //found start bit
                {
                    start_bit_found = 1;
                    i++;
                }
                else if((start_bit_found == 1) && (ret_bit == 0x02)) //new start bit, reset number
                {
                    button_id = 0;
                    button_bits_added = 0;
                    i++;
                }
                else if((button_bits_added < 24) && (ret_bit != 0xFF)) //a valid bit found
                {
                    button_id = button_id << 1;
                    button_id |= (0x0001 & ret_bit);
                    button_bits_added++;
                    i += 2;
                }
                else
                {
                    i++;
                }
                if(button_bits_added == 24)
                {
                    //found all bits, break
                    break;
                }
            }
            
            if(button_count == 0)
            {
                button_id_1 = button_id;
            }
            else if(button_count == 1)
            {
                button_id_2 = button_id;
            }
            else if(button_count == 2)
            {
                button_id_3 = button_id;
            }
        
            button_count++;
        }
        if(!((button_id_1 == button_id_2) && ((button_id_1 == button_id_3))))
        {
            button_id = 0;
        }
    }
    return button_id;
}

void reset_decoder(void)
{
    unsigned int i;
    
    for(i=0; i<DECODE_ARRAY_SIZE; i++)
    {
        decode_array[i] = 0;
    }
    decode_array_head = 0;
    decode_last_timestamp = 0;
    decode_data_available = 0;
    
}

unsigned int get_next_decoded_bit(unsigned int decode_index)
{
    unsigned int ret_bit = 0xFF;
    if((decode_array[decode_index] >= 9900) && (decode_array[decode_index] <= 10150))
    {
        ret_bit = 0x2;
    }
    else if((decode_array[decode_index] >= 275) && (decode_array[decode_index] <= 400))
    {
        if((decode_array[decode_index+1] >= 900) && (decode_array[decode_index+1] <= 1100))
        {
            ret_bit = 0;
        }
    }
    else if((decode_array[decode_index] >= 900) && (decode_array[decode_index] <= 1100))
    {
        if((decode_array[decode_index+1] >= 275) && (decode_array[decode_index+1] <= 400))
        {
            ret_bit = 1;
        }
    }
    return ret_bit;
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
        if((index == 0) && (button_id == button_id_list.button_id_1))
        {
            ret_channel = AUDIO_CHANNEL_1;
        }
        else if((index == 1) && (button_id == button_id_list.button_id_2))
        {
            ret_channel = AUDIO_CHANNEL_2;
        }
        else if((index == 2) && (button_id == button_id_list.button_id_3))
        {
            ret_channel = AUDIO_CHANNEL_3;
        }
        else if((index == 3) && (button_id == button_id_list.button_id_4))
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
    temp_button_id.button_id_1 = button_id_list.button_id_1 = 0xFF000000;
    temp_button_id.button_id_2 = button_id_list.button_id_2 = 0xFF000000;
    temp_button_id.button_id_3 = button_id_list.button_id_3 = 0xFF000000;
    temp_button_id.button_id_4 = button_id_list.button_id_4 = 0xFF000000;
    
    //read the data from flash to RAM
    if(flash_read((char *)&temp_button_id, FLASH_BLOCK_SIZE, FLASH_BLOCK_VERSION_OFFSET) == 1)
    {
        //check for valid version
        if(temp_button_id.flash_struct_ver == FLASH_DATA_VERSION)
        {
            //valid version found, check for active channels
            if((temp_button_id.num_of_valid_ids >= 0) && (temp_button_id.num_of_valid_ids <= AUDIO_CHANNEL_TOTAL))
            {
                //initialize button list
                button_id_list.flash_struct_ver = temp_button_id.flash_struct_ver;
                button_id_list.num_of_valid_ids = temp_button_id.num_of_valid_ids;
                button_id_list.button_id_1 = temp_button_id.button_id_1;
                button_id_list.button_id_2 = temp_button_id.button_id_2;
                button_id_list.button_id_3 = temp_button_id.button_id_3;
                button_id_list.button_id_4 = temp_button_id.button_id_4;

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
    button_id_list.button_id_1 = 0xFF000000;
    button_id_list.button_id_2 = 0xFF000000;
    button_id_list.button_id_3 = 0xFF000000;
    button_id_list.button_id_4 = 0xFF000000;

    if(flash_write((char *)&button_id_list, FLASH_BLOCK_SIZE, FLASH_BLOCK_VERSION_OFFSET) == 0)
    {
        //TODO: add error state (flash status LED at a certain rate)
    }

}

void add_button_id(unsigned long new_button)
{
    //TODO: add support for multipe buttons
    button_id_list.button_id_1 = new_button;
    button_id_list.num_of_valid_ids = 1;
    write_button_ids();
}

void handle_receiver_inputs(void)
{
    unsigned long data_received = 0;
    unsigned int audio_channel = AUDIO_CHANNEL_NONE;

    if( (decode_data_available > 0) && (program_button_1_active == 0))
    {
        stop_timera();
        data_received = call_button_received();
        
        if( data_received > 0)
        {
            turn_on_p1_led(GPIO_RF_ACTIVITY_LED);
            audio_channel = get_audio_channel(data_received);
            play_audio(audio_channel);
        }
        start_timera();
    }
    else if((decode_data_available > 0) && (program_button_1_active == 1))
    {
        stop_timera();
        data_received = get_multiple_call_button_ids(3);
        reset_decoder();
        if(data_received > 0)
        {
            add_button_id(data_received);
            program_button_1_active = 0;
            button_1_programmed = 600;
        }
        else
        {
            start_timera();
        }
    }
    turn_off_p1_led(GPIO_RF_ACTIVITY_LED);
}

void handle_user_inputs(void)
{
    unsigned int p2_gpio_cur_state = P2IN;
    unsigned int p2_gpio_debounce_state = 0;
    unsigned int prog_button_pressed = 0;
    unsigned int prog_button_released = 0;
    unsigned int prog_button_1_pressed = 0;
    unsigned int prog_button_1_released = 0;
    unsigned int prog_button_debounce_count = 0;
    unsigned int prog_button_1_debounce_count = 0;
    unsigned int prog_buttons_erase_pressed = 0;
    unsigned int prog_buttons_erase_released = 0;
    unsigned int prog_buttons_erase_debounce_count = 0;
    unsigned int prog_buttons_1_rec_pressed = 0;
    unsigned int prog_buttons_1_rec_debounce_count = 0;
    p2_gpio_int_state = 0;
    //TODO: move debounce to timer and check for button release
    //debounce
    inline_delay(0x300);
    
    p2_gpio_debounce_state = p2_gpio_cur_state & P2IN;
    
    prog_button_pressed = p2_gpio_debounce_state & GPIO_PROGRAM_BUTTON;
    prog_button_1_pressed = p2_gpio_debounce_state & GPIO_PROGRAM_BUTTON_1;
    prog_buttons_erase_pressed = p2_gpio_debounce_state & GPIO_ERASE_BUTTONS;
    prog_buttons_1_rec_pressed = p2_gpio_debounce_state & GPIO_CHAN1_REC_BUTTON;
    
    if(prog_button_pressed || prog_button_1_pressed || prog_buttons_erase_pressed || prog_buttons_1_rec_pressed)
    {
        prog_button_debounce_count = 3;
        prog_button_1_debounce_count = 3;
        prog_buttons_erase_debounce_count = 3;
        prog_buttons_1_rec_debounce_count = 3;
        
        if((program_mode_active == 1) && (prog_buttons_1_rec_pressed > 0))
        {
            turn_on_p1_led(GPIO_RF_ACTIVITY_LED);
            set_gpio_p1_high(GPIO_AUDIO_REC1_ENABLE);
        }
        //wait for the release
        while((prog_button_pressed && (prog_button_debounce_count > 0))   || 
              (prog_button_1_pressed && (prog_button_1_debounce_count>0)) ||
              (prog_buttons_erase_pressed && (prog_buttons_erase_debounce_count>0)) ||
              (prog_buttons_1_rec_pressed && (prog_buttons_1_rec_debounce_count>0)))
        {
            inline_delay(0x30);
            if(prog_button_pressed && ((P2IN & GPIO_PROGRAM_BUTTON) == 0))
            {
                prog_button_debounce_count--;
            }
            if(prog_button_1_pressed && ((P2IN & GPIO_PROGRAM_BUTTON_1) == 0))
            {
                prog_button_1_debounce_count--;
            }
            if(prog_buttons_erase_pressed && ((P2IN & GPIO_ERASE_BUTTONS) == 0))
            {
                prog_buttons_erase_debounce_count--;
            }
            if(prog_buttons_1_rec_pressed && ((P2IN & GPIO_CHAN1_REC_BUTTON) == 0))
            {
                prog_buttons_1_rec_debounce_count--;
            }
        }
        if(prog_button_debounce_count == 0)
        {
            //program button was pressed and released
            prog_button_released = 1;
        }
        if(prog_button_1_debounce_count == 0)
        {
            //program button 1 was pressed and released
            prog_button_1_released = 1;
        }
        if(prog_buttons_erase_debounce_count == 0)
        {
            //erase button was pressed and released
            prog_buttons_erase_released = 1;
        }
        if((program_mode_active == 1) && prog_buttons_erase_debounce_count)
        {
            set_gpio_p1_low(GPIO_AUDIO_REC1_ENABLE);
            turn_off_p1_led(GPIO_RF_ACTIVITY_LED);
        }
    }

    //check to see if program button is pressed and not in program mode
    if( (program_mode_active == 0) && prog_button_released)
    {
        turn_on_p1_led(GPIO_STATUS_LED);
        program_mode_active = 1;
        
        //stop receiver
        stop_timera();
        turn_off_p1_led(GPIO_RF_ACTIVITY_LED);
    }
    else if( (program_mode_active == 1) && prog_button_released)
    {
        //exit program mode
        //wait for release
        turn_off_p1_led(GPIO_STATUS_LED);
        program_mode_active = 0;
        program_button_1_active = 0;
        
        reset_decoder();
        //start timer
        start_timera();
    }
    else if((program_button_1_active == 0) && (program_mode_active == 1) && prog_button_1_released)
    {
        program_button_1_active = 1;
        reset_decoder();
        //start timer
        start_timera();
    }
    else if((program_button_1_active == 0) && (program_mode_active == 1) && prog_buttons_erase_released)
    {
        turn_on_p1_led(GPIO_RF_ACTIVITY_LED);
        erase_button_ids();
        inline_delay(0x800);
        turn_off_p1_led(GPIO_RF_ACTIVITY_LED);
    }
    
    //TODO: add interrupt support for buttons
    //clear interrupts
    //P2IE |= (GPIO_PROGRAM_BUTTON);
    //P2IFG &= ~(GPIO_PROGRAM_BUTTON);
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerA0(void)
{
    new_cap = TACCR0;
    if(new_cap >= old_cap)
    {
        cap_diff = new_cap - old_cap;
    }
    else
    {
        cap_diff = (0xFFFF-old_cap) + new_cap;
    }
    
    if(((program_mode_active == 0) || (program_button_1_active == 1))  && (add_decode_transition(cap_diff) == 1))
    {
        //disable interrupt
        CCTL0 &= ~(CCIE);
        //TODO: enable interrupt operation
        //LPM0_EXIT;
    }
    
    if (toggle_led_index == MINIMUM_DECODE_DELTA)
    {
        toggle_led_index = 0;
        P1OUT ^= GPIO_RF_ACTIVITY_LED;  // Toggle P1.0 using exclusive-OR
        __no_operation();
    }
    toggle_led_index++;
    old_cap = new_cap;                       // store this capture value
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TimerA1(void)
{
    
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

