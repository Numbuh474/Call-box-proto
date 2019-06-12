unsigned char isd_rx [7];

typedef struct
{
    unsigned char cmd;
    unsigned char length;
    unsigned char fields;
}isd_cmd_t;

static const isd_cmd_t pu =         {0x01,2,0};
static const isd_cmd_t stop =       {0x02,2,0};
static const isd_cmd_t reset =      {0x03,2,0};
static const isd_cmd_t clr_int =    {0x04,2,0};
static const isd_cmd_t rd_status =  {0x05,3,0};
static const isd_cmd_t rd_play_ptr ={0x06,4,0};
static const isd_cmd_t pd =         {0x07,2,0};
static const isd_cmd_t rd_rec_ptr = {0x08,4,0};
static const isd_cmd_t devid =      {0x09,3,0};
static const isd_cmd_t play =       {0x40,2,0};
static const isd_cmd_t rec =        {0x41,2,0};
static const isd_cmd_t erase =      {0x42,2,0};
static const isd_cmd_t g_erase =    {0x43,2,0};
static const isd_cmd_t rd_apc =     {0x44,4,0};
static const isd_cmd_t wr_apc1 =    {0x45,3,1};
static const isd_cmd_t wr_apc2 =    {0x65,3,1};
static const isd_cmd_t wr_nvcfg =   {0x46,2,0};
static const isd_cmd_t ld_nvcfg =   {0x47,2,0};
static const isd_cmd_t fwd =        {0x48,2,0};
static const isd_cmd_t chk_mem =    {0x49,2,0};
static const isd_cmd_t extclk =     {0x4a,2,0};
static const isd_cmd_t set_play =   {0x80,7,2};
static const isd_cmd_t set_rec =    {0x81,7,2};
static const isd_cmd_t set_erase =  {0x82,7,2};
//lowest byte first, little endian

int transmit(isd_cmd_t* command, unsigned int data, unsigned int addr)
{
    if (!command)
        return 0;
    unsigned int i,j;
    //set SS low
    for (i=0; i<command->length; i++)
    {
        for (j=0; j<8; j++)
        {
            //MO on rising edge SO on falling edge
            //set data, set clock pin high>>>
            //set clock pin low, save data<<<
        }
    }
    //set ss high
}
