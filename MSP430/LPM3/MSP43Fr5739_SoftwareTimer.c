//------------------------------------------------------------------------------------
// Project name: Software timer for MSP430FR5739
// Author: Trieu Le
// Date: 26-Aug-2016
//------------------------------------------------------------------------------------
#include <msp430fr5739.h>
#include <stdint.h>
#include <string.h>
//------------------------------------------------------------------------------------
#define MAX_TIMER_SIZE      16      // Maximum 16 timer
#define TIMER_RESOLUTION_MS 1       // Timer handle every 1ms
///
/// Software timer structure
///
typedef struct STimer
{
    int16_t periodic;
    int16_t periodic_tick;
    void (*callback)(void);
}STimer_t;

static STimer_t soft_timer[MAX_TIMER_SIZE];
static volatile uint16_t soft_timer_tick = 0;

int timer_init(void)
{
    /* Clear the timer structure */
    memset(soft_timer, 0, sizeof(soft_timer));
    /* Set timer to use SMCLK, clock divider 2, up-mode */
    TA1CTL = TASSEL1 | ID_1 | MC0;
    /* TA1CCR0 set to the interval for the desires resolution based on 1MHz SMCLK */
    TA1CCR0 = (((1000000 / 2) / 1000) * TIMER_RESOLUTION_MS) - 1;
    /* Enable CCIE interupt */
    TA1CCTL0 = CCIE;
    return 0;
}

void AddTimer(const unsigned long ms, void (*pF)(void))
{
    for (int iCount = 0; iCount < MAX_TIMER_SIZE; iCount++)
    {
        if (soft_timer[iCount].callback == NULL)
        {
            soft_timer[iCount].periodic = ms; //(((ms*CLK_TCK)/1000)+1);
            soft_timer[iCount].periodic_tick = ms; //(((ms*CLK_TCK)/1000)+1);
            soft_timer[iCount].callback = pF;
            break;
        }
    }
}

void DeleteTimer(void (*pF)(void))
{
    __disable_interrupt();
    for (int iCount = 0; iCount < MAX_TIMER_SIZE; iCount++)
    {
        if (soft_timer[iCount].callback == pF)
        {
            soft_timer[iCount].callback = NULL;
        }
    }
    __enable_interrupt();
}
//------------------------------------------------------------------------------------
static void blink_led(void);
static void handle_1s(void);
static void handle_2s(void);
//------------------------------------------------------------------------------------
///
/// Main program body
///
void main( void )
{
    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW + WDTHOLD;
    P3DIR |= BIT5;
    P3OUT |= BIT5;

    // Initialize the timer module
    if (timer_init() != 0) {
        // Timers could not be initialized...hang
        while (1);
    }
    AddTimer(500,blink_led);
    AddTimer(100,handle_1s);
    AddTimer(200,handle_2s);

    __bis_SR_register(GIE);       										// LPM3 + Global Interrupt
    while(1)
    {

    }
}
//------------------------------------------------------------------------------------
///
/// Timer 1 ISR
///
#pragma vector = TIMER1_A0_VECTOR
__interrupt void timer1_isr(void)
{
    /* Clear the interrupt flag */
    TA1CCTL0 &=~ CCIFG;
    for (int iCount = 0; iCount < MAX_TIMER_SIZE; iCount++)
    {
        if (soft_timer[iCount].callback != NULL)
        {
            if ((--soft_timer[iCount].periodic_tick) <= 0)
            {
                soft_timer[iCount].periodic_tick = soft_timer[iCount].periodic;
                soft_timer[iCount].callback();
            }
        }
    }
}

static void blink_led(void)
{

    P3OUT ^= BIT5; // Toggle P3.5 output
}

static void handle_1s(void)
{
    _NOP();
}

static void handle_2s(void)
{
    _NOP();
}
//------------------------------------------------------------------------------------
// End of file
//------------------------------------------------------------------------------------
