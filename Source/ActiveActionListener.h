/*
  ==============================================================================

    ActiveActionListener.h
    Created: 19 Mar 2021 3:06:05am
    Author:  menin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class ActiveActionListener: public ActionListener
{
public:
    ActiveActionListener(std::function<void()> function);
    ~ActiveActionListener();
    void actionListenerCallback(const String& message) override;
    void setFunction(std::function<void()> function) { mFunction = function; }
private:
    std::function<void()> mFunction;
};

