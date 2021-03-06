/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleSamperAudioProcessorEditor::SimpleSamperAudioProcessorEditor (SimpleSamperAudioProcessor& p)
    : AudioProcessorEditor (&p),
    audioProcessor (p), 
    mWaveThumbnail(p), 
    mADSRComponent(p), 
    mFileLoaderListener(nullptr)//,
    //mDragAndDropComponent(p)
{
    addAndMakeVisible(mWaveThumbnail);
    addAndMakeVisible(mADSRComponent);

    //mDragAndDropComponent.setVisible(false);
    //addAndMakeVisible(mDragAndDropComponent);

    mFileLoaderListener.setFunction(std::bind(&SimpleSamperAudioProcessorEditor::FileLoaded, this));
    audioProcessor.getBroadcasterFileLoaded().addActionListener(&mFileLoaderListener);

    startTimerHz(30);

    setSize (700, 300);
}

SimpleSamperAudioProcessorEditor::~SimpleSamperAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void SimpleSamperAudioProcessorEditor::paint (juce::Graphics& g)
{
    //paintZone(g);
    //g.fillAll(Colours::black);
    g.fillAll(mCurrentColour);
}

void SimpleSamperAudioProcessorEditor::resized()
{
    //mDragAndDropComponent.setBounds(0.0f, 0.0f, getWidth(), getHeight());
    mWaveThumbnail.setBoundsRelative(0, 0.25f, 1, 0.5f);
    mADSRComponent.setBoundsRelative(0.6f, 0.6f, 0.4f, 0.4f);
}


void SimpleSamperAudioProcessorEditor::FileLoaded() {
    mWaveThumbnail.setShouldBePainting(true);
    repaint();
}


bool SimpleSamperAudioProcessorEditor::isInterestedInFileDrag(const StringArray& files)
{
    WildcardFileFilter filter(audioProcessor.getFormatManager().getWildcardForAllFormats(), {}, {});

    for (String file : files) {
        if (!filter.isFileSuitable(file))
            return false;
    }
    return true;
}

void SimpleSamperAudioProcessorEditor::filesDropped(const StringArray& files, int x, int y)
{
    for (auto file : files) {
        if (isInterestedInFileDrag(file)) {
            audioProcessor.loadFile(file);
        }
    }
    mCurrentColour = DefaultDragDropColour;
    repaint();
}

void SimpleSamperAudioProcessorEditor::fileDragEnter(const StringArray& files, int x, int y)
{
    if (isInterestedInFileDrag(files))
        mCurrentColour = trueDragDropColour;
    else
        mCurrentColour = falseDragDropColour;
    repaint();
}

void SimpleSamperAudioProcessorEditor::fileDragExit(const StringArray& files)
{
    mCurrentColour = DefaultDragDropColour;
    repaint();
}

void SimpleSamperAudioProcessorEditor::timerCallback() 
{
    repaint();
}


//
//void SimpleSamperAudioProcessorEditor::sliderValueChanged(Slider* slider) 
//{
//    ADSR::Parameters& adsr = audioProcessor.getADSRParams();
//    if (slider == &mAttackSlider) {
//        adsr.attack = mAttackSlider.getValue();
//    }
//    if (slider == &mDecaySlider) {
//        adsr.decay = mDecaySlider.getValue();
//    }
//    if (slider == &mSustainSlider) {
//        adsr.sustain = mSustainSlider.getValue();
//    }
//    if (slider == &mReleaseSlider) {
//        adsr.release = mReleaseSlider.getValue();
//    }
//    audioProcessor.updateADSR();
//}
