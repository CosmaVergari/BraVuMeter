/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BraVuMeterAudioProcessor::BraVuMeterAudioProcessor()
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
    // OPEN the serial port
    // The opening is done here because there is only one instance of AudioProcessor,
    // instead the SerialSender constructor is called once every time the UI is open,
    // the same applies for the destructor at closing of UI
    serialPort = new SerialPort([](juce::String, juce::String) {});
    bool opened = serialPort->open(kSerialPortName);

    if (opened)
    {
        SerialPortConfig serialPortConfig;
        serialPort->getConfig(serialPortConfig);
        serialPortConfig.bps = 19200;
        serialPortConfig.databits = 8;
        serialPortConfig.parity = SerialPortConfig::SERIALPORT_PARITY_NONE;
        serialPortConfig.stopbits = SerialPortConfig::STOPBITS_1;
        serialPort->setConfig(serialPortConfig);

        juce::Logger::outputDebugString("Serial port: " + kSerialPortName + " opened");

        //create streams for reading/writing
        serialPortOutput = new SerialPortOutputStream(serialPort);
        serialPortInput = new SerialPortInputStream(serialPort);
    }
    else
    {
        // report error
        juce::Logger::outputDebugString("Unable to open serial port:" + kSerialPortName);
    }
}

BraVuMeterAudioProcessor::~BraVuMeterAudioProcessor()
{
    if (serialPort != nullptr)
    {
        serialPort->close();
        free(serialPort);
    }
}

//==============================================================================
const juce::String BraVuMeterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BraVuMeterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BraVuMeterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BraVuMeterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BraVuMeterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BraVuMeterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BraVuMeterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BraVuMeterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BraVuMeterAudioProcessor::getProgramName (int index)
{
    return {};
}

void BraVuMeterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BraVuMeterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    rmsLevelLeft.reset(sampleRate, 0.5);
    rmsLevelRight.reset(sampleRate, 0.5);

    rmsLevelLeft.setCurrentAndTargetValue(-100.f);
    rmsLevelRight.setCurrentAndTargetValue(-100.f);
}

void BraVuMeterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BraVuMeterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BraVuMeterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // The smoothing is done by ticking explicitly the unit of measure defined in reset() method of 
    // LinearSmoothed type. The unit of measure defined for us here is samples.
    // However we are processing audio as blocks of samples so we advance by the number of samples
    // processed in each block
    rmsLevelLeft.skip(buffer.getNumSamples());
    rmsLevelRight.skip(buffer.getNumSamples());

    {
        // Leaving RMS samples average on the number of smaples of the block because it is
        // good enough for now
        const auto value = Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));

        // Smoothing implemented as decay
        if (value < rmsLevelLeft.getCurrentValue())
            // Apply smoothing if new value is less than old value
            rmsLevelLeft.setTargetValue(value);
        else
            // Do not apply smoothing otherwise (to have good up transient response)
            rmsLevelLeft.setCurrentAndTargetValue(value);
    }

    {
        // Same for right channel
		const auto value = Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));
        if (value < rmsLevelRight.getCurrentValue())
            rmsLevelRight.setTargetValue(value);
        else
            rmsLevelRight.setCurrentAndTargetValue(value);
    }
    

}

//==============================================================================
bool BraVuMeterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BraVuMeterAudioProcessor::createEditor()
{
    return new BraVuMeterAudioProcessorEditor (*this);
}

//==============================================================================
void BraVuMeterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BraVuMeterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

float BraVuMeterAudioProcessor::getRmsValue(const int channel) const
{
    // We accept only stereo
    jassert(channel == 0 || channel == 1);

    if (channel == 0)
        return rmsLevelLeft.getCurrentValue();
    if (channel == 1)
        return rmsLevelRight.getCurrentValue();
    return 0.f;
}

SerialPortOutputStream* BraVuMeterAudioProcessor::getSerialOutput() const
{
    return serialPortOutput;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BraVuMeterAudioProcessor();
}
