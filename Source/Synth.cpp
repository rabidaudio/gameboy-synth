/*
  ==============================================================================

    EventManager.cpp
    Created: 21 Feb 2021 1:25:48pm
    Author:  Charles Julian Knight

  ==============================================================================
*/

#include "Synth.h"
#include "NoiseFrequencyTable.h"

Apu::Apu()
{
    stereo_ = true;
    buf_ = &sbuf_; // default streo
    clock_ = 0;
    timeToNextFrame_ = CLOCKS_PER_FRAME;
}

Apu::~Apu() {}

void Apu::configure(double sampleRate, int channels, FrameListener* listener)
{
    listener_ = listener;
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
    apu_.write_register(tick(CLOCKS_PER_INSTRUCTION), addr, data);
}

uint8_t Apu::readRegister(gb_addr_t addr)
{
    return apu_.read_register(tick(CLOCKS_PER_INSTRUCTION), addr);
}

inline long Apu::samplesAvailable()
{
    if (stereo_) {
        return sbuf_.samples_avail() / 2;
    }
    return mbuf_.samples_avail();
}

blip_time_t Apu::tick(blip_time_t step) {
    clock_ += step; // TODO: remove clock_
    if (timeToNextFrame_ < step) {
        if (listener_ != NULL) {
            listener_->onFrame();
        }
        timeToNextFrame_ += CLOCKS_PER_FRAME;
    }
    timeToNextFrame_ -= step;
//        return clock_;
    return step;
}

gb_time_t Apu::readSamples(juce::AudioBuffer<float>* out)
{
    // TODO: is it a performance problem to simulate and read in very small steps?
    // is this better than double buffering?
    gb_time_t clock = 0;
    long sampleCount = out->getNumSamples();
    jassert( (stereo_ && out->getNumChannels() == 2) || (out->getNumChannels() == 1) );
    long read = 0;
    int channelCount = stereo_ ? 2 : 1;
    bool stereo;
    while (read < sampleCount) {
        while (!samplesAvailable()) {
//            stereo = apu_.end_frame(tick());
            clock += tick(CLOCKS_PER_FRAME);
            stereo = apu_.end_frame(clock);
            buf_->end_frame(clock, stereo);
        }
        buf_->read_samples(samples_, channelCount);
        for (int c = 0; c < channelCount; c++) {
            out->getWritePointer(c)[read] = ((float) samples_[c]) / 0x7FFF;
        }
        read++;
    }
//    clock_ = 0;
    return clock;
}

void Apu::reset()
{
    writeRegister(NR52, 0x00); // turn off
    sbuf_.clear();
    mbuf_.clear();
    clock_ = 0;
    apu_.reset();
    listener_ = NULL;
}

uint16_t midiNoteToPeriod(uint8_t note)
{
//    double frequency = pow(2, ((double)(note)-69)/12) * 440.0;
    double frequency = juce::MidiMessage::getMidiNoteInHertz(note);

    // Note: notes below #36 will wrap around. I am choosing to consider this
    // the desired behavior
    double period = (-131072.0 / frequency)+2048;
    // -131072.0/(1750-2048) == 440
    return (uint16_t) lround(period) & 0x07FF;
}

uint8_t midiVelocityTo4BitVolume(uint8_t velocity)
{
    return velocity >> 3; // 7 bits to 4 bits
}

uint16_t envelopePeriodToFrames(uint8_t period) {
    return (uint16_t) period * 4;
    
    // each audio frame is 4194304 / 256 = 16364 clock cycles (256 Hz)
    // each envelope is 64 Hz, i.e. every 4 frames
//    return (gb_time_t) period * CLOCK_SPEED / 256 * 4;
    
    //    4194304 / 256 * 4 / 68?
        
        // audio frames are 256Hz : 4194304 / 256
        // envelope frames are 64Hz, so every 4 audio frames
    
    // 512 samples == 600 ticks
    // 512 samples at 48KHz ==> 0.01066666667 seconds
    // 600 / 0.01066666667 = 56250 ticks/second
    // 64 Hz => 878.9 ticks
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
void Oscillator::setAttack(uint8_t period)
{
    jassert(id_ != 2);
    attack_ = (period & 0x0F);
}

void Oscillator::setRelease(uint8_t period)
{
    jassert(id_ != 2);
    release_ = (period & 0x0F);
}

void Oscillator::configureEnvelope(uint8_t velocity)
{
    jassert(id_ != 2);
    
    velocity = midiVelocityTo4BitVolume(velocity);
    
    if (velocity == 0) {
        // turn off
        switch (envState_) {
            case EnvelopeState::off: // nothing to do
            case EnvelopeState::release: // should be impossible, but let's just turn off
                envState_ = EnvelopeState::off;
                envelopeFramesRemaining_ = 0;
                apu_->writeRegister(startAddr_ + NRX2, 0); // off
                break;
            case EnvelopeState::on:
            case EnvelopeState::attack:
                // how many clock cycles until the envelope will reach 0 from current velocity
                envelopeFramesRemaining_ = envelopePeriodToFrames(velocity_ * release_);
                if (envelopeFramesRemaining_ == 0) {
                    // no envelope, trigger immeidately
                    envState_ = EnvelopeState::off;
                    apu_->writeRegister(startAddr_ + NRX2, 0); // off
                } else {
                    // start release
                    envState_ = EnvelopeState::release;
                    apu_->writeRegister(startAddr_ + NRX2, (velocity_ << 4) | (uint8_t) EnvelopeDirection::decreasing | release_);
                }
                break;
        }
    } else {
        // turn on
        switch (envState_) {
            case EnvelopeState::on: // nothing to do
            case EnvelopeState::attack: // should be impossible, just turn on
                envState_ = EnvelopeState::on;
                envelopeFramesRemaining_ = 0;
                apu_->writeRegister(startAddr_ + NRX2, (velocity << 4)); // on
                break;
            case EnvelopeState::off:
            case EnvelopeState::release:
                // how many clock cycles until the envelope will reach new velocity from zero
                envelopeFramesRemaining_ = envelopePeriodToFrames(velocity * attack_);
                if (envelopeFramesRemaining_ == 0) {
                    // trigger immediately
                    envState_ = EnvelopeState::on;
                    apu_->writeRegister(startAddr_ + NRX2, (velocity << 4)); // on
                } else {
                    // start attack
                    envState_ = EnvelopeState::attack;
                    uint8_t value = (0 << 4) | (uint8_t) EnvelopeDirection::increasing | attack_;
                    apu_->writeRegister(startAddr_ + NRX2, value);
                }
                break;
        }
    }
    velocity_ = velocity;
}

void Oscillator::onFrame()
{
    if (id_ == 2) return; // no envelope on osc 2
    
    if (envState_ == EnvelopeState::on || envState_ == EnvelopeState::off) {
        return;
    }
    if (envelopeFramesRemaining_ > 0) {
        envelopeFramesRemaining_--;
        return;
    }
    if (envState_ == EnvelopeState::attack) {
        envState_ = EnvelopeState::on;
        envelopeFramesRemaining_ = 0;
//        apu_->writeRegister(startAddr_ + NRX2, (velocity_ << 4)); // on
    }
    if (envState_ == EnvelopeState::release) {
        envState_ = EnvelopeState::off;
        envelopeFramesRemaining_ = 0;
//        apu_->writeRegister(startAddr_ + NRX2, 0); // off
    }
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
        apu_->writeRegister(startAddr_ + NRX2, 0); // off
        return;
    }
    configureEnvelope(event.velocity);
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
    for (uint16_t i = 0; i < 32; i += 2) {
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

void NoiseOscillator::setShiftWidth(NoiseShiftWidth width)
{
    width_ = width;
}

void NoiseOscillator::setEvent(MidiEvent event)
{
    configureEnvelope(event.velocity);
    
    NoiseFrequencyParams frequencyParams = NOISE_PARAM_TABLE[(event.note + 32) % NOISE_PARAM_TABLE_LEN];
    uint8_t noiseRegisterValue = frequencyParams.shift << 4 | (uint8_t) width_ << 3 | frequencyParams.div;
    apu_->writeRegister(startAddr_ + NRX3, noiseRegisterValue);
    
    apu_->writeRegister(startAddr_ + NRX4, 0x80); // start sound
}

void NoiseOscillator::afterInit()
{
    
}

Synth::Synth()
{
    setDefaults();
}

void Synth::configure(double sampleRate, int channels)
{
    apu_.configure(sampleRate, channels, this);
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

void Synth::setAttackPeriod(OSCID oscillator, uint8_t period) {
    jassert(oscillator < NUM_OSC);
    oscs_[oscillator]->setAttack(period);
}

void Synth::setReleasePeriod(OSCID oscillator, uint8_t period) {
    jassert(oscillator < NUM_OSC);
    oscs_[oscillator]->setRelease(period);
}

void Synth::reconfigure(OSCID oscillator)
{
    // TODO: allow changing settings without resetting all keys
    uint8_t voices = 0;
    size_t voicesRequired = 0;
    uint8_t enabled = 0;
    for (OSCID i = 0; i < NUM_OSC; i++) {
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

void Synth::onFrame()
{
    for (OSCID i = 0; i < NUM_OSC; i++) {
        oscs_[i]->onFrame();
    }
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
