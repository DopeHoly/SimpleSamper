/*
  ==============================================================================

    SmartOscillator.cpp
    Created: 22 Apr 2021 6:47:29pm
    Author:  menin

  ==============================================================================
*/

#include "SmartOscillator.h"

//template<typename SampleType>
//SmartOscillator<SampleType>::SmartOscillator(int rootMidiNode)
//{
//    midiRootNote = rootMidiNode;
//}
//
//template<typename T>
//void SmartOscillator<T>::initialise(const std::function<NumericType(NumericType)>& function)
//{
//    generator = function;
//    //m_frequency = 500;
//    m_phase = 0.0;
//    m_time = 0.0;
//    cnt = 0;
//}

//template<typename T>
//void SmartOscillator<T> :: prepare(const ProcessSpec& spec,
//    ValuesFunc frequency,
//    ValuesFunc gain,
//    double phase
//) noexcept
//{
//    sampleRate = static_cast<NumericType> (spec.sampleRate);
//    m_deltaTime = 1 / sampleRate;
//    m_frequency = frequency;
//    m_gain = gain;
//    m_phase = phase;
//    reset();
//}


//template<typename T>
//void SmartOscillator<T> ::reset() noexcept
//{
//    cnt = 0;
//    m_time = 0.0;
//}

//template<typename T>
//T JUCE_VECTOR_CALLTYPE SmartOscillator<T> :: processSample(int& midiNoteNumber) noexcept
//{
//    jassert(isInitialised());
//    auto freq = m_frequency.GetValue(m_time);
//    freq *= std::pow(2.0, (midiNoteNumber - midiRootNote) / 12.0);
//    auto value = generator(2.0 * double_Pi * freq * m_time + m_phase);
//    value *= m_gain.GetValue(m_time);
//
//    m_time += m_deltaTime;
//    //++cnt;
//    //if (cnt == sampleRate) {
//    //    cnt = 0;
//    //    m_time = 0.0;
//    //}
//    return value;
//}