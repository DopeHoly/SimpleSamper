/*
  ==============================================================================

    MyOscillator.h
    Created: 25 Apr 2021 10:01:06pm
    Author:  menin

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

using namespace juce::dsp;

template <typename SampleType>
class MyOscillator
{
public:
    /** The NumericType is the underlying primitive type used by the SampleType (which
        could be either a primitive or vector)
    */
    using NumericType = typename SampleTypeHelpers::ElementType<SampleType>::Type;

    /** Creates an uninitialised oscillator. Call initialise before first use. */
    MyOscillator() = default;

    /** Creates an oscillator with a periodic input function (-pi..pi).

        If lookup table is not zero, then the function will be approximated
        with a lookup table.
    */
    MyOscillator(const std::function<NumericType(NumericType)>& function)
    {
        initialise(function);
    }

    /** Returns true if the Oscillator has been initialised. */
    bool isInitialised() const noexcept { return static_cast<bool> (generator); }

    /** Initialises the oscillator with a waveform. */
    void initialise(const std::function<NumericType(NumericType)>& function)
    {
        generator = function;
        m_frequency = 500;
        m_phase = 0.0;
        m_time = 0.0;
        cnt = 0;
    }

    //==============================================================================
    /** Sets the frequency of the oscillator. */
    void setFrequency(NumericType newFrequency) noexcept
    {
        m_frequency = newFrequency;
    }

    /** Returns the current frequency of the oscillator. */
    NumericType getFrequency() const noexcept { return m_frequency; }

    //==============================================================================
    /** Called before processing starts. */
    void prepare(const ProcessSpec& spec) noexcept
    {
        sampleRate = static_cast<NumericType> (spec.sampleRate);
        m_deltaTime = 1 / sampleRate;

        reset();
    }

    /** Resets the internal state of the oscillator */
    void reset() noexcept
    {
        cnt = 0;
        m_time = 0.0;
    }

    //==============================================================================
    /** Returns the result of processing a single sample. */
    SampleType JUCE_VECTOR_CALLTYPE processSample(SampleType input) noexcept
    {
        jassert(isInitialised());
        //auto increment = MathConstants<NumericType>::twoPi * frequency.getNextValue() / sampleRate;
        auto value = input + generator(2.0 * double_Pi * m_frequency * m_time + m_phase);
        m_time += m_deltaTime;
        ++cnt;
        if (cnt == sampleRate) {
            cnt = 0;
            m_time = 0.0;
        }
        return value;
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
        //jassert(isInitialised());
        //auto&& outBlock = context.getOutputBlock();
        //auto&& inBlock = context.getInputBlock();

        //// this is an output-only processor
        ////jassert(outBlock.getNumSamples() <= static_cast<size_t> (rampBuffer.size()));

        //auto len = outBlock.getNumSamples();
        //auto numChannels = outBlock.getNumChannels();
        //auto inputChannels = inBlock.getNumChannels();
        //auto baseIncrement = MathConstants<NumericType>::twoPi / sampleRate;

        //if (context.isBypassed)
        //    context.getOutputBlock().clear();

        //if (frequency.isSmoothing())
        //{
        //    auto* buffer = rampBuffer.getRawDataPointer();

        //    for (size_t i = 0; i < len; ++i)
        //        buffer[i] = phase.advance(baseIncrement * frequency.getNextValue())
        //        - MathConstants<NumericType>::pi;

        //    if (!context.isBypassed)
        //    {
        //        size_t ch;

        //        if (context.usesSeparateInputAndOutputBlocks())
        //        {
        //            for (ch = 0; ch < jmin(numChannels, inputChannels); ++ch)
        //            {
        //                auto* dst = outBlock.getChannelPointer(ch);
        //                auto* src = inBlock.getChannelPointer(ch);

        //                for (size_t i = 0; i < len; ++i)
        //                    dst[i] = src[i] + generator(buffer[i]);
        //            }
        //        }
        //        else
        //        {
        //            for (ch = 0; ch < jmin(numChannels, inputChannels); ++ch)
        //            {
        //                auto* dst = outBlock.getChannelPointer(ch);

        //                for (size_t i = 0; i < len; ++i)
        //                    dst[i] += generator(buffer[i]);
        //            }
        //        }

        //        for (; ch < numChannels; ++ch)
        //        {
        //            auto* dst = outBlock.getChannelPointer(ch);

        //            for (size_t i = 0; i < len; ++i)
        //                dst[i] = generator(buffer[i]);
        //        }
        //    }
        //}
        //else
        //{
        //    auto freq = baseIncrement * frequency.getNextValue();
        //    auto p = phase;

        //    if (context.isBypassed)
        //    {
        //        frequency.skip(static_cast<int> (len));
        //        p.advance(freq * static_cast<NumericType> (len));
        //    }
        //    else
        //    {
        //        size_t ch;

        //        if (context.usesSeparateInputAndOutputBlocks())
        //        {
        //            for (ch = 0; ch < jmin(numChannels, inputChannels); ++ch)
        //            {
        //                p = phase;
        //                auto* dst = outBlock.getChannelPointer(ch);
        //                auto* src = inBlock.getChannelPointer(ch);

        //                for (size_t i = 0; i < len; ++i)
        //                    dst[i] = src[i] + generator(p.advance(freq) - MathConstants<NumericType>::pi);
        //            }
        //        }
        //        else
        //        {
        //            for (ch = 0; ch < jmin(numChannels, inputChannels); ++ch)
        //            {
        //                p = phase;
        //                auto* dst = outBlock.getChannelPointer(ch);

        //                for (size_t i = 0; i < len; ++i)
        //                    dst[i] += generator(p.advance(freq) - MathConstants<NumericType>::pi);
        //            }
        //        }

        //        for (; ch < numChannels; ++ch)
        //        {
        //            p = phase;
        //            auto* dst = outBlock.getChannelPointer(ch);

        //            for (size_t i = 0; i < len; ++i)
        //                dst[i] = generator(p.advance(freq) - MathConstants<NumericType>::pi);
        //        }
        //    }

        //    phase = p;
        //}
    }

    NumericType GetPhase() { return m_phase; };
    void SetPhase(NumericType value) { m_phase = value; };


private:
    //==============================================================================
    std::function<NumericType(NumericType)> generator;
    NumericType m_frequency;
    NumericType m_phase;
    NumericType m_time;
    int cnt;
    NumericType m_deltaTime;
    NumericType sampleRate = 48000.0;
};