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
    mStartCalculationListener(nullptr)
    //mSAComponent()
    //mDragAndDropComponent(p)
{
    //init buttons actions
    mLoadButton.onClick = [&]() {audioProcessor.loadFileFromOpenFileDialog(); };
    mStartCalculationButton.onClick = [&]() {audioProcessor.StartCalculation(); };
    mPlaySampleButton.onClick = [&]() {audioProcessor.PlaySample(); };

    mStartCalculationButton.setEnabled(false);
    mPlaySampleButton.setEnabled(false);


    addAndMakeVisible(mLoadButton);
    addAndMakeVisible(mStartCalculationButton);
    addAndMakeVisible(mPlaySampleButton);

    addAndMakeVisible(mWaveThumbnail);
    //addAndMakeVisible(mADSRComponent);
    addAndMakeVisible(mSpectrogramCWT);
    //addAndMakeVisible(mSAComponent);

    //mDragAndDropComponent.setVisible(false);
    //addAndMakeVisible(mDragAndDropComponent);

    mFileLoaderListener.setFunction(std::bind(&SimpleSamperAudioProcessorEditor::FileLoaded, this));
    mStartCalculationListener.setFunction(std::bind(&SimpleSamperAudioProcessorEditor::StartCalculation, this));
    audioProcessor.getBroadcasterFileLoaded().addActionListener(&mFileLoaderListener);
    audioProcessor.getBroadcasterStartCalculationLoaded().addActionListener(&mStartCalculationListener);

    //audioProcessor.loadFile("C:\\ProgramData\\Ableton\\Live 10 Suite\\Resources\\Core Library\\Samples\\Grand Piano\\GrandPiano-A#-1-p.aif");
    //audioProcessor.loadFile("D:\\YandexDisk\\Ableton Projects\\A440.wav");
    //audioProcessor.loadFile("D:\\YandexDisk\\Ableton Projects\\2kWave.wav");
    //audioProcessor.loadFile("D:\\YandexDisk\\5kWave.wav");
    //audioProcessor.loadFile("D:\\YandexDisk\\Ableton Projects\\Chords.wav");
    //audioProcessor.loadFile("D:\\YandexDisk\\Ableton Projects\\saw.wav");
    startTimerHz(30);

    setSize (1500, 1000);
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
    mLoadButton.setBoundsRelative(0, 0, 0.33f, 0.05f);
    mPlaySampleButton.setBoundsRelative(0.335f, 0, 0.33f, 0.05f);
    mStartCalculationButton.setBoundsRelative(0.665f, 0, 0.33f, 0.05f);

    mWaveThumbnail.setBoundsRelative(0, 0.05f, 1, 0.25f);
    mSpectrogramCWT.setBoundsRelative(0, 0.3f, 1, 0.7f);
    //mADSRComponent.setBoundsRelative(0.6f, 0.3f, 0.4f, 0.4f);
    //mSAComponent.setBoundsRelative(0, 0.5f, 1, 0.5f);
}


void SimpleSamperAudioProcessorEditor::FileLoaded() {
    mSpectrogramCWT.Clear();
    mWaveThumbnail.setShouldBePainting(true);
    mStartCalculationButton.setEnabled(true);
    mPlaySampleButton.setEnabled(true);
    repaint();
}

void SimpleSamperAudioProcessorEditor::StartCalculation() {
    mSpectrogramCWT.SetSpectre(&audioProcessor.GetSpectre());

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
