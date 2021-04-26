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
    mFileLoaderListener(nullptr),
    mSAComponent()
    //mDragAndDropComponent(p)
{
    addAndMakeVisible(mWaveThumbnail);
    addAndMakeVisible(mADSRComponent);
    addAndMakeVisible(mSAComponent);

    //mDragAndDropComponent.setVisible(false);
    //addAndMakeVisible(mDragAndDropComponent);

    mFileLoaderListener.setFunction(std::bind(&SimpleSamperAudioProcessorEditor::FileLoaded, this));
    audioProcessor.getBroadcasterFileLoaded().addActionListener(&mFileLoaderListener);

    //audioProcessor.loadFile("C:\\ProgramData\\Ableton\\Live 10 Suite\\Resources\\Core Library\\Samples\\Grand Piano\\GrandPiano-A#-1-p.aif");
    audioProcessor.loadFile("D:\\YandexDisk\\Ableton Projects\\A440.wav");
    //audioProcessor.loadFile("D:\\YandexDisk\\Ableton Projects\\Chords.wav");
    startTimerHz(30);

    setSize (700, 600);
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
    mWaveThumbnail.setBoundsRelative(0, 0, 1, 0.5f);
    mADSRComponent.setBoundsRelative(0.6f, 0.3f, 0.4f, 0.4f);
    mSAComponent.setBoundsRelative(0, 0.5f, 1, 0.5f);
}


void SimpleSamperAudioProcessorEditor::FileLoaded() {
    mWaveThumbnail.setShouldBePainting(true);
    mSAComponent.SetSample(audioProcessor.getWaveForm());
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
