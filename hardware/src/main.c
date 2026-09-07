#include <gb/gb.h>
#include <stdint.h>

#include "synth/synth.h"

// a global instance of synth state
// TODO: fit in hiram?
Synth GLOBAL_SYNTH;

// by doing this as a macro it should get compiled down to a direct write
#define GETREG(addr) _IO[addr-0xFF00]

inline void apu_writeRegister(uint16_t addr, uint8_t data) {
    GETREG(addr) = data;
}

inline uint8_t apu_readRegister(uint16_t addr) {
    return GETREG(addr);
}

void main(void)
{
    synth_init();

    synth_setTranspose(SYNTH_OSC2, 7); // 5th above
    synth_setLength(SYNTH_OSC1, 52);
    synth_setLength(SYNTH_OSC2, 26);

    uint8_t x = 0;
    uint8_t note = 36;
    uint16_t period = 0;

    // Loop forever
    while(1) {
        if (x == 0) {
            synth_setNote(SYNTH_OSC1, note);
            synth_setNote(SYNTH_OSC2, note);
            synth_triggerNote(SYNTH_OSC1);
            synth_triggerNote(SYNTH_OSC2);
            note++;
            if (note >= 128) note = 36;
        }
        x++;
        if (x == 30) x = 0;

		// Done processing, yield CPU and wait for start of next frame
        vsync();
    }
}
