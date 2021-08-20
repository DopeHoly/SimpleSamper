/*
  ==============================================================================

    SmartOscillator.h
    Created: 22 Apr 2021 6:47:29pm
    Author:  menin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <math.h>
#include "ValuesFunc.h"

using namespace juce::dsp;

template <typename SampleType>
class SmartOscillator 
{
public:
    using NumericType = typename SampleTypeHelpers::ElementType<SampleType>::Type;


    SmartOscillator() = default;
    SmartOscillator(int rootMidiNode)
    {
        midiRootNote = rootMidiNode;
    }

    SmartOscillator(const std::function<NumericType(NumericType)>&function)
    {
        initialise(function);
    }

    bool isInitialised() const noexcept { return static_cast<bool> (generator); }    
    
    void initialise(const std::function<NumericType(NumericType)>& function)
    {
        generator = function;
        //m_frequency = 500;
        m_phase = 0.0;
        m_time = 0.0;
        cnt = 0;
    }


    void prepare(const ProcessSpec& spec,
        ValuesFunc& frequency,
        ValuesFunc& gain,
        double phase
    ) noexcept
    {
        sampleRate = static_cast<NumericType> (spec.sampleRate);
        m_deltaTime = 1 / sampleRate;
        m_frequency = frequency;
        m_gain = gain;
        m_phase = phase;
        reset();
    }

    void reset() noexcept
    {
        cnt = 0;
        m_time = 0.0;
    }

    SampleType JUCE_VECTOR_CALLTYPE processSample(int& midiNoteNumber) noexcept
    {
        jassert(isInitialised());
        auto freq = m_frequency.GetValue(m_time);
        freq *= std::pow(2.0, (midiNoteNumber - midiRootNote) / 12.0);
        auto value = generator(2.0 * double_Pi * freq * m_time + m_phase);
        value *= m_gain.GetValue(m_time);

        m_time += m_deltaTime;
        //++cnt;
        //if (cnt == sampleRate) {
        //    cnt = 0;
        //    m_time = 0.0;
        //}
        return value;
    }

    NumericType GetPhase() { return m_phase; };
    void SetPhase(NumericType value) { m_phase = value; };

    ValuesFunc& GetFrequency(){ return m_frequency; };
    ValuesFunc& GetGain(){ return m_gain; };
   

private:
    ValuesFunc m_frequency;
    ValuesFunc m_gain;
    std::function<NumericType(NumericType)> generator;
    NumericType m_phase{ 0 };
    NumericType m_time{ 0 };
    int cnt{ 0 };
    NumericType m_deltaTime;
    NumericType sampleRate = 48000.0;
    int midiRootNote{ 60 };
};