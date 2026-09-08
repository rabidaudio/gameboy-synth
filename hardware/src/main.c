#include <gb/gb.h>
#include <stdint.h>

#include "synth/synth.h"

#include <stdio.h>

// a global instance of synth state
Synth GLOBAL_SYNTH;

// by doing this as a macro it should get compiled down to a direct write
#define GETREG(addr) _IO[addr-0xFF00]

inline void apu_writeRegister(uint16_t addr, uint8_t data) {
    GETREG(addr) = data;
}

inline uint8_t apu_readRegister(uint16_t addr) {
    return GETREG(addr);
}

void tim(void)
{
    synth_handleTimer();
}

void main(void)
{
    synth_init();

    CRITICAL {        
        add_TIM(tim);
        set_interrupts(VBL_IFLAG | TIM_IFLAG);
        synth_configureTimers();
    }

    // synth_setChannel(SYNTH_OSC1, SYNTH_CHANNEL_STATE_FIXEDLEN);
    synth_setLength(SYNTH_OSC1, 60);
    synth_setEnvelope(SYNTH_OSC1, 0, 0);

    // synth_setTranspose(SYNTH_OSC2, 7); // 5th above
    // synth_setLength(SYNTH_OSC2, 26);
    // synth_setChannel(SYNTH_OSC1, 0); // disabled
    // synth_setChannel(SYNTH_OSC4, SYNTH_CHANNEL_STATE_ENABLED);
    // synth_setLength(SYNTH_OSC4, 60);

    uint8_t x = 0;
    uint8_t note = 69;
    bool y = false;

    // Loop forever
    while(1) {
        if (x == 0) {
            if (y) {
                synth_setNote(SYNTH_OSC1, note);
                synth_triggerNote(SYNTH_OSC1);
            } else {
                synth_stopNote(SYNTH_OSC1);
            }
            y = !y;

            // note++;
            // if (note >= 128) note = 26;
        }
        x++;
        if (x == 90) x = 0;

		// Done processing, yield CPU and wait for start of next frame
        vsync();
    }
}
