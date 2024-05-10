#include "config.h"
#include <inttypes.h>
#include <Arduino.h>

#include "pwm.h"
#include "pid.h"
#include "pulsectl.h"
#include "swspi.h"
#include "timer.h"
#include "enc.h"
#include "storage.h"

int16_t pwm;
int32_t position;

/* Position definition:
    0 + startupAngle/360 * 2^15 is startup value
    then counts in pos/neg direction with 2^15 steps per rev
*/


int main() {
    setup();
    while (true) {
        loop();
    }
}

void setup() {
    if(!eeprom_initialized()){
        eeprom_flush();
    }
    pwm_init();
    storage_init();
    swspi_init();
    enc_init();
    pulse_control_init(enc_get_position_value());
    pid_init(reg_seek_position);
    timer_init();
    sei();
    pwm_enable();
}

void loop() {
    pulse_control_update();
  
    // Get the new position value.
    position = enc_get_position_value(); 
  
    // Call the PID algorithm module to get a new PWM value.
    pwm = pid_position_to_pwm(position);
  
    // Update the servo movement as indicated by the PWM value.
    // Sanity checks are performed against the position value.
    pwm_update(position, pwm);
  
    // Store position just in case of power loss
    storage_update(position);
}
