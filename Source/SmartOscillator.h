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

    SmartOscillator(const std::function<NumericType(NumericType)>&function)
    {
        initialise(function);
    }

    bool isInitialised() const noexcept { return static_cast<bool> (generator); }    
    
    void initialise(const std::function<NumericType(NumericType)>& function)
    {
        generator = function;
        m_frequency = 500;
        m_phase = 0.0;
        m_time = 0.0;
        cnt = 0;
    }

    void prepare(const ProcessSpec& spec,
        ValuesFunc frequency,
        ValuesFunc gain
        ) noexcept
    {
        sampleRate = static_cast<NumericType> (spec.sampleRate);
        m_deltaTime = 1 / sampleRate;
        m_frequency = frequency;
        m_gain = gain;
        reset();
    }

    void reset() noexcept
    {
        cnt = 0;
        m_time = 0.0;
    }

    SampleType JUCE_VECTOR_CALLTYPE processSample() noexcept
    {
        jassert(isInitialised());
        auto value = generator(2.0 * double_Pi * m_frequency.GetValue(m_time) * m_time + m_phase);
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

private:

    std::function<NumericType(NumericType)> generator;
    ValuesFunc m_frequency;
    ValuesFunc m_gain;
    NumericType m_phase;
    NumericType m_time;
    int cnt;
    NumericType m_deltaTime;
    NumericType sampleRate = 48000.0;
};