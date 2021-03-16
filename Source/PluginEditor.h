/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SimpleSamperAudioProcessorEditor  : public AudioProcessorEditor,
                                          public FileDragAndDropTarget
{
public:
    SimpleSamperAudioProcessorEditor (SimpleSamperAudioProcessor&);
    ~SimpleSamperAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    bool isInterestedInFileDrag(const StringArray& files) override;
    void filesDropped(const StringArray& files, int x, int y) override;

private:
    TextButton mLoadButton{ "Load" };
    Label mLabel;

    SimpleSamperAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleSamperAudioProcessorEditor)
};
