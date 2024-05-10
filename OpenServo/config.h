#ifndef _OS_CONFIGSERVO_H_
#define _OS_CONFIGSERVO_H_



// Number of turns the servo should have
#define TURN_NUMBER             5

/* Position is measured in steps, with 32768 steps per revolution
    Define min position which corresponds to minimum pulse width
    It is also possible to give a negative min position to have the 5 turn window 
    be somewhere else
*/ 
#define MIN_POSITION            0
// same thing for max pos, here defined by number of turns desired
#define MAX_POSITION            (MIN_POSITION + 32768*TURN_NUMBER)

#define MIN_PULSE_LENGTH        1000
#define MAX_PULSE_LENGTH        2000


#endif // _OS_CONFIGSERVO_H_