/*
  ==============================================================================

    EventManager.cpp
    Created: 21 Feb 2021 1:25:48pm
    Author:  Charles Julian Knight

  ==============================================================================
*/

#include "Synth.h"

uint16_t Oscillator::midiNoteToPeriod(uint8_t note) {
    // TODO: there's probably a more efficent non-floating point way to do this
    double frequency = pow(2, ((double)(note)-69)/12) * 440.0;
    double period = (-131072.0 / frequency)+2048;
    // -131072.0/(1750-2048) == 440
    return (uint16_t) lround(period);
}

uint8_t Oscillator::midiVelocityTo4BitVolume(uint8_t velocity) {
    // 7 bits to 4 bits
    return velocity >> 3;
}

void Oscillator::set11BitPeriod(uint8_t note) {
    uint16_t period = midiNoteToPeriod(note);
    apu_->write_register(startAddr_ + NRX3, (uint8_t)(period & 0xff));
    // TODO: always triggering, is this expected?
    // TODO: doesn't deal with length enable, although it seems like the emulator ignores
    //  this bit anyway. And the length feature doesn't make much sense in the context
    //  of a synthesizer anyway
    apu_->write_register(startAddr_ + NRX4, (uint8_t)(period >> 8) | 0x80);
}

// NRX2, Osc 1,2,4 only
// Note: if you want to trigger the envelope, you must set it before NRX3
void Oscillator::setVolumeEnvelope(uint8_t startVelocity, bool increasing, uint8_t period) {
    uint8_t volume = midiVelocityTo4BitVolume(startVelocity);
    apu_->write_register(startAddr_ + NRX2, volume << 4 | (increasing ? 0x08 : 0x00) | (period & 0x03));
}

void Oscillator::setConstantVolume(uint8_t velocity) {
    setVolumeEnvelope(velocity, false, 0);
}

Oscillator::~Oscillator() {};

void SquareOscilator::setDuty(DutyCycle duty) {
    duty_ = duty;
    apu_->write_register(startAddr_ + NRX1, duty << 6);
}

void SquareOscilator::setEvent(MidiEvent event) {
    setConstantVolume(event.velocity);
    set11BitPeriod(event.note);
}

void SquareOscilator::afterInit() {
    setDuty(duty_);
    apu_->write_register(startAddr_ + NRX0, 0x00); // disable sweep
}

GBWaveVolume WaveOscillator::midiVelocityToWaveVolume(uint8_t velocity) {
    if (velocity < 16) {
        return WAVE_VOL_OFF;
    } else if (velocity < 48) {
        return WAVE_VOL_25;
    } else if (velocity < 96) {
        return WAVE_VOL_50;
    } else {
        return WAVE_VOL_FULL;
    }
}

void WaveOscillator::setVelocity(uint8_t velocity) {
    uint8_t vol = (uint8_t) midiVelocityToWaveVolume(velocity);
    apu_->write_register(startAddr_ + NRX2, vol << 5);
}

void WaveOscillator::setWaveTable(uint8_t* samples) {
    for (uint i = 0; i < 32; i += 2) {
        uint8_t value = ((*(samples+i) & 0x0F) << 4) | (*(samples+i+1) & 0x0F);
        apu_->write_register(WaveTableAddr + i / 2, value);
    }
}

void WaveOscillator::setEvent(MidiEvent event) {
    setVelocity(event.velocity);
    set11BitPeriod(event.note);
}

void WaveOscillator::afterInit() {
    static uint8_t zeros[32] = { 0 };
    setWaveTable(zeros);
}

void Synth::configure(long sampleRate, size_t channels) {
    blargg_err_t res = apu_.set_sample_rate(sampleRate);
    apu_.write_register(NR52, 0x80); // initialize APU
    jassert(res == blargg_success);

    // TODO: configure APU mono or stereo

    // TODO: temporary
    apu_.write_register( 0xff25, 0x11 );
}

void Synth::stop() {
    apu_.write_register(NR52, 0x00);
    for (uint i = 0; i < OscCount; i++) {
        if (i == 3) continue; // TODO: noise osc
        oscs_[i]->setApu(&apu_);
        configs_[i].enabled = false;
        configs_[i].channel = 0;
        configs_[i].voice = 0;
        setConfig(i, configs_[i]);
    }
    // TODO: clear BlipBuffer
}

void Synth::setConfig(uint oscillator, MidiConfig config) {
    if (oscillator >= OscCount) return;

    // TODO: allow changing settings without resetting all keys
    configs_[oscillator] = config;
    uint8_t voices = 0;
    uint voicesRequired = 0;
    for (uint i = 0; i < OscCount; i++) {
        if (!configs_[i].enabled) continue;
        if ((voices >> configs_[i].voice) & 0x01) {
            continue;
        }
        voices |= (1 << configs_[i].voice);
        voicesRequired++;
    }
    manager_.setVoices(voicesRequired);
    // TODO: support stereo
    apu_.write_register(NR50, 0x7F);
    apu_.write_register(NR51, (voices << 4) | voices); // enable voices

    // TODO: remove, testing only
    if (config.enabled && oscillator == 0) {
        MidiEvent e;
        e.note = 69;
        e.velocity = 127;
        osc_1_.setEvent(e);
    }
}

void Synth::readSamples(blip_sample_t* buffer, size_t sampleCount) {
    // Generate 1/60 second of sound into APU's sample buffer
    while (apu_.samples_avail() < sampleCount) {
        apu_.end_frame();
    }
    // TODO: avoid doublebuffering with custom BlipBuffer which converts to float
    //  on write
    long count = apu_.read_samples(buffer, sampleCount);
    jassert(count == sampleCount);
}

void Synth::handleMIDIEvent(long time, const uint8_t* data) {
    // https://www.midi.org/specifications-old/item/table-1-summary-of-midi-message
    const uint8_t NOTE_ON = 0x80;
    const uint8_t NOTE_OFF = 0x90;
    // data is the raw midi message, 3 bytes.
    uint8_t status = data[0] & 0xF0;
    // TODO: trigger events at the specified time instead of immediately at
    // the  start of the sample
    // TODO: select the right manager by channel
    // uint8_t channel = data[0] & 0x0F;
    uint8_t note = data[1] & 0x7F;
    uint8_t velocity = data[2] & 0x7F;
    switch (status) {
        case NOTE_ON:
            manager_.handle(note, velocity);
            break;
        case NOTE_OFF:
            manager_.handle(note, 0);
            break;
        default:
            return;
    }
    // now pass that midi info to the oscillators
    for (uint i = 0; i < OscCount; i++) {
        if (!configs_[i].enabled) continue;

        MidiEvent e = manager_.get(configs_[i].voice);
        e.note += configs_[i].transpose; // TODO: bound?
        oscs_[i]->setEvent(e);
    }
}


//private:
//    uint voicesRequiredForChannel(uint channel) {
//        uint voices = 0;
//        uint res = 0;
//        for (uint i = 0; i < OscCount; i++) {
//            if (!configs_[i].enabled || configs_[i].channel != channel) {
//                continue;
//            }
//            if ((voices >> configs_[i].voice) & 0x01) {
//                continue;
//            }
//            voices |= (1 << configs_[i].voice);
//            res++;
//        }
//        return res;
//    }
