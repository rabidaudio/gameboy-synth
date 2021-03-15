/*
  ==============================================================================

    EventManager.cpp
    Created: 21 Feb 2021 1:25:48pm
    Author:  Charles Julian Knight

  ==============================================================================
*/

#include "Synth.h"

Apu::Apu()
{
    stereo_ = true;
    buf_ = &sbuf_; // default streo
    clock_ = 0;
}

Apu::~Apu() {}

void Apu::configure(double sampleRate, int channels)
{
    stereo_ = channels != 1;
    if (stereo_) {
        buf_ = &sbuf_;
        apu_.output(sbuf_.center(), sbuf_.left(), sbuf_.right());

    } else {
        buf_ = &mbuf_;
        apu_.output(mbuf_.center());
    }
    buf_->clock_rate(CLOCK_SPEED);
    blargg_err_t res = buf_->set_sample_rate((long) sampleRate);
    jassert(res == blargg_success);
    // Adjust frequency equalization to make it sound like a tiny speaker
    // TODO: expose these parameters
    apu_.treble_eq(-20.0); // lower values muffle it more
    buf_->bass_freq(461); // higher values simulate smaller speaker
    writeRegister(NR52, 0x80); // turn on
}

void Apu::writeRegister(gb_addr_t addr, uint8_t data)
{
    apu_.write_register(tick(), addr, data);
}

uint8_t Apu::readRegister(gb_addr_t addr)
{
    return apu_.read_register(tick(), addr);
}

inline long Apu::samplesAvailable()
{
    if (stereo_) {
        return sbuf_.samples_avail() / 2;
    }
    return mbuf_.samples_avail();
}

void Apu::readSamples(juce::AudioBuffer<float>* out)
{
    // TODO: is it a performance problem to simulate and read in very small steps?
    // is this better than double buffering?
    long sampleCount = out->getNumSamples();
    jassert( (stereo_ && out->getNumChannels() == 2) || (out->getNumChannels() == 1) );
    long read = 0;
    int channelCount = stereo_ ? 2 : 1;
    while (read < sampleCount) {
        while (!samplesAvailable()) {
            bool stereo = apu_.end_frame(tick());
            buf_->end_frame(clock_, stereo);
        }
        buf_->read_samples(samples_, channelCount);
        for (int c = 0; c < channelCount; c++) {
            out->getWritePointer(c)[read] = ((float) samples_[c]) / 0x7FFF;
        }
        read++;
    }
    clock_ = 0;
}

void Apu::reset()
{
    writeRegister(NR52, 0x00); // turn off
    sbuf_.clear();
    mbuf_.clear();
    clock_ = 0;
    apu_.reset();
}

uint16_t Oscillator::midiNoteToPeriod(uint8_t note)
{
//    double frequency = pow(2, ((double)(note)-69)/12) * 440.0;
    double frequency = juce::MidiMessage::getMidiNoteInHertz(note);

    // Note: notes below #36 will wrap around. I am choosing to consider this
    // the desired behavior
    double period = (-131072.0 / frequency)+2048;
    // -131072.0/(1750-2048) == 440
    return (uint16_t) lround(period) & 0x07FF;
}

uint8_t Oscillator::midiVelocityTo4BitVolume(uint8_t velocity)
{
    return velocity >> 3; // 7 bits to 4 bits
}

void Oscillator::set11BitPeriod(uint8_t note)
{
    uint16_t period = midiNoteToPeriod(note);
    apu_->writeRegister(startAddr_ + NRX3, (uint8_t)(period & 0xff));
    // TODO: always triggering, is this expected?
    // TODO: doesn't deal with length enable, although it seems like the emulator ignores
    //  this bit anyway. And the length feature doesn't make much sense in the context
    //  of a synthesizer anyway
    apu_->writeRegister(startAddr_ + NRX4, (uint8_t)(period >> 8) | 0x80);
}

// NRX2, Osc 0,1,3 only
// Note: if you want to trigger the envelope, you must set it before NRX3
void Oscillator::setVolumeEnvelope(uint8_t startVelocity, bool increasing, uint8_t period)
{
    jassert(id_ != 2);
    uint8_t v = midiVelocityTo4BitVolume(startVelocity);
    v = (uint8_t)((float) v * volume); // scaled
    apu_->writeRegister(startAddr_ + NRX2, v << 4 | (increasing ? 0x08 : 0x00) | (period & 0x03));
}

void Oscillator::setConstantVolume(uint8_t velocity)
{
    setVolumeEnvelope(velocity, false, 0);
}

Oscillator::~Oscillator() {};

void SquareOscilator::setDuty(DutyCycle duty)
{
    if (duty == duty_) return;
    duty_ = duty;
    apu_->writeRegister(startAddr_ + NRX1, (uint8_t) duty << 6);
}

void SquareOscilator::setEvent(MidiEvent event)
{
    if (event.note < 36 || event.note > 108) {
        setConstantVolume(0); // ignore it
        return;
    }
    setConstantVolume(event.velocity);
    set11BitPeriod(event.note);
}

void SquareOscilator::afterInit()
{
    setDuty(duty_);
    apu_->writeRegister(startAddr_ + NRX0, 0x00); // disable sweep
}

GBWaveVolume WaveOscillator::midiVelocityToWaveVolume(uint8_t velocity)
{
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

void WaveOscillator::setVelocity(uint8_t velocity)
{
    uint8_t vol = (uint8_t) midiVelocityToWaveVolume(velocity);
    apu_->writeRegister(startAddr_ + NRX2, vol << 5);
}

void WaveOscillator::setWaveTable(uint8_t* samples)
{
    // TODO: pandocs say you should only change the wavetable while the osc is off
    // apu_->writeRegister(startAddr_ + NRX0, 0x00);
    for (uint i = 0; i < 32; i += 2) {
        uint8_t value = ((*(samples+i) & 0x0F) << 4) | (*(samples+i+1) & 0x0F);
        apu_->writeRegister(WaveTableAddr + i / 2, value);
    }
}

void WaveOscillator::setEvent(MidiEvent event)
{
    if (event.note < 36 || event.note > 120) {
        setVelocity(0); // ignore it
        return;
    }
    setVelocity(event.velocity);
    set11BitPeriod(event.note);
}

void WaveOscillator::afterInit()
{
    apu_->writeRegister(startAddr_ + NRX0, 0x80); // enable the dac
}

static bool switch_ = false; // TODO: super hacky temp
static int offset_ = 0;

void NoiseOscillator::setEvent(MidiEvent event)
{

    // s=0, w=0, r=0 -> very white noise
    // increasing s, w=0, r=0 -> increasing tone, eventually sparse clicky
    // s=0, w=0, increasing r -> each step reduces tone by steps, seems like roughly 3rds, 5ths and octaves?
    // s=0, w=1, r=0 -> much more tonal
    // increasing s, w=1, r=0 -> each step reduces octave of tone by 1, eventually sparse clicky
    // s=0, w=1, increasing r -> each step reduces tone, but not by octave, seems like roughly  3rds, 5ths and octaves?
    // s=3,w=1,r=1 == s=2,w=1,r=2 (texture might be different but tone is the same
    // this pattern seems to hold roughly - increasing s and decresing r keeps the same tone but a different texture
    // s:0,w:1,r:2 == s:2,w:1,r:0

    // s:0,w:1,r:4 (C5) == s:4,w:1,r:0 (Bflat3) very different texture but the same sound
    // s:0,w:1,r:6 (G#5) == s:6,w:1,r:0 (B3) B3 is much clicker, tonal difference is significant, but probably the same actual tone

    // s:0,w:0,r:4 (E2) == s:4,w:0,r:0 (D1) both white noise, comparable texture but different low-frequency "beats"


    // 23 for even s+r range
    // *2 for width
    // *8 for offset but really a lot of these sound the sames
//    uint8_t n = event.note; // 7 bits
//    if (n >= 41 && n < 36+23) {
//        bool width = switch_;
//        int sum = n - 41;
//        int s, r;
//        r = offset_ + 4;
//        s = sum - r;
//        // shrink the allowed range of r at the bounds, but keeping the s/r sum
//        if (s < 0) {
//            s = 0;
//            r = sum - s;
//        } else if (s > 15) {
//            s = 15;
//            r = sum - s;
//        }
//
//        if (event.velocity > 0) {
//            printf("sum:%d,s:%d,w:%d,r:%d\n", sum, s, width, r);
//        }
//
//        jassert(s >= 0 && s < 16);
//        jassert(r >= 0 && r < 8);
//
//        uint8_t v = ((uint8_t)s << 4) | (width ? 0x80 : 0x00) | (uint8_t)r;
//        apu_->writeRegister(startAddr_ + NRX3, v);
//
//
//    }

    setConstantVolume(event.velocity);
    apu_->writeRegister(startAddr_ + NRX4, 0x80); // also need to set the trigger
}

void NoiseOscillator::afterInit()
{
    apu_->writeRegister(startAddr_ + NRX3, 0x00); // TODO: initial value?
}

void NoiseOscillator::setShiftFrequency(uint8_t s)
{
    uint8_t v = apu_->readRegister(startAddr_ + NRX3);
    v = (v & 0x0F) | ((s & 0x0F) << 4);
    apu_->writeRegister(startAddr_ + NRX3, v);
}
void NoiseOscillator::setCounterWidth(bool narrow)
{
    uint8_t v = apu_->readRegister(startAddr_ + NRX3);
    v = (v & 0xF7) | (narrow ? 0x08 : 0x00);
    apu_->writeRegister(startAddr_ + NRX3, v);
//    switch_ = narrow;
}
void NoiseOscillator::setDividerRatio(uint8_t r)
{
    uint8_t v = apu_->readRegister(startAddr_ + NRX3);
    v = (v & 0xF8) | (r & 0x07);
    apu_->writeRegister(startAddr_ + NRX3, v);
//    offset_ = ((int)r) - 4;
}

Synth::Synth()
{
    setDefaults();
}

void Synth::configure(double sampleRate, int channels)
{
    apu_.configure(sampleRate, channels);
}

void Synth::setDefaults()
{
    for (OSCID i = 0; i < NUM_OSC; i++) {
        oscs_[i]->setApu(&apu_);
        configs_[i].enabled = false;
        configs_[i].channel = 0;
        configs_[i].voice = 0;
        reconfigure(i);
    }
}

void Synth::stop()
{
    apu_.reset();
}

void Synth::setEnabled(OSCID oscillator, bool enabled)
{
    jassert(oscillator < NUM_OSC);
    configs_[oscillator].enabled = enabled;
    reconfigure(oscillator);
}

void Synth::setTranspose(OSCID oscillator, int8_t transpose)
{
    jassert(oscillator < NUM_OSC);
    configs_[oscillator].transpose = transpose;
    reconfigure(oscillator);
}

void Synth::setMIDIVoice(OSCID oscillator, uint8_t voice)
{
    jassert(oscillator < NUM_OSC);
    configs_[oscillator].voice = voice;
    reconfigure(oscillator);
}

void Synth::setMIDIChannel(OSCID oscillator, uint8_t channel)
{
    jassert(oscillator < NUM_OSC);
    configs_[oscillator].channel = channel & 0x0F;
    reconfigure(oscillator);
}

void Synth::reconfigure(OSCID oscillator)
{
    // TODO: allow changing settings without resetting all keys
    uint8_t voices = 0;
    uint voicesRequired = 0;
    uint8_t enabled = 0;
    for (uint i = 0; i < NUM_OSC; i++) {
        if (!configs_[i].enabled) continue;
        enabled |= (1 << i);
        if ((voices >> configs_[i].voice) & 0x01) {
            continue;
        }
        voices |= (1 << configs_[i].voice);
        voicesRequired++;
    }
    manager_.setVoices(voicesRequired);
    // TODO: support stereo assignment
    apu_.writeRegister(NR50, 0x7F);
    apu_.writeRegister(NR51, (enabled << 4) | enabled); // enable voices
}

void Synth::handleMIDI(juce::MidiBuffer& midiMessages)
{
    for (const juce::MidiMessageMetadata metadata : midiMessages) {
        handleMIDIEvent(metadata.getMessage());
    }
}

void Synth::readSamples(juce::AudioBuffer<float> *out)
{
    apu_.readSamples(out);
}

void Synth::handleMIDIEvent(juce::MidiMessage msg)
{
    if (msg.isSysEx()) return;

    if (manager_.voices() == 0) return;
    // https://www.midi.org/specifications-old/item/table-1-summary-of-midi-message
    // TODO: use time of msg
    if (msg.isNoteOn()) {
        manager_.handle(msg.getNoteNumber(), msg.getVelocity());
    } else if (msg.isNoteOff()) {
        manager_.handle(msg.getNoteNumber(), 0);
    } else {
        return;
    }
    // now pass that midi info to the oscillators
    for (OSCID i = 0; i < NUM_OSC; i++) {
        if (!configs_[i].enabled) continue;
        MidiEvent e = manager_.get(configs_[i].voice);
        e.note += configs_[i].transpose;
        oscs_[i]->setEvent(e);
    }
}
