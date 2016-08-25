//------------------------------------------------------------------------------------
// Project: Software timer for MSP430FR5739
//------------------------------------------------------------------------------------
#include <msp430fr5739.h>
#include <stdint.h>
#include <string.h>
//------------------------------------------------------------------------------------
#define MAX_TIMERS 16
#define TIMER_RESOLUTION_MS 1
#define SR_ALLOC() uint16_t __sr
#define ENTER_CRITICAL() __sr = __get_interrupt_state(); __disable_interrupt()
#define EXIT_CRITICAL() __set_interrupt_state(__sr)

///
/// Software timer structure
///
struct timer
{
    uint16_t expiry;
    uint16_t periodic;
    void (*callback)(void);
};
static struct timer _timer[MAX_TIMERS];
static volatile uint16_t _timer_tick = 0;

int timer_init(void)
{
    /* Clear the timer structure */
    memset(_timer, 0, sizeof(_timer));
    /* Set timer to use SMCLK, clock divider 2, up-mode */
    TA1CTL = TASSEL1 | ID_1 | MC0;
    /* TA1CCR0 set to the interval for the desires resolution based on 1MHz SMCLK */
    TA1CCR0 = (((1000000 / 2) / 1000) * TIMER_RESOLUTION_MS) - 1;
    /* Enable CCIE interupt */
    TA1CCTL0 = CCIE;
    return 0;
}

int timer_create(uint16_t timeout_ms, int periodic, void (*callback)(void))
{
    int handle = -1;
    size_t i;

    /* Find a free timer */
    for (i = 0; i < MAX_TIMERS; i++)
    {
        if (_timer[i].callback == NULL)
        {
            break;
        }
    }

    /* Make sure a valid timer is found */
    if (i < MAX_TIMERS)
    {
        SR_ALLOC();
        ENTER_CRITICAL();

        /* Set up the timer */
        if (periodic != 0)
        {
            _timer[i].periodic = (timeout_ms < 100) ? 1 : (timeout_ms / TIMER_RESOLUTION_MS);
        }
        else
        {
            _timer[i].periodic = 0;
        }

        _timer[i].callback = callback;
        _timer[i].expiry = _timer_tick + _timer[i].periodic;

        EXIT_CRITICAL();
        handle = i;
    }

    return handle;
}

int timer_delete(int handle)
{
    int status = -1;

    if (handle < MAX_TIMERS) {
        SR_ALLOC();
        ENTER_CRITICAL();

        /* Clear the callback to delete the timer */
        _timer[handle].callback = NULL;

        EXIT_CRITICAL();
        status = 0;
    }

    return status;
}
//------------------------------------------------------------------------------------
int timer_handle,handle_count;
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
    __bis_SR_register(GIE);       										// LPM3 + Global Interrupt

    timer_handle = timer_create(2000, 1, blink_led);
    timer_create(2000, 1, handle_1s);
    timer_create(2000, 1, handle_2s);
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
    size_t i;
    /* Clear the interrupt flag */
    TA1CCTL0 &=~ CCIFG;
    /* Increment the timer tick */
    _timer_tick++;
    for (i = 0; i < MAX_TIMERS; i++)
    {
        /* If the timer is enabled and expired, invoke the callback */
        if ((_timer[i].callback != NULL) && (_timer[i].expiry == _timer_tick))
        {
            (*_timer[i].callback)();
            if (_timer[i].periodic > 0)
            {
                /* Timer is periodic, calculate next expiration */
                _timer[i].expiry += _timer[i].periodic;
            }
            else
            {
                /* If timer is not periodic, clear the callback to disable */
                _timer[i].callback = NULL;
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
