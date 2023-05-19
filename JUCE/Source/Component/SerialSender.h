/*
  ==============================================================================

    SerialSender.h
    Created: 10 Mar 2023 3:39:40pm
    Author:  cosma

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <math.h>
#include <iostream>

namespace Serial
{
    class SerialSender : public Component
    {
    public:

        void paint(Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();

            // Just for fun we also draw the level on the interface
            g.setColour(Colours::white.withBrightness(0.4f));
            g.fillRoundedRectangle(bounds, 5.f);

            // Constrain the level values between the limits
            if (levelL < -30.0f)
                levelL = -30.0f;
            if (levelR < -30.0f)
                levelR = -30.0f;
            if (levelL > 5.f)
                levelL = 5.f;
            if (levelR > 5.f)
                levelR = 5.f;

            // Map the level value from between -60dB to +6dB to 12-bit value
            // because that is what is resolution of arduino DAC
            const long maxDACValue = pow(2, DACResolutionBits);
            float dacScaledLevelL = jmap(levelL, -30.0f, +5.f, 1.f, static_cast<float>(maxDACValue));
            float dacScaledLevelR = jmap(levelR, -30.0f, +5.f, 1.f, static_cast<float>(maxDACValue));
            uint8 intScaledLevelL = static_cast<uint8>(dacScaledLevelL);
            uint8 intScaledLevelR = static_cast<uint8>(dacScaledLevelR);

            // Send actual values to serial port
            outStream->write(static_cast<void*>(&intScaledLevelL), 1);
            outStream->write(static_cast<void*>(&intScaledLevelR), 1);
            outStream->write("\x00", 1);

            // FOr the GUI we represent the average of the two levels
            const float level = (levelL + levelR) / 2.f;
            const auto guiScaledLevel = jmap(level, -30.0f, +5.f, 0.f, static_cast<float>(getWidth()));
            g.setColour(Colours::white);
            g.fillRoundedRectangle(bounds.removeFromLeft(guiScaledLevel), 5.f);
        }

        void setLevel(const float valueL, const float valueR) { levelL = valueL; levelR = valueR; }
        void setSerialOutputStream(SerialPortOutputStream* out) { outStream = out;  }

    private:
        float levelL = -60.0f;
        float levelR = -60.0f;
        const int DACResolutionBits = 8;
        SerialPortOutputStream* outStream;
        
    };
}
