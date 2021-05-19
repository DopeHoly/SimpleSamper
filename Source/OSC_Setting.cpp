/*
  ==============================================================================

    OSC_Setting.cpp
    Created: 25 Apr 2021 8:55:05pm
    Author:  menin

  ==============================================================================
*/

#include "OSC_Setting.h"

OSC_Setting::OSC_Setting(double frequency, double volume, double phase, double real, double imag) :
	freq(frequency),
	vol(volume),
	phase(phase),
	re(real),
	im(imag)
{
}

bool OSC_Setting::compare(OSC_Setting a, OSC_Setting b)
{
	return (a.freq < b.freq);
}