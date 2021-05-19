/*
  ==============================================================================

    OSC_Setting.h
    Created: 25 Apr 2021 8:55:05pm
    Author:  menin

  ==============================================================================
*/

#pragma once
class OSC_Setting
{
public:
	OSC_Setting(double frequency, double volume, double phase = 0, double real = 0, double imag = 0);
	double freq{ 0 };
	double vol{ 0 };
	double phase{ 0 };
	double re{ 0 };
	double im{ 0 };
	int marker{ -1 };

	bool static compare(OSC_Setting a, OSC_Setting b);
};