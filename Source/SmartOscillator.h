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

//template <typename SampleType>
//class SmartOscillator 
//{
//public:
//    using NumericType = typename SampleTypeHelpers::ElementType<SampleType>::Type;
//
//
//    Oscillator() = default;
//
//    Oscillator(const std::function<NumericType(NumericType)>&function)
//    {
//        initialise(function);
//    }
//
//    bool isInitialised() const noexcept { return static_cast<bool> (generator); }
//
//
//private:
//
//    std::function<NumericType(NumericType)> generator;
//    NumericType frequency{ 440.0 };
//    NumericType sampleRate = 48000.0;
//    Phase<NumericType> phase;
//};