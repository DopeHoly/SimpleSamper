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
	OSC_Setting(double frequency, double volume, double phase = 0);
	double freq{ 0 };
	double vol{ 0 };
	double phase{ 0 };

	bool static compare(OSC_Setting a, OSC_Setting b);
};