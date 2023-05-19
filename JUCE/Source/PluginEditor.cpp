/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BraVuMeterAudioProcessorEditor::BraVuMeterAudioProcessorEditor (BraVuMeterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(sender);
    sender.setSerialOutputStream(audioProcessor.getSerialOutput());
    setSize (400, 300);

    startTimerHz(60);
}

BraVuMeterAudioProcessorEditor::~BraVuMeterAudioProcessorEditor()
{
}

void BraVuMeterAudioProcessorEditor::timerCallback()
{
    sender.setLevel(audioProcessor.getRmsValue(0), audioProcessor.getRmsValue(1));
    sender.repaint();
}

//==============================================================================
void BraVuMeterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::darkgrey);
}

void BraVuMeterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    sender.setBounds(100, 100, 200, 15);
}
