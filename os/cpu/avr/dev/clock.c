#include "sys/clock.h"
#include "dev/clock-avr.h"
#include "sys/etimer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile clock_time_t count;

ISR(AVR_OUTPUT_COMPARE_INT)
{
	++count;
	if(etimer_pending()) {
		etimer_request_poll();
	}
}

/*
 * Set up the clock for a 1 msec tick interval
 */
void clock_setup()
{
	/* choose internal clock as source */
	ASSR    = 0;

	/* reset the timer 0 counter to zero */
	TCNT0   = 0;

	/*
	 * set the compare reg to 32. With a prescaler
	 * val of 256, this would give us a ~1 msec interrupt
	 */

	/*
	 * #define F_CPU                   8000000UL
	 * #define AVR_CONF_TMR0_PRESCALE  256
	 * #define AVR_TCCR0B_CONF         _BV(CS02)
	 * Ravenusb:
	 * #define CLOCK_CONF_SECOND	   1000
	 * Raven:
	 * #define CLOCK_CONF_SECOND       125
	 *
	 * Ravenusb:
	 * 每秒8000000Hz, 每秒的时钟滴答数是1000(每秒的中断数)
	 * 分频之后的频率为: 8000000 / 256 = 31250 Hz
	 * 每个时钟滴答的时间为 1000000us / 1000 = 1000 us = 1 ms 
	 * 每个时钟滴答所需要的时钟周期数为8000000 / 256 / 1000 = 31
	 *
	 * Raven:
	 * 然而经过时钟预分频之后, 预分频的值为8000000 / 256 = 31250 Hz
	 * 每个时钟滴答的时间为: 1000000us / 125 = 8000us = 8 ms
	 * 每个时钟滴答所需要的时钟周期数为: 8000000 / 256 / 125 = 250 周期
	 *
	 * F_CPU = AVR_CONF_TMR0_PRESCALE * CLOCK_CONF_SECOND * OCR2A, less 1 for CTC mode
	 * OCR0A = F_CPU / AVR_CONF_TMR0_PRESCALE / CLOCK_CONF_SECOND - 1;
	 */
	OCR0A = 32;

	/*
	 * Set timer control register:
	 *  - prescale: 256
	 *  - counter reset via comparison register (WGM01)
	 */
	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS02);

	/* Clear interrupt flag register */
	TIFR0 = TIFR0;

	/*
	 * Raise interrupt when value in OCR0 is reached. Note that the
	 * counter value in TCNT0 is cleared automatically.
	 */
	TIMSK0 = _BV (OCIE0A);
}

void clock_init(void)
{
	cli ();


	clock_setup();

	/*
	 * Counts the number of ticks. Since clock_time_t is an unsigned
	 * 16 bit data type, time intervals of up to 524 seconds can be
	 * measured.
	 */
	count = 0;

	sei ();
}

clock_time_t clock_time(void)
{
	return count;
}

/**
 * Delay the CPU for a multiple of TODO
 */
void clock_delay(unsigned int i)
{
	unsigned j;

	for (; i > 0; i--) {		/* Needs fixing XXX */
		for (j = 50; j > 0; j--)
			asm volatile("nop");
	}
}

/*
 * Wait for a multiple of 1 / 125 sec = 0.008 ms.
 */
void clock_wait(clock_time_t i)
{
	clock_time_t start;

	start = clock_time();
	while(clock_time() - start < (clock_time_t)i);
}

void clock_set_seconds(unsigned long sec)
{
    // TODO
}

unsigned long clock_seconds(void)
{
	return count / CLOCK_SECOND;
}
