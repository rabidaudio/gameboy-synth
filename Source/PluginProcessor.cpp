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
    synth.configure(sampleRate, getTotalNumOutputChannels());
    midiCollector_.reset(sampleRate);
    // TODO: if we switch away from BasicApu and match the sample rates, we'll need to chnage
    // this buffer size. Currently buffer is allocated to 60fps steps
    buffer_ = (blip_sample_t*) malloc(sizeof(blip_sample_t) * sampleRate / 60 * 2);
}

void GameBoySynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    free(buffer_);
    synth.stop();
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
    int totalNumOutputChannels = getTotalNumOutputChannels();
    jassert(totalNumOutputChannels == 2); // TODO: could be mono?

    size_t sampleCount = buffer.getNumSamples();

    // also append any events from the collector
    midiCollector_.removeNextBlockOfMessages(midiMessages, (int) sampleCount);

    synth.process(buffer_, sampleCount, midiMessages);
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        for (int i = 0; i < sampleCount; i++) {
            channelData[i] = ((float) buffer_[i]) / 32768.0;
        }
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
