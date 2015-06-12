#include <avr/io.h>
#include <avr/interrupt.h>

#define NOUT 2
#define NSTEPS 2

#define LED_PORT PORTC
#define LED_DDR DDRC
#define LED_PIN 0

#if (NOUT>8)
  #error NOUT should be < 8
#endif

struct stStep {
  uint8_t out;
  uint32_t duration;
};

volatile uint8_t* out_ddr[NOUT] = {&DDRB, &DDRB};
volatile uint8_t* out_port[NOUT] = {&PORTB, &PORTB};
uint8_t out_pin[NOUT] = {1, 0};

struct stStep TimTab[NSTEPS] = {
	{0b0,	238	* 60l},
	{0b11,	2	* 60l},
};

volatile uint32_t time = 0;

/*
 * 1 tick per second
 * using 32768Hz crystal oscillator
 */
ISR(TIMER2_OVF_vect)
{
	time++;
}

void main(void)
{
	int i, istep;
	uint32_t tnext,_time,tled;

	time = 0;
	tnext = 0;
	istep = 0;
	tled = 0;

	LED_DDR |= 1 << LED_PIN;
	LED_PORT &= ~(1 << LED_PIN);

	for (i = 0; i < NOUT; i++)
		*out_ddr[i] |= 1 << out_pin[i];

	ASSR |= 1 << AS2; /* external clock source */
	TCCR2 = 5 << CS20; /* clk / 128 */
	TCNT2 = 0;
	while (ASSR & ((1<<TCN2UB) | (1<<TCR2UB)));
	TIFR |= 1 << TOV2;
	TIMSK |= 1 << TOIE2;
	
	sei();

	while (1) {
		TIMSK &= ~(1 << TOIE2);
		_time = time;
		TIMSK |= (1 << TOIE2);

		if (_time >= tnext) {
			for (i = 0; i < NOUT; i++) {
				if (TimTab[istep].out & (1 << i))
					*out_port[i] |= 1 << out_pin[i];
				else
					*out_port[i] &= ~(1 << out_pin[i]);
			}
			tnext += TimTab[istep].duration;
			istep++;
			if (istep >= NSTEPS)
				istep=0;
		}

		if (_time - tled >= 1) {
			if (LED_PORT & (1 << LED_PIN))
				LED_PORT &= ~(1 << LED_PIN);
			else
				LED_PORT |= 1 << LED_PIN;
			tled = _time;
		}
	};
}
