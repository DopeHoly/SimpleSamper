/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "WaveThumbnail.h"
#include "ADSRComponent.h"
#include "ActiveActionListener.h"
#include "DragAndDropComponent.h"

//==============================================================================
/**
*/
class SimpleSamperAudioProcessorEditor  : public AudioProcessorEditor,
                                          public FileDragAndDropTarget,
                                          public Timer

{
public:
    SimpleSamperAudioProcessorEditor (SimpleSamperAudioProcessor&);
    ~SimpleSamperAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    //FileDragAndDropTarget
    bool isInterestedInFileDrag(const StringArray& files) override;
    void fileDragEnter(const StringArray& files, int x, int y) override;
    void fileDragExit(const StringArray& files) override;
    void filesDropped(const StringArray& files, int x, int y) override;

    //Timer
    void timerCallback() override;

private:

    ActiveActionListener mFileLoaderListener;

    WaveThumbnail mWaveThumbnail;
    ADSRComponent mADSRComponent;

    SimpleSamperAudioProcessor& audioProcessor;


    Colour DefaultDragDropColour{ Colours::transparentBlack };
    Colour trueDragDropColour{ Colours::lime};
    Colour falseDragDropColour{ Colours::red.withAlpha(0.5f) };
    Colour mCurrentColour{ Colours::transparentBlack };

    void FileLoaded();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleSamperAudioProcessorEditor)
};
