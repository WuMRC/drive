/*
  Micro40Timer.h - Using timer2 with 200 microsecond resolution
  Adetunji Dahunsi <tunjid.com>
  
  Based on MsTimer2 by Javier Valencia <javiervalencia80@gmail.com>
	
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Micro40Timer.h>
#include <Arduino.h>  

unsigned long Micro40Timer::microSeconds;
void (*Micro40Timer::func)();
volatile unsigned long Micro40Timer::count;
volatile char Micro40Timer::overflowing;
volatile unsigned int Micro40Timer::tcnt2;

#if defined(__arm__) && defined(TEENSYDUINO)
static IntervalTimer itimer;
#endif

void Micro40Timer::set(unsigned long microS, void (*f)()) {
	float prescaler = 0.0;
	
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || (__AVR_ATmega1280__) || (__AVR_ATmega2560__)
	TIMSK2 &= ~(1<<TOIE2);
	TCCR2A &= ~((1<<WGM21) | (1<<WGM20));
	TCCR2B &= ~(1<<WGM22);
	ASSR &= ~(1<<AS2);
	TIMSK2 &= ~(1<<OCIE2A);
	
	if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL)) {	// prescaler set to 64
		TCCR2B |= (1<<CS22);
		TCCR2B &= ~((1<<CS21) | (1<<CS20));
		prescaler = 64.0;
	} else if (F_CPU < 1000000UL) {	// prescaler set to 8
		TCCR2B |= (1<<CS21);
		TCCR2B &= ~((1<<CS22) | (1<<CS20));
		prescaler = 8.0;
	} else { // F_CPU > 16Mhz, prescaler set to 128
		TCCR2B |= ((1<<CS22) | (1<<CS20));
		TCCR2B &= ~(1<<CS21);
		prescaler = 128.0;
	}
#elif defined (__AVR_ATmega8__)
	TIMSK &= ~(1<<TOIE2);
	TCCR2 &= ~((1<<WGM21) | (1<<WGM20));
	TIMSK &= ~(1<<OCIE2);
	ASSR &= ~(1<<AS2);
	
	if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL)) {	// prescaler set to 64
		TCCR2 |= (1<<CS22);
		TCCR2 &= ~((1<<CS21) | (1<<CS20));
		prescaler = 64.0;
	} else if (F_CPU < 1000000UL) {	// prescaler set to 8
		TCCR2 |= (1<<CS21);
		TCCR2 &= ~((1<<CS22) | (1<<CS20));
		prescaler = 8.0;
	} else { // F_CPU > 16Mhz, prescaler set to 128
		TCCR2 |= ((1<<CS22) && (1<<CS20));
		TCCR2 &= ~(1<<CS21);
		prescaler = 128.0;
	}
#elif defined (__AVR_ATmega128__)
	TIMSK &= ~(1<<TOIE2);
	TCCR2 &= ~((1<<WGM21) | (1<<WGM20));
	TIMSK &= ~(1<<OCIE2);
	
	if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL)) {	// prescaler set to 64
		TCCR2 |= ((1<<CS21) | (1<<CS20));
		TCCR2 &= ~(1<<CS22);
		prescaler = 64.0;
	} else if (F_CPU < 1000000UL) {	// prescaler set to 8
		TCCR2 |= (1<<CS21);
		TCCR2 &= ~((1<<CS22) | (1<<CS20));
		prescaler = 8.0;
	} else { // F_CPU > 16Mhz, prescaler set to 256
		TCCR2 |= (1<<CS22);
		TCCR2 &= ~((1<<CS21) | (1<<CS20));
		prescaler = 256.0;
	}
	#elif defined(__arm__) && defined(TEENSYDUINO)
	// TODO: should this emulate the limitations and numerical
	// range bugs from the versions above?
	tcnt2 = 0.00004 * 1000000.0;
	return;
#endif
	
	tcnt2 = 256 - (int)((float)F_CPU * 0.001 / (25 * prescaler));
	
	if (microS == 0)
		microSeconds = 1;
	else
		microSeconds = microS; 
		
	func = f;
}

void Micro40Timer::start() {
	count = 0;
	overflowing = 0;
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || (__AVR_ATmega1280__) || (__AVR_ATmega2560__)
	TCNT2 = tcnt2;
	TIMSK2 |= (1<<TOIE2);
#elif defined (__AVR_ATmega128__)
	TCNT2 = tcnt2;
	TIMSK |= (1<<TOIE2);
#elif defined (__AVR_ATmega8__)
	TCNT2 = tcnt2;
	TIMSK |= (1<<TOIE2);
#elif defined(__arm__) && defined(TEENSYDUINO)
	itimer.begin(Micro40Timer::_overflow, tcnt2);
#endif
}

void Micro40Timer::stop() {
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || (__AVR_ATmega1280__) || (__AVR_ATmega2560__)
	TIMSK2 &= ~(1<<TOIE2);
#elif defined (__AVR_ATmega128__)
	TIMSK &= ~(1<<TOIE2);
#elif defined (__AVR_ATmega8__)
	TIMSK &= ~(1<<TOIE2);
#elif defined(__arm__) && defined(TEENSYDUINO)
	itimer.end();
#endif
}

void Micro40Timer::_overflow() {
	count += 40; // increment count by 40 microseconds
	
	if (count >= microSeconds && !overflowing) {
		overflowing = 1;
		count = 0;
		(*func)();
		overflowing = 0;
	}
}

#if defined (__AVR__)
ISR(TIMER2_OVF_vect) {
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || (__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
	TCNT2 = Micro40Timer::tcnt2;
#elif defined (__AVR_ATmega128__)
	TCNT2 = Micro40Timer::tcnt2;
#elif defined (__AVR_ATmega8__)
	TCNT2 = Micro40Timer::tcnt2;
//#elif defined(__arm__) && defined(TEENSYDUINO)

#endif
	Micro40Timer::_overflow();
}
#endif // AVR
