/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GameBoySynthAudioProcessor::GameBoySynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

GameBoySynthAudioProcessor::~GameBoySynthAudioProcessor()
{
}

//==============================================================================
const juce::String GameBoySynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GameBoySynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GameBoySynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GameBoySynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GameBoySynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GameBoySynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GameBoySynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GameBoySynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GameBoySynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void GameBoySynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GameBoySynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    blargg_err_t res = apu_.set_sample_rate((long) sampleRate);
    jassert(res == blargg_success);
    apu_.write_register( 0xff26, 0x80 );
    apu_.write_register( 0xff25, 0x11 );
    juce::Logger::writeToLog("ready");

//    eventManager_.init(&apu_);
    // TODO: if we switch away from BasicApu and match the sample rates, we'll need to chnage
    // this buffer size. Currently buffer is allocated to 60fps steps
//    buffer_ = (blip_sample_t*) malloc(sizeof(blip_sample_t) * sampleRate / 60 * 2);

    // TODO: testing
//    GBMidiConfig c;
//    c.enabled = true;
//    c.channel = 0;
//    c.transpose = 0;
//    c.voice = 0;
//    eventManager_.setConfig(0, c);
}

void GameBoySynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
//    free(buffer_);
    // TODO: stop APU, clear BlipBuffer, reset EventManager
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GameBoySynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GameBoySynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
//    auto totalNumInputChannels  = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
//    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
//        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

//    if (rand() % 60 == 0) {
//        static uint8_t data[3] = { 0x80, 0, 0x7F };
//        data[1] = rand() % 0x7F;
//        eventManager_.handleMIDIEvent(0, (const uint8_t*) data);
//    }

    unsigned int sampleCount = buffer.getNumSamples();
    // Generate 1/60 second of sound into APU's sample buffer
    while (apu_.samples_avail() < sampleCount) {
        static int delay;
        if ( --delay <= 0 )
        {
            delay = 12;

            // Start a new random tone
//            int chan = rand() & 0x11;
//            apu_.write_register( 0xff26, 0x80 );
//            apu_.write_register( 0xff25, chan ? chan : 0x11 );
            apu_.write_register( 0xff11, 0x80 );
            int freq = (rand() & 0x3ff) + 0x300;
            apu_.write_register( 0xff13, freq & 0xff );
            apu_.write_register( 0xff12, 0xf1 );
            apu_.write_register( 0xff14, (freq >> 8) | 0x80 );
        }
        apu_.end_frame();
    }

    // TODO: avoid doublebuffering with custom BlipBuffer which converts to float
    //  on write
    long count = apu_.read_samples(buffer_, sampleCount);
    jassert(count == sampleCount);

    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        for (int i = 0; i < sampleCount; i++) {
            channelData[i] = ((float) buffer_[i]) / 32768.0;
        }

//        for (int i = 0; i < sampleCount; i++) {
//            channelData[i] = sinf((float) i / (float)(sampleCount / 4) * 2.0 * 3.1415) / 2;
//        }
//        for (int i = 0; i < sampleCount; i++) {
//            channelData[i] = ((float) buffer_[(i*totalNumOutputChannels) + channel]) / 32768.0;
//        }
//        juce::AudioDataConverters::convertInt16BEToFloat(buffer_, channelData, sampleCount);
    }
}

//==============================================================================
bool GameBoySynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GameBoySynthAudioProcessor::createEditor()
{
    return new GameBoySynthAudioProcessorEditor (*this);
}

//==============================================================================
void GameBoySynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void GameBoySynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GameBoySynthAudioProcessor();
}
