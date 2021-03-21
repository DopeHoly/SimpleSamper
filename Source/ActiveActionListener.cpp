/*
  ==============================================================================

    ActiveActionListener.cpp
    Created: 19 Mar 2021 3:06:05am
    Author:  menin

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ActiveActionListener.h"

ActiveActionListener::ActiveActionListener(std::function<void()> function):
    mFunction(function)
{
}

ActiveActionListener::~ActiveActionListener()
{
}

void ActiveActionListener::actionListenerCallback(const String& message)
{
    mFunction();
}
