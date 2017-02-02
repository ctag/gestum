/**
 * SPACE SENSEI - Let's learn about IMUs
 * Christopher Bero <csb0019@uah.edu>
 */

#include "main.h"

/* Init
 * This function is kept over from the Arduino library for no reason.
 * I have arbitrarily decided that it will be used to initialize
 * low level hardware.
 */
void init ()
{
        // Pin setup
        DDRB = (1 << PB1); // Set as output
        DDRD = (1 << PD6); // Set as output
}

/**
 * setup() - a poem
 * GLaDOS is sleeping \
 * The testing floors are quiet \
 * But where are we now?
 */
void setup()
{
	//
}

/* Loop
 * Does all of the main tasks.
 */
void loop()
{
	//
}

/* Main
 * This function is used to override main() provided by the Arduino library.
 * It simply structures the other high-level functions.
 */
int main(void)
{
        init();

    setup();

    for (;;)
    {
        loop();
    }

    return 0;
}
