/*********************************************************************
*
*       Modtronix Example Web Server Application using Microchip TCP/IP Stack
*
*********************************************************************
* FileName:        WebSrvr.c
* Dependencies:    string.H
*                  usart.h
*                  stacktsk.h
*                  Tick.h
*                  http.h
*                  MPFS.h
* Processor:       PIC18
* Complier:        MCC18 v1.00.50 or higher
*                  HITECH PICC-18 V8.35PL3 or higher
* Company:         Microchip Technology, Inc.
*
* Software License Agreement
*
* The software supplied herewith by Microchip Technology Incorporated
* (the Company) for its PICmicro® Microcontroller is intended and
* supplied to you, the Company’s customer, for use solely and
* exclusively on Microchip PICmicro Microcontroller products. The
* software is owned by the Company and/or its supplier, and is
* protected under applicable copyright laws. All rights are reserved.
* Any use in violation of the foregoing restrictions may subject the
* user to criminal sanctions under applicable laws, as well as to
* civil liability for the breach of the terms and conditions of this
* license.
*
* THIS SOFTWARE IS PROVIDED IN AN AS IS CONDITION. NO WARRANTIES,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*
*
* HiTech PICC18 Compiler Options excluding device selection:
*                  -FAKELOCAL -G -O -Zg -E -C
*
**********************************************************************
* File History
*
* 2004--2-15, David Hosken (DH):
*  - Original (Rev. 1.0)
* 2009-05-29, David Hosken (DH):
*  - Added code for watchdog timer, and alive counters
*  - Fixed bug initializing CPU ports. This bug could have caused very short
*    micro-second spikes on IO ports at start up. Sequence of calling appcfgCpuIO()
*    and appcfgCpuIOValues() functions were fixed.
*********************************************************************/
 
/*
 * Following define uniquely deines this file as main
 * entry/application In whole project, there should only be one such
 * definition and application file must define AppConfig variable as
 * described below.
 */
//#define THIS_IS_STACK_APPLICATION

//#include <string.h>
#include <p18cxxx.h>

#define Yellow_blinking 	1 
#define RED 		   	2 
#define YELLOW_RED     	3 
#define GREEN		   	4 
#define Yellow		   	5 
#define SCH_MAX_TASKS 5

float count = 0;
int state = 1;
int val ;
static int millisP;
static int buttonP;
static int temperature;

void ResetTimer0();

void Init_timer0();

void SCH_Dispatcher_Tasks();
void SCH_add_task(void (*)(void), unsigned int, unsigned int);
void SCH_Delete_Task(int);
void setup_scheduler(void);
static void scheduler_update(void);

 void Read_T (void);
void LED_result(void);
void check_buttons(void);
static void setup_board(void);

/* Utilities */
static void start_50ms_timer(void);
static void enter_sleep_state(void);
static unsigned int millis_passed;

typedef struct 
{
	void (* pTask)(void);
	int delay;
	unsigned int Repeat;
	unsigned int RunMe;
		
    }sTask;
	
static sTask SCH_Tasks_G[SCH_MAX_TASKS]/*= {
           {NULL,0,0,0},
           {NULL,0,0,0}
         }*/;

void ResetTimer0()
	{	// 26 473
		TMR0H = 0x67;   //order is important, high must be written first
		TMR0L = 0x69;   //Low Byte
		INTCON &= 0xFB; // reset the interrupt flag bit
	}
	void enter_sleep_state() {
	OSCCONbits.IDLEN = 1;
	_asm
		sleep
	_endasm
}
	
 void setup_scheduler(void) {
  unsigned short int i;
  
  for (i = 0; i < SCH_MAX_TASKS; ++i) {
    SCH_Delete_Task(i);
  }
}
static void start_50ms_timer(void) {
  /*
    Preset timer values
	  for 50 milliseconds 
	*/
	TMR0H = 0x0B;
	TMR0L = 0xDC;

	/* Enable timer interrup */
	INTCONbits.TMR0IE = 1;

	/* Clear overflow */
	INTCONbits.TMR0IF = 0;

  /* Start the timer */
	T0CON = 0x82;
}

/////////////////////////////////////////////////
//High Interrupt ISR
    #pragma interrupt scheduler_update save = section(".tmpdata") 
    static void scheduler_update()
{
	unsigned int index;
	if (INTCONbits.TMR0IF) 
	{
		millis_passed += 50;
		
		//ResetTimer0();
		for (index = 0; index < SCH_MAX_TASKS; ++index)
		{
			if (SCH_Tasks_G[index].pTask != 0) 
			{
				SCH_Tasks_G[index].delay -= 50;// reduce the delay by 50 s

				if (SCH_Tasks_G[index].delay <= 0) 
				{
					++SCH_Tasks_G[index].RunMe;
					if (SCH_Tasks_G[index].Repeat != 0) 
					{
					SCH_Tasks_G[index].delay = SCH_Tasks_G[index].Repeat;
					}
				}
			}
        }

		start_50ms_timer();
    }
	/////////////////////////////////////////////
    //USART1
   /* if(PIR1_RCIF && PIE1_RCIE)  //USART1 Receive
    {
        serRxIsr();
    }
    //USART1 Transmit
    if(PIR1_TXIF && PIE1_TXIE)
    {
        serTxIsr();
    }
    //USART2
    if(PIR3_RC2IF && PIE3_RC2IE)  //USART1 Receive
    {
        ser2RxIsr();
    }
    //USART2 Transmit
    if(PIR3_TX2IF && PIE3_TX2IE)
    {
        ser2TxIsr();
    }*/
}

#pragma code high_vector=0x808
void high_vector() {
	_asm
		goto scheduler_update
	_endasm
}
#pragma code


/*
 * Main entry point.
 */
/*int ReadTimer0()
    {
        int tempTimerValue;
        int timerValue = 0x00;
        timerValue = TMR0L;                       //must read TMR0L first to load TMR0H
        tempTimerValue = TMR0H;
        timerValue |= tempTimerValue << 8; //bit shift the high value and OR together
        return timerValue;
    }    
*/	  

static void setup_board() {
	/* Disable watchdog */
	WDTCONbits.SWDTEN = 0;

	/* Set LED ports to output */
	TRISCbits.TRISC1 = 0;

	/* Set button ports to input */
	TRISBbits.TRISB0 = 1;
	TRISBbits.TRISB1 = 1;

  /* Initialize analog ports */
	ADCON0 = 0x4;
	ADCON1 = 0x0;
	ADCON2 = 0x7;

	/* Set initial timer and state */
	millis_passed = 0;
	buttonP = 0;
}		
void Init_timer0()
{

	int i =0;
	for (i=0;i< SCH_MAX_TASKS ;i++)
	{
		SCH_Delete_Task(i);
	}
	
	// initialising the timer and start the timer	
	T0CON |= 0x86;// prescaler 128 and start the timer
	// initialisation of the interrupt
	INTCON |= 0xA0;
	//ResetTimer0();
	
}

void SCH_Dispatcher_Tasks()
{
	unsigned int index;
	for(index =0; index <  SCH_MAX_TASKS;index++ )
	{
		if (SCH_Tasks_G[index].RunMe >0)
		{
			(*SCH_Tasks_G[index].pTask)();
			
			//SCH_Tasks_G[index].RunMe -= 1;
			--SCH_Tasks_G[index].RunMe;
			
			if(SCH_Tasks_G[index].Repeat ==0)
			SCH_Delete_Task(index);
		}
	}
	enter_sleep_state();
}

void SCH_Delete_Task(int index)
{
	if (index >= SCH_MAX_TASKS) 
	{
		return;
	}
  SCH_Tasks_G[index].pTask = 0;
  SCH_Tasks_G[index].delay = 0;
  SCH_Tasks_G[index].delay = 0;
  SCH_Tasks_G[index].RunMe = 0;
}
void Read_T()
{
	//read the temperaturte here
  PIR1bits.ADIF = 0;
  PIE1bits.ADIE = 0;

  /* Start conversion */
  ADCON0 = 0x7;

  // Conversion in progress
  while (ADCON0 == 0x7);
  // Read the data 
  temperature = (ADRESH << 4) + ADRESL;
}
void check_buttons() {
	if (PORTBbits.RB0 || PORTBbits.RB1) {
		buttonP = 0;
	}
	else {
		buttonP = 1;
	}
}
void LED_result()
{
	//make some changes to the 3 LEDs
	unsigned short int index;

  if (buttonP != 1) {
    LATCbits.LATC1 ^= 1;
  }
  else {
    LATCbits.LATC1 = 0;
  }

  for(index =0; index <  SCH_MAX_TASKS;index++ ) {
    if ((*SCH_Tasks_G[index].pTask) == &LED_result) {
      SCH_Tasks_G[index].delay = temperature;
    }
  }
  
 
  
}
void SCH_add_task(void (*pFunction)(), unsigned int delay, unsigned int repeat)
{
	//make some changes to the 3 LEDs
	unsigned short int index = 0;
	while ((SCH_Tasks_G[index].pTask !=0) && (index < SCH_MAX_TASKS))
	{
		index ++;
	}
	if(index == SCH_MAX_TASKS ) return;
	
	SCH_Tasks_G[index].pTask = pFunction;
	SCH_Tasks_G[index].delay = delay + 1;
	SCH_Tasks_G[index].Repeat = repeat;
	SCH_Tasks_G[index].RunMe = 0;
	
}

void main(void)
{
	setup_board();
	setup_scheduler();
	
	// adding task to the scheduler
	
	SCH_add_task(&Read_T, 0, 100);
	SCH_add_task(&check_buttons, 50, 500);
	SCH_add_task(&LED_result, 100, 50);
	start_50ms_timer();
	//supper loop
	while(1)
	{	
		SCH_Dispatcher_Tasks();
	}
}
 	
/*static void ProcessIO(void)
{
    static TICK8 tmr10ms = 0;
    
    //Enter each 10ms
    if ( TickGetDiff8bit(tmr10ms) >= ((TICK8)TICKS_PER_SECOND / (TICK8)100) )
    {
        tmr10ms = TickGet8bit();

        //Service Expansion Board, if any is present
        switch(appcfgGetc(APPCFG_XBRD_TYPE)) {
        case XBRD_TYPE_MXD2R:
            mxd2rService();
            break;
        case XBRD_TYPE_IOR5E:
            ior5eService();
            break;
        }
    }
    

    //Convert next ADC channel, and store result in adcChannel array
    #if (defined(APP_USE_ADC8) || defined(APP_USE_ADC10)) && (ADC_CHANNELS > 0)
    {
        static BYTE adcChannel; //Current ADC channel. Value from 0 - n
        static BOOL ADC_Wait = 0;

        //Increment ADCChannel and start new convertion
        if (!ADC_Wait)
        {
            //Increment to next ADC channel
            if ((++adcChannel) >= ADC_CHANNELS)
            {
                adcChannel = 0;
            }
            
            //Check if current ADC channel (adcChannel) is configured to be ADC channel
            if (adcChannel < ((~ADCON1) & 0x0F))
            {
                //Convert next ADC Channel
                ADCON0 &= ~0x3C;
                ADCON0 |= (adcChannel << 2);
                ADCON0_ADON = 1;    //Switch on ADC
                ADCON0_GO = 1;      //Go
                ADC_Wait  = 1;
            }
            //Not ADC channel, set to 0
            else
            {
                AdcValues[adcChannel] = 0;
            }
        }
        
        //End of ADC Convertion: save data
        if ((ADC_Wait)&&(!ADCON0_GO))
        {
            #if defined(APP_USE_ADC8)
            AdcValues[adcChannel] = ADRESH;
            #elif defined(APP_USE_ADC10)
            //AdcValues[adcChannel] = (WORD) ((((WORD)ADRESH) << 8) | ((WORD)ADRESL));
            //AdcValues[adcChannel] = (WORD)((ADRESH*256)+ADRESL);
            AdcValues[adcChannel] = ((WORD)ADRESH << 8) | (WORD)ADRESL;
            #endif
            ADC_Wait = 0;
        }
    }
    #endif
}*/

//static ROM char HTTPHDR_AUTHORIZATION[] = "AUTHORIZATION: BASIC ";

/**
 * This function is a "callback" from HTTPServer task. For each HTTP Header found in the HTTP Request
 * message from the client, this function is called. The application has the chance to parse the
 * received headers.
 *
 * @param httpInfo  HTTP_INFO structure of current HTTP connection
 * @param hdr       Buffer containing NULL terminated header to handle
 * @param rqstRes   Name of the Requested resource - GET command's action. All characters are in uppercase!
 */
 /*
void HTTPProcessHdr(HTTP_INFO* httpInfo, BYTE* hdr, BYTE* rqstRes) {
    BYTE i;
    char unpw[20];      //Buffer for storing username and password, seperated by ':'
    char base64[25];    //Buffer for storing base64 result

    //Check if buffer begins with ROM string, ignore case
    if (strBeginsWithIC((char*)hdr, HTTPHDR_AUTHORIZATION)) {
        i = strcpyee2ram(unpw, APPCFG_USERNAME0, 8);    //Returns number of bytes written, excluding terminating null
        unpw[i++] = ':';                                //Overwrite terminating null with ':'
        strcpyee2ram(&unpw[i], APPCFG_PASSWORD0, 8);
        
        base64Encode((char*)base64, (char*)unpw, strlen(unpw));

        if (strcmp( (char*)(&hdr[sizeof(HTTPHDR_AUTHORIZATION)-1]), base64) == 0) {
            httpInfo->flags.bits.bUserLoggedIn = TRUE;

            #if (DEBUG_MAIN >= LOG_DEBUG)
                debugPutMsg(6); //@mxd:6:HTTP User Authenticated
            #endif
        }
    }
}
*/

/////////////////////////////////////////////////
//Implement callback for FTPVerify() function
/*#if defined(STACK_USE_FTP_SERVER)
    #if (FTP_USER_NAME_LEN > APPCFG_USERNAME_LEN)
    #error "The FTP Username length can not be shorter then the APPCFG Username length!"
    #endif
*/
/*BOOL FTPVerify(char *login, char *password)
{
    #if (DEBUG_MAIN >= LOG_INFO)
        debugPutMsg(4); //@mxd:4:Received FTP Login (%s) and Password (%s)
        debugPutString(login);
        debugPutString(password);
    #endif

    if (strcmpee2ram(login, APPCFG_USERNAME0) == 0) {
        if (strcmpee2ram(password, APPCFG_PASSWORD0) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}
#endif
*/
/**
 * Initialize the boards hardware
 */
/*static void InitializeBoard(void)
{
    #if (defined(MCHP_C18) && (defined(__18F458) || defined(__18F448) || defined(__18F6680))) \
        || (defined(HITECH_C18) && (defined(_18F458) || defined(_18F448) || defined(_18F6680)))
        CMCON  = 0x07; /* Comparators off CM2:CM0 = 111 for PIC 18F448, 18F458, 18F6680 
    #endif


    /////////////////////////////////////////////////
    //Enable 4 x PLL if configuration bits are set to do so
    #if ( defined(MCHP_C18) && defined(__18F6621))
    OSCCON = 0x04;              //Enable PLL (PLLEN = 1)
    while (OSCCON_LOCK == 0);   //Wait until PLL is stable (LOCK = 1)
    
    //Seeing that this code does currently not work with Hi-Tech compiler, disable this feature
    OSCCON_SCS1 = 1;
    //Switch on software 4 x PLL if flag is set in configuration data
    //if (appcfgGetc(APPCFG_SYSFLAGS) & APPCFG_SYSFLAGS_PLLON) {
    //    OSCCON_SCS1 = 1;
    //}
    #endif
    

    /////////////////////////////////////////////////
    //Setup timer1 as a free running 16 bit timer.
    //TMR1L is incremented every 800ns. TMR1H is incremented every 204.8us
    //Is used to measure events in code.
    //TMR1H.bit0 is toggled every 204.8us
    //TMR1H.bit1 is toggled every 409.6us
    //TMR1H.bit2 is toggled every 819.2us
    //TMR1H.bit3 is toggled every 1.6384ms
    //TMR1H.bit4 is toggled every 3.2768ms
    //TMR1H.bit5 is toggled every 6.5536ms
    //TMR1H.bit6 is toggled every 13.1072ms
    //TMR1H.bit7 is toggled every 26.2144ms
    //1xxx xxxx = Enable read/write as a 16bit times. TMR1H is a buffer that is loaded when TMR1L is accessed.
    //xx11 xxxx = No prescaler
    //xxxx 0xxx = Timer1 oscillator disabled (RC0 and RC1)
    //xxxx xx0x = Internal clock source = Fosc/4 = 40/4 = 10MHz for a 40MHz clock
    //xxxx xxx1 = Timer 1 on
    T1CON = 0xB1;

    T1CON_RD16 = 0; //Enable reading TMR1L and TMR1H individually

    //When reading TMR1 in code, execute following command if TMR1L and TMR1H byte has to be read as a single value
    //T1CON_RD16 = 1; //Enable reading TMR1L and TMR1H as 16-bit value
    //If TMR1H has to be read on it's own (without first reading TMR1L), execute following command.
    //T1CON_RD16 = 0; //Enable reading TMR1L and TMR1H individually

    //Disable external pull-ups
    INTCON2_RBPU = 1;

    //Enable T0 interrupt
    T0CON = 0;
}*/
