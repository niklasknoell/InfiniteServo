/*
    Copyright (c) 2006 Michael P. Thompson <mpthompson@gmail.com>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

    $Id$
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "pulsectl.h"
#include "pwm.h"
#include "timer.h"

// Globals used for pulse measurement.
static volatile uint8_t overflow_count;
static volatile uint8_t pulse_flag;
static volatile uint16_t pulse_time;
static volatile uint16_t pulse_duration;
uint32_t reg_seek_position;

//
// Digital Lowpass Filter Implementation
//
// See: A Simple Software Lowpass Filter Suits Embedded-system Applications
// http://www.edn.com/article/CA6335310.html
//
// k    Bandwidth (Normalized to 1Hz)   Rise Time (samples)
// 1    0.1197                          3
// 2    0.0466                          8
// 3    0.0217                          16
// 4    0.0104                          34
// 5    0.0051                          69
// 6    0.0026                          140
// 7    0.0012                          280
// 8    0.0007                          561
//

#define FILTER_SHIFT 2

static int32_t filter_reg;

static int32_t filter_update(int32_t input)
{
  
    // Update the filter with the current input.
    filter_reg = filter_reg - (filter_reg >> FILTER_SHIFT) + input;

    // Scale output for unity gain.
    return (int32_t) (filter_reg >> FILTER_SHIFT);
}

void pulse_control_init(int32_t position)
// Initialize servo pulse control.
{
    // Initialize the pulse time values.
    pulse_flag = 0;
    pulse_duration = 0;
    overflow_count = 0;

    // Initialize the pulse time.
    pulse_time = timer_get();

    // Set timer/counter2 control register A.
    TCCR2A = (0<<COM2A1) | (0<<COM2A0) |                    // Disconnect OC2A.
             (0<<COM2B1) | (0<<COM2B0) |                    // Disconnect OC2B.
             (0<<WGM21) | (0<<WGM20);                       // Mode 0 - normal operation.

    // Configure PCINT3 to interrupt on change.
    PCMSK0 = (1<<PCINT0);
    PCMSK1 = 0;
    PCMSK2 = 0;
    PCICR = (0<<PCIE2) |
            (0<<PCIE1) |
            (1<<PCIE0);

    // Configure PB0/PCINT3 as an input.
    DDRB &= ~(1<<DDB0);
    PORTB &= ~(1<<PINB0);

    //prevent bump start by initialising the seek_position and lowpass filter with current position
    reg_seek_position = position;
    filter_reg = position;
    filter_reg <<= FILTER_SHIFT;
}


void pulse_control_update(void)
// Update the servo seek position and velocity based on any received servo pulses.
{
    int32_t pulse_position;

    // Did we get a pulse?
    if (pulse_flag)
    {
        // Ignore unusual pulse durations as a sanity check.
        if ((pulse_duration > (MIN_PULSE_LENGTH-200)) && (pulse_duration < (MAX_PULSE_LENGTH+200)))
        {
            
             // old implementation, not sure why
             //pulse_position = pulse_duration + (pulse_duration>>4); //might push back less for slight bigger region
             
             // desired position given through pulse length
             pulse_position = MIN_POSITION + (pulse_duration-MIN_PULSE_LENGTH)/(MAX_PULSE_LENGTH - MIN_PULSE_LENGTH) * (MAX_POSITION-MIN_POSITION);



            // This warning should be mostly redundant, moved everything to bigger ints, so ~256 turns should be possible
                            /*Warning: TL;DR only interesting if wanting multipe turns beyond 8 turns.
                             * The max value as read in the table above for pulse_duration for a 2300µS pulse is 4633,
                             * At this moment the corner restore upon reboot saving feature is allowed a maximum value of 255 (8bit).
                             * This value is pulse_position/64 -> Thus pulse_position should NEVER exceed 255*64=16320
                             * Example: A ~4turn servo could be created by 
                             * pulse_position = pulse_duration*2;
                             * Giving a max value for a 2300µS of 9266, a safe value
                             * Example: A ~8 turn servo could be created by
                             * pulse_position = pulse_duration*4;
                             * Giving a max value for a 2300µS of 4633*4=18532, this exeeds the 16320 threshold.
                             * The servo will operate as it should, however if will break on reboot in a position above 1980µS, causing windup behaviour possibily causing damage.
                             * A possible solution for this problem. Since A saving resolution of 1/32th a circle isn't needed in a 11 turn servo, the saving devision could be increased.
                             * This devision is however found on many places and not centralised. It is found:
                             * enc.cpp line 52:
                             *  position >>= 6 // >>=6 stands for /64, this number determines the 255*64 limit
                             * enc.cpp line 55:
                             *  uint16_t offset = ((prev_position+16-position)/32)<<11; // 32 is calculated from 2048/64, 16 is half of that
                             * storage.cpp line 52:
                             *  position >>= 6; // same 32
                             *Warning 2:
                             * The maximum recordable position is 2^32. Due to the use of signed int16_t ofset in enc.cpp line 26, if the position exceeds this value, the servo keeps winding infinitly. 
                             * With an increase in position of 2^11 (2048) for 1 turn this gives max 16 turns
                             * Also in OpenServo.ino line 38 this value is forced converted to singed
                             * This value is then reused in the function pid_position_to_pwm(position);
                             *  In pid.cpp line 41 the function pid_position_to_pwm(int16_t current_position) could be converted to accept the an unsinged value 
                             * And reused in the function pwm_update(position, pwm)
                             *  In pwm.cpp line 183 the function pwm_update(uint16_t position, int16_t pwm), the value is used for sanity checking. It should be updated to accept the unsigned value and wanted range
                             * Also in OpenServo.ino in the main loop at line 164 the if (position >= 0)  { statement should be updated or removed
                             */
            
            // Shouldnt be needed with the adjustable ends
                            // Limit the pulse position. 20% margin on above values for example
                            // if (pulse_position < MIN_POSITION) pulse_position = MIN_POSITION;
                            // if (pulse_position > MAX_POSITION) pulse_position = MAX_POSITION;


            // Apply a low pass filter to the pulse position.
            pulse_position = filter_update(pulse_position);

            reg_seek_position = pulse_position;

            pwm_enable();

            pulse_time = timer_get();
        }

        // Reset the pulse time flag.
        pulse_flag = 0;
    }
    else
    {
        // If we haven't seen a pulse in .5 seconds disable pwm.
        if (timer_delta(pulse_time) > 50)
        {
            // Disable pwm.
            pwm_disable();

            // Update the pulse time used as a time stamp.
            pulse_time = timer_get();
        }
    }

    return;
}

bool pulse_control_flag(void){
  return pulse_flag;
}

ISR(TIMER2_OVF_vect)
// Handles timer/counter2 overflow interrupt.
{
    // Increment the upper byte of the pulse timer.
    overflow_count += 1;
}


ISR(PCINT0_vect)
// Handles pin change 0 interrupt.
{
    // Make sure we don't overwrite pulse times.
    if (!pulse_flag)
    {
        // We have to be careful how we handle Timer2 as we are using it as
        // a 16 bit counter with the overflow incrementing the upper 8 bits.
        // We need to be careful that TOV2 is checked to accurately update the
        // overflow count before the overflow count is used.

        // Did we go high or low?
        if (PINB & (1<<PINB0))
        {
            // Reset timer 2 count values.
            TCNT2 = 0;
            overflow_count = 0;

            // Set timer/counter2 control register B.
            TCCR2B = (0<<FOC2A) | (0<<FOC2B) |                      // No force output compare A or B.
                     (0<<WGM22) |                                   // Mode 0 - normal operation.
                     (0<<CS22) | (1<<CS21) | (0<<CS20);             // Clk/8 prescaler -- a 1MHz clock rate.

            // Set the timer/counter2 interrupt masks.
            TIMSK2 = (0<<OCIE2A) |                                  // No interrupt on compare match A.
                     (0<<OCIE2B) |                                  // No interrupt on compare match B.
                     (1<<TOIE2);                                    // Interrupt on overflow.

            // Clear any pending overflow interrupt.
            TIFR2 = (1<<TOV2);
        }
        else
        {
            // Stop timer2.
            TCCR2B = (0<<FOC2A) | (0<<FOC2B) |                      // No force output compare A or B.
                     (0<<WGM22) |                                   // Mode 0 - normal operation.
                     (0<<CS22) | (0<<CS21) | (0<<CS20);             // Disable clock source.

            // Check for one last overflow.
            if (TIFR2 & (1<<TOV2)) overflow_count += 1;

            // Save the pulse duration.
            pulse_duration = ((uint16_t) overflow_count << 8) | TCNT2;

            // We timed a pulse.
            pulse_flag = 1;

            // Set the timer/counter2 interrupt masks.
            TIMSK2 = (0<<OCIE2A) |                                  // No interrupt on compare match A.
                     (0<<OCIE2B) |                                  // No interrupt on compare match B.
                     (0<<TOIE2);                                    // No interrupt on overflow.

            // Clear any pending overflow interrupt.
            TIFR2 = (1<<TOV2);
        }
    }
}
