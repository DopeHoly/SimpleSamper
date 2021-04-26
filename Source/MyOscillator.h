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
    MyOscillator(const std::function<NumericType(NumericType)>& function,
        size_t lookupTableNumPoints = 0)
    {
        initialise(function, lookupTableNumPoints);
    }

    /** Returns true if the Oscillator has been initialised. */
    bool isInitialised() const noexcept { return static_cast<bool> (generator); }

    /** Initialises the oscillator with a waveform. */
    void initialise(const std::function<NumericType(NumericType)>& function,
        size_t lookupTableNumPoints = 0)
    {
        if (lookupTableNumPoints != 0)
        {
            auto* table = new LookupTableTransform<NumericType>(function,
                -MathConstants<NumericType>::pi,
                MathConstants<NumericType>::pi,
                lookupTableNumPoints);

            lookupTable.reset(table);
            generator = [table](NumericType x) { return (*table) (x); };
        }
        else
        {
            generator = function;
        }
    }

    //==============================================================================
    /** Sets the frequency of the oscillator. */
    void setFrequency(NumericType newFrequency, bool force = false) noexcept
    {
        if (force)
        {
            frequency.setCurrentAndTargetValue(newFrequency);
            return;
        }

        frequency.setTargetValue(newFrequency);
    }

    /** Returns the current frequency of the oscillator. */
    NumericType getFrequency() const noexcept { return frequency.getTargetValue(); }

    //==============================================================================
    /** Called before processing starts. */
    void prepare(const ProcessSpec& spec) noexcept
    {
        sampleRate = static_cast<NumericType> (spec.sampleRate);
        rampBuffer.resize((int)spec.maximumBlockSize);

        reset();
    }

    /** Resets the internal state of the oscillator */
    void reset() noexcept
    {
        phase.reset();

        if (sampleRate > 0)
            frequency.reset(sampleRate, 0.05);
    }

    //==============================================================================
    /** Returns the result of processing a single sample. */
    SampleType JUCE_VECTOR_CALLTYPE processSample(SampleType input) noexcept
    {
        jassert(isInitialised());
        auto increment = MathConstants<NumericType>::twoPi * frequency.getNextValue() / sampleRate;
        return input + generator(phase.advance(increment) - MathConstants<NumericType>::pi);
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
        jassert(isInitialised());
        auto&& outBlock = context.getOutputBlock();
        auto&& inBlock = context.getInputBlock();

        // this is an output-only processor
        jassert(outBlock.getNumSamples() <= static_cast<size_t> (rampBuffer.size()));

        auto len = outBlock.getNumSamples();
        auto numChannels = outBlock.getNumChannels();
        auto inputChannels = inBlock.getNumChannels();
        auto baseIncrement = MathConstants<NumericType>::twoPi / sampleRate;

        if (context.isBypassed)
            context.getOutputBlock().clear();

        if (frequency.isSmoothing())
        {
            auto* buffer = rampBuffer.getRawDataPointer();

            for (size_t i = 0; i < len; ++i)
                buffer[i] = phase.advance(baseIncrement * frequency.getNextValue())
                - MathConstants<NumericType>::pi;

            if (!context.isBypassed)
            {
                size_t ch;

                if (context.usesSeparateInputAndOutputBlocks())
                {
                    for (ch = 0; ch < jmin(numChannels, inputChannels); ++ch)
                    {
                        auto* dst = outBlock.getChannelPointer(ch);
                        auto* src = inBlock.getChannelPointer(ch);

                        for (size_t i = 0; i < len; ++i)
                            dst[i] = src[i] + generator(buffer[i]);
                    }
                }
                else
                {
                    for (ch = 0; ch < jmin(numChannels, inputChannels); ++ch)
                    {
                        auto* dst = outBlock.getChannelPointer(ch);

                        for (size_t i = 0; i < len; ++i)
                            dst[i] += generator(buffer[i]);
                    }
                }

                for (; ch < numChannels; ++ch)
                {
                    auto* dst = outBlock.getChannelPointer(ch);

                    for (size_t i = 0; i < len; ++i)
                        dst[i] = generator(buffer[i]);
                }
            }
        }
        else
        {
            auto freq = baseIncrement * frequency.getNextValue();
            auto p = phase;

            if (context.isBypassed)
            {
                frequency.skip(static_cast<int> (len));
                p.advance(freq * static_cast<NumericType> (len));
            }
            else
            {
                size_t ch;

                if (context.usesSeparateInputAndOutputBlocks())
                {
                    for (ch = 0; ch < jmin(numChannels, inputChannels); ++ch)
                    {
                        p = phase;
                        auto* dst = outBlock.getChannelPointer(ch);
                        auto* src = inBlock.getChannelPointer(ch);

                        for (size_t i = 0; i < len; ++i)
                            dst[i] = src[i] + generator(p.advance(freq) - MathConstants<NumericType>::pi);
                    }
                }
                else
                {
                    for (ch = 0; ch < jmin(numChannels, inputChannels); ++ch)
                    {
                        p = phase;
                        auto* dst = outBlock.getChannelPointer(ch);

                        for (size_t i = 0; i < len; ++i)
                            dst[i] += generator(p.advance(freq) - MathConstants<NumericType>::pi);
                    }
                }

                for (; ch < numChannels; ++ch)
                {
                    p = phase;
                    auto* dst = outBlock.getChannelPointer(ch);

                    for (size_t i = 0; i < len; ++i)
                        dst[i] = generator(p.advance(freq) - MathConstants<NumericType>::pi);
                }
            }

            phase = p;
        }
    }

    NumericType GetPhase() { return phase.phase; };
    void SetPhase(NumericType value) { phase.phase = value; };


private:
    //==============================================================================
    std::function<NumericType(NumericType)> generator;
    std::unique_ptr<LookupTableTransform<NumericType>> lookupTable;
    Array<NumericType> rampBuffer;
    SmoothedValue<NumericType> frequency{ static_cast<NumericType> (440.0) };
    NumericType sampleRate = 48000.0;
    Phase<NumericType> phase;
};