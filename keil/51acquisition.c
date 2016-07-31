#include <intrins.h>
#include <reg51.h>
#include <string.h> 

/* AT89C51 ������Ϣ�� 
 * 1��״̬������ʱ�����ڵ���������ʱ�����ھ�����Ƶ�õ�
 * 2���������ڰ���6��״̬����S1~S6����һ�����������ڣ�CPU���һ����������
      11.0592M����Ļ�������Ϊ1.0851us
      12M����Ļ���������1us, 6M����Ļ���������2us,
 * 3) AT89C51��ָ����� ������ָ� ˫����ָ���������ָ��
 *    _NOP_(); �ǵ�����ָ��,�������ھ�ȷ��ʱ
 *    C�����������ٲ�������ָ�� LJMP(��������), RET(��������)
 */


#define FMT_VER         (0x2)
#define CS_CONTROL_INC  (0x40)  /* CS_CONTROL starting from P1.6, so the increasement is 0x40 */
#define SYNC_CHAR       (0x99)
#define SYNC_LONG       (0xFFFFFFFF)
#define ADC_NUM         (6)
#define CHL_NUM         (8)
#define ROW_NUM         (48)
#define COL_NUM         (48)    
#define SMP_BIT         (16)    

#define SEND_CHAR(c)    { SBUF = c; while (TI == 0) TI = 0;}

#define SEND_SHORT(s)   { SBUF = ( (unsigned short)(s) & 0xFF00 ) >> 8; while (TI == 0) TI = 0; \
                          SBUF = ( (unsigned short)(s) & 0x00FF ) >> 0; while (TI == 0) TI = 0;}    /* MSB first */

#define SEND_LONG(l)    { SBUF = ( (unsigned int)(l) & 0xFF000000 ) >> 24; while (TI == 0) TI = 0; \
                          SBUF = ( (unsigned int)(l) & 0x00FF0000 ) >> 16; while (TI == 0) TI = 0; \
                          SBUF = ( (unsigned int)(l) & 0x0000FF00 ) >>  8; while (TI == 0) TI = 0; \
                          SBUF = ( (unsigned int)(l) & 0x000000FF ) >>  0; while (TI == 0) TI = 0;}  /* MSB first */

#define DELAY(n)        { int i; for (i = 0; i < n; i++); }

                              // p3^0 p3^1  txd rxd P3�ڿ��� 
sbit AD7606_CONVSTA = P3^2;   // ��CONVSTABֱ������һ��    
sbit AD7606_CONVSTB = P3^3;   // sbit AD7606_CONVSTA = P2^1;  
sbit AD7606_REST    = P3^4;
sbit AD7606_RD      = P3^5;
sbit AD7606_BUSY    = P3^6;

sbit CS_CONTROL_A   = P1^6;
sbit CS_CONTROL_B   = P1^7;
sbit CS_CONTROL_C   = P3^7;   // ��ԭ��2�����õ�FRST VIO�˿� �޸�Ϊ  CS0 CS1 CS2  ����3-8������������6��ģ���CS��  

sbit SN_A = P1^0;             // P1.0-P1.5 6���˿�������ɨ���·��48������
sbit SN_B = P1^1;
sbit SN_C = P1^2;
sbit SN_D = P1^3;
sbit SN_E = P1^4;
sbit SN_F = P1^5;


//����ͨ�ų�ʼ�趨
void init_SPI(unsigned int baudrate)
{
    switch (baudrate)
    {
     case 9600:
        SCON = 0x50; // UARTΪģʽ1��8λ���ݣ��������
        TMOD = 0x20; // ��ʱ��1Ϊģʽ2,8λ�Զ���װ
        PCON = 0x00; // SMOD=1; SMOD�����⹦�ܼĴ���PCON�ĵ�7λ,���Կ��Ʋ����ʵģ�SMOD=1ʱ����ͨѶ�Ĳ����ʱ�����Ϊ0ʱ�����ʲ�������
        TL1  = 0xFD;
        TH1  = 0xFD;  // Baud:19200 fosc="11".0592MHz
        IE   = 0x90;  // Enable Serial Interrupt 
        TR1  = 1;     // Timer 1 run     
        TI   = 1; 
        break;
    case 19600:
        break;
    default:
        break;
    }
    return;
}

void init_ADC(void) 
{        
    AD7606_RD      = 1;
    CS_CONTROL_C   = 0;
    P1             = 0x00;  // AD7606_CS0=1;  000 3-8������ѡ�е�һ����LED����
    AD7606_CONVSTA = 1;    
    AD7606_CONVSTB = 1;                         

    DELAY(10000);

    AD7606_REST    = 1;
    AD7606_REST    = 0;
}

static unsigned char _data[COL_NUM*SMP_BIT/8];
/*
   the column addressing method:
   using P1.6 P1.7 P3.7 to select 6 AD7606, each AD7606 read 8 channels sequentially.
   so 6 * 8 samples are acquired.
                         __________
    CS_CONTROL_A   P1.6 -|A       |
    CS_CONTROL_B   P1.7 -|B       |
    CS_CONTROL_C   P3.7 -|C     Y0|- 
                        -|      Y1|-CS1
                        -|      Y2|-CS2
                        -|      Y3|-CS3
                        -|      Y4|-
                        -|      Y5|-CS4
                        -|      Y6|-CS5
                        -|      Y7|-CS6
                         |________|

   the following timing is cited from AD7606.pdf, fig. 4.
            __    ____
       CSx  CS        \____________ ______________________________

            __    __________      __      __       __
 AD7606_RD  RD              \____/  \____/  \_____/  \_____ ......

                     ________  ______  ______  ______  ____ ......
            DATA  __/INVALIDr\/  V1  \/  V2  \/  V3  \/
                    \________/\______/\______/\_____ /\____ ......

  give a low level on AD7606_RD, the AD7606 output the value in channels consequently. 
 */

void read_data(void)
{
     unsigned int i = 0;
     unsigned int n = 0;

     AD7606_CONVSTA = 0;
     AD7606_CONVSTA = 1;

     AD7606_CONVSTB = 0;
     AD7606_CONVSTB = 1;
      
     while (AD7606_BUSY) 
     {
         n = 0;

         CS_CONTROL_A = 0;
         CS_CONTROL_B = 0;
         CS_CONTROL_C = 0;

         P1 += CS_CONTROL_INC;
         for( i = 0; i < CHL_NUM; i++ ) AD7606_RD = 0; _data[n++] = P2; _data[n++] = P0; AD7606_RD = 1;
         P1 += CS_CONTROL_INC; 
         for( i = 0; i < CHL_NUM; i++ ) AD7606_RD = 0; _data[n++] = P2; _data[n++] = P0; AD7606_RD = 1;
         P1 += CS_CONTROL_INC;
         for( i = 0; i < CHL_NUM; i++ ) AD7606_RD = 0; _data[n++] = P2; _data[n++] = P0; AD7606_RD = 1;

         CS_CONTROL_A = 0;
         CS_CONTROL_B = 0;
         CS_CONTROL_C = 1;

         P1 += CS_CONTROL_INC;
         for( i = 0; i < CHL_NUM; i++ ) AD7606_RD = 0; _data[n++] = P2; _data[n++] = P0; AD7606_RD = 1;
         P1 += CS_CONTROL_INC;
         for( i = 0; i < CHL_NUM; i++ ) AD7606_RD = 0; _data[n++] = P2; _data[n++] = P0; AD7606_RD = 1;
         P1 += CS_CONTROL_INC;
         for( i = 0; i < CHL_NUM; i++ ) AD7606_RD = 0; _data[n++] = P2; _data[n++] = P0; AD7606_RD = 1;
    } 
    return;
}

void send_data(unsigned int bidx)      
{
    unsigned int  n = 0; 
    for( n = 0; n < ADC_NUM * CHL_NUM; n += 2 )
    {    
         if ( bidx )
         {
             SEND_CHAR(n);
         }
         SEND_CHAR(_data[n + 0 + ADC_NUM * CHL_NUM]);
         SEND_CHAR(_data[n + 1 + ADC_NUM * CHL_NUM]);

         SEND_CHAR(_data[n + 0]);
         SEND_CHAR(_data[n + 1]);
    }
    return;
}

void acquire_image(void)
{
#if 0
    P1=0x00; DELAY(0); read_data(); SEND_CHAR( 0); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x20; DELAY(0); read_data(); SEND_CHAR( 1); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x01; DELAY(0); read_data(); SEND_CHAR( 2); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x21; DELAY(0); read_data(); SEND_CHAR( 3); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x02; DELAY(0); read_data(); SEND_CHAR( 4); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);  //  ����Ҫ��Ծ
    P1=0x22; DELAY(0); read_data(); SEND_CHAR( 5); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x03; DELAY(0); read_data(); SEND_CHAR( 6); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x23; DELAY(0); read_data(); SEND_CHAR( 7); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x04; DELAY(0); read_data(); SEND_CHAR( 8); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x24; DELAY(0); read_data(); SEND_CHAR( 9); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x05; DELAY(0); read_data(); SEND_CHAR(10); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x25; DELAY(0); read_data(); SEND_CHAR(11); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x06; DELAY(0); read_data(); SEND_CHAR(12); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x26; DELAY(0); read_data(); SEND_CHAR(13); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x07; DELAY(0); read_data(); SEND_CHAR(14); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x27; DELAY(0); read_data(); SEND_CHAR(15); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);

    P1=0x10; DELAY(0); read_data(); SEND_CHAR(16); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x30; DELAY(0); read_data(); SEND_CHAR(17); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x11; DELAY(0); read_data(); SEND_CHAR(18); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x31; DELAY(0); read_data(); SEND_CHAR(19); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x12; DELAY(0); read_data(); SEND_CHAR(20); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);  //  ����Ҫ��Ծ
    P1=0x32; DELAY(0); read_data(); SEND_CHAR(21); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x13; DELAY(0); read_data(); SEND_CHAR(22); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x33; DELAY(0); read_data(); SEND_CHAR(23); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x14; DELAY(0); read_data(); SEND_CHAR(24); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x34; DELAY(0); read_data(); SEND_CHAR(25); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x15; DELAY(0); read_data(); SEND_CHAR(26); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x35; DELAY(0); read_data(); SEND_CHAR(27); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x16; DELAY(0); read_data(); SEND_CHAR(28); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x36; DELAY(0); read_data(); SEND_CHAR(29); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x17; DELAY(0); read_data(); SEND_CHAR(30); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x37; DELAY(0); read_data(); SEND_CHAR(31); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);

    P1=0x18; DELAY(0); read_data(); SEND_CHAR(32); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x38; DELAY(0); read_data(); SEND_CHAR(33); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x19; DELAY(0); read_data(); SEND_CHAR(34); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x39; DELAY(0); read_data(); SEND_CHAR(35); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x1A; DELAY(0); read_data(); SEND_CHAR(36); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);  //  ����Ҫ��Ծ
    P1=0x3A; DELAY(0); read_data(); SEND_CHAR(37); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x1B; DELAY(0); read_data(); SEND_CHAR(38); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x3B; DELAY(0); read_data(); SEND_CHAR(39); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x1C; DELAY(0); read_data(); SEND_CHAR(40); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x3C; DELAY(0); read_data(); SEND_CHAR(41); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x1D; DELAY(0); read_data(); SEND_CHAR(42); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x3D; DELAY(0); read_data(); SEND_CHAR(43); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x1E; DELAY(0); read_data(); SEND_CHAR(44); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x3E; DELAY(0); read_data(); SEND_CHAR(45); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x1F; DELAY(0); read_data(); SEND_CHAR(46); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);
    P1=0x3F; DELAY(0); read_data(); SEND_CHAR(47); SEND_CHAR(SYNC_CHAR); send_data(1); SEND_CHAR(SYNC_CHAR); SEND_CHAR(SYNC_CHAR);

#else

/* frame format:
   -----------------------------------------------------------------------------------------------------
   sync(4bytes) | version(2bytes) | xdim(2bytes) | ydim(2bytes) | nBytesOfValue(2bytes) | cval(2bytes)
     0xFFFFFFFF      e.g. 0x0001         e.g. 48        e.g. 47                   e.g. 2          e.g...      
   -----------------------------------------------------------------------------------------------------
 */
    SEND_LONG(SYNC_LONG);
    SEND_SHORT(FMT_VER);
    SEND_SHORT(ROW_NUM);
    SEND_SHORT(COL_NUM);
    SEND_SHORT(SMP_BIT/8);

    P1=0x00; DELAY(0); read_data(); send_data(0); 
    P1=0x20; DELAY(0); read_data(); send_data(0);
    P1=0x01; DELAY(0); read_data(); send_data(0);
    P1=0x21; DELAY(0); read_data(); send_data(0);
    P1=0x02; DELAY(0); read_data(); send_data(0);  //  ����Ҫ��Ծ
    P1=0x22; DELAY(0); read_data(); send_data(0);
    P1=0x03; DELAY(0); read_data(); send_data(0);
    P1=0x23; DELAY(0); read_data(); send_data(0);
    P1=0x04; DELAY(0); read_data(); send_data(0);
    P1=0x24; DELAY(0); read_data(); send_data(0);
    P1=0x05; DELAY(0); read_data(); send_data(0);
    P1=0x25; DELAY(0); read_data(); send_data(0);
    P1=0x06; DELAY(0); read_data(); send_data(0);
    P1=0x26; DELAY(0); read_data(); send_data(0);
    P1=0x07; DELAY(0); read_data(); send_data(0);
    P1=0x27; DELAY(0); read_data(); send_data(0);

    P1=0x10; DELAY(0); read_data(); send_data(0);
    P1=0x30; DELAY(0); read_data(); send_data(0);
    P1=0x11; DELAY(0); read_data(); send_data(0);
    P1=0x31; DELAY(0); read_data(); send_data(0);
    P1=0x12; DELAY(0); read_data(); send_data(0);  //  ����Ҫ��Ծ
    P1=0x32; DELAY(0); read_data(); send_data(0);
    P1=0x13; DELAY(0); read_data(); send_data(0);
    P1=0x33; DELAY(0); read_data(); send_data(0);
    P1=0x14; DELAY(0); read_data(); send_data(0);
    P1=0x34; DELAY(0); read_data(); send_data(0);
    P1=0x15; DELAY(0); read_data(); send_data(0);
    P1=0x35; DELAY(0); read_data(); send_data(0);
    P1=0x16; DELAY(0); read_data(); send_data(0);
    P1=0x36; DELAY(0); read_data(); send_data(0);
    P1=0x17; DELAY(0); read_data(); send_data(0);
    P1=0x37; DELAY(0); read_data(); send_data(0);

    P1=0x18; DELAY(0); read_data(); send_data(0);
    P1=0x38; DELAY(0); read_data(); send_data(0);
    P1=0x19; DELAY(0); read_data(); send_data(0);
    P1=0x39; DELAY(0); read_data(); send_data(0);
    P1=0x1A; DELAY(0); read_data(); send_data(0);  //  ����Ҫ��Ծ
    P1=0x3A; DELAY(0); read_data(); send_data(0);
    P1=0x1B; DELAY(0); read_data(); send_data(0);
    P1=0x3B; DELAY(0); read_data(); send_data(0);
    P1=0x1C; DELAY(0); read_data(); send_data(0);
    P1=0x3C; DELAY(0); read_data(); send_data(0);
    P1=0x1D; DELAY(0); read_data(); send_data(0);
    P1=0x3D; DELAY(0); read_data(); send_data(0);
    P1=0x1E; DELAY(0); read_data(); send_data(0);
    P1=0x3E; DELAY(0); read_data(); send_data(0);
    P1=0x1F; DELAY(0); read_data(); send_data(0);
    P1=0x3F; DELAY(0); read_data(); send_data(0);

#endif

    return;
}
    
void main(void) 
{
    init_SPI(9600);

    while(1)
    {     
        init_ADC();
        acquire_image();
    }

}

end;
