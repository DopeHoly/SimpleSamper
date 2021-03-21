/*
  ==============================================================================

    DragAndDropComponent.h
    Created: 19 Mar 2021 2:46:05am
    Author:  menin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class DragAndDropComponent  : public Component,
                              public DragAndDropTarget
{
public:
    DragAndDropComponent(SimpleSamperAudioProcessor&);
    ~DragAndDropComponent() override;

    void paint (juce::Graphics&);
    void resized() override;

    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;


private:
    SimpleSamperAudioProcessor& audioProcessor;

    Colour DefaultDragDropColour{ Colours::transparentBlack };
    Colour trueDragDropColour{ Colours::green.withAlpha(0.5f) };
    Colour falseDragDropColour{ Colours::red.withAlpha(0.5f) };
    Colour mCurrentColour{ Colours::transparentBlack };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DragAndDropComponent)
};
