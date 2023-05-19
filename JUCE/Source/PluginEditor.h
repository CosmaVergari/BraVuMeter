/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Component/SerialSender.h"

/*
* The Timer inheritance comes in order to perform the actual painting of the component
* and the sending of the information to the external VU at regular intervales
*/
class BraVuMeterAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    BraVuMeterAudioProcessorEditor (BraVuMeterAudioProcessor&);
    ~BraVuMeterAudioProcessorEditor() override;

    //==============================================================================
    void timerCallback() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BraVuMeterAudioProcessor& audioProcessor;

    Serial::SerialSender sender;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BraVuMeterAudioProcessorEditor)
};
