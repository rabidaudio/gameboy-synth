#include <gb/gb.h>
#include <stdint.h>

#include "synth.h"

// a global instance of synth state
// TODO: fit in hiram?
Synth synth;

void main(void)
{
    synth_init(&synth);

    uint8_t x = 0;
    uint8_t note = 36;
    uint16_t period = 0;

    // Loop forever
    while(1) {
        if (x == 0) {
            synth_play_note(note);
            note++;
            if (note >= 128) note = 36;
        }
        x++;
        if (x == 30) x = 0;

		// Done processing, yield CPU and wait for start of next frame
        vsync();
    }
}
