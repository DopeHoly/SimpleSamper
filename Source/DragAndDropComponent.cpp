/*
  ==============================================================================

    DragAndDropComponent.cpp
    Created: 19 Mar 2021 2:46:06am
    Author:  menin

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DragAndDropComponent.h"

//==============================================================================
DragAndDropComponent::DragAndDropComponent(SimpleSamperAudioProcessor& processor)
    :audioProcessor(processor)
{
}

DragAndDropComponent::~DragAndDropComponent()
{
}

void DragAndDropComponent::paint(juce::Graphics& g)
{
    g.fillAll(mCurrentColour);
}

void DragAndDropComponent::resized()
{
}


bool DragAndDropComponent::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
    dragSourceDetails.description)
    WildcardFileFilter filter(audioProcessor.getFormatManager().getWildcardForAllFormats(), {}, {});

    for (String file : files) {
        if (!filter.isFileSuitable(file))
            return false;
    }
    return true;
}

void DragAndDropComponent::itemDropped(const SourceDetails& dragSourceDetails)
{
    for (auto file : files) {
        if (isInterestedInFileDrag(file)) {
            audioProcessor.loadFile(file);
        }
    }
    mCurrentColour = DefaultDragDropColour;
    repaint();
}

void DragAndDropComponent::itemDragEnter(const SourceDetails& dragSourceDetails)
{
    if(isInterestedInFileDrag(files))
        mCurrentColour = trueDragDropColour;
    else
        mCurrentColour = falseDragDropColour;
    repaint();
}

void DragAndDropComponent::itemDragExit(const SourceDetails& dragSourceDetails)
{
    mCurrentColour = DefaultDragDropColour;
    repaint();
}


