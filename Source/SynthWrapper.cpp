/*
  ==============================================================================

    SynthWrapper.cpp
    Created: 5 Sep 2026 5:29:46pm
    Author:  Julien

  ==============================================================================
*/

#include "SynthWrapper.h"

void apu_writeRegister(uint16_t addr, uint8_t value) {
    SynthWrapper::INSTANCE.writeRegister(addr, value);
}

uint8_t apu_readRegister(uint16_t addr) {
    return SynthWrapper::INSTANCE.readRegister(addr);
}

SynthWrapper::SynthWrapper() {
    stereo_ = true;
    buf_ = &sbuf_; // default streo
    clock_ = 0;
    timeToNextFrame_ = CLOCKS_PER_FRAME;
}

void SynthWrapper::configure(double sampleRate, int channels) {
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
    synth_init();
}

inline long SynthWrapper::samplesAvailable() {
    if (stereo_) {
        return sbuf_.samples_avail() / 2;
    }
    return mbuf_.samples_avail();
}

void SynthWrapper::writeRegister(gb_addr_t addr, uint8_t data) {
    apu_.write_register(tick(CLOCKS_PER_INSTRUCTION), addr, data);
}

uint8_t SynthWrapper::readRegister(gb_addr_t addr) {
    return apu_.read_register(tick(CLOCKS_PER_INSTRUCTION), addr);
}

blip_time_t SynthWrapper::tick(blip_time_t step) {
    clock_ += step; // TODO: remove clock_
    if (timeToNextFrame_ < step) {
        timeToNextFrame_ += CLOCKS_PER_FRAME;
    }
    timeToNextFrame_ -= step;
//        return clock_;
    return step;
}

gb_time_t SynthWrapper::readSamples(juce::AudioBuffer<float>* out)
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

void SynthWrapper::handleMIDI(juce::MidiBuffer& midiMessages)
{
    MidiEvent e;
    for (const juce::MidiMessageMetadata metadata : midiMessages) {
        e.type = metadata.data[0];
        for (size_t i = 1; i < metadata.numBytes; i++) {
            e.args[i-1] = metadata.data[i];
        }
        synth_handleMidiEvent(&e);
    }
}

void SynthWrapper::reset()
{
    synth_stop();
    sbuf_.clear();
    mbuf_.clear();
    clock_ = 0;
    apu_.reset();
}
