#ifndef Micro40Timer_h
#define Micro40Timer_h

#include <avr/interrupt.h>

namespace Micro40Timer {
	extern unsigned long microSeconds;
	extern void (*func)();
	extern volatile unsigned long count;
	extern volatile char overflowing;
	extern volatile unsigned int tcnt2;
	
	void set(unsigned long microS, void (*f)());
	void start();
	void stop();
	void _overflow();
}

#endif
