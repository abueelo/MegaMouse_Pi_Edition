#pragma once

#include "pico/stdlib.h"
#include "EnumHelper.h"

// Reads the mouse's 7 digital IR sensors (all active-high: HIGH means a wall/reflection is
// detected) and turns the raw pins into the two things the rest of the code needs - whether a
// wall is present on a side, and, while a wall is present to reference, whether the mouse is
// centered or drifting towards/away from it.
class SensorArray
{
private:
    uint frontPin;
    uint leftInnerPin, leftMiddlePin, leftOuterPin;
    uint rightInnerPin, rightMiddlePin, rightOuterPin;

    bool front;
    bool leftInner, leftMiddle, leftOuter;
    bool rightInner, rightMiddle, rightOuter;

public:
    SensorArray(uint frontPin_,
                uint leftInnerPin_, uint leftMiddlePin_, uint leftOuterPin_,
                uint rightInnerPin_, uint rightMiddlePin_, uint rightOuterPin_);

    void Read(); // samples all 7 GPIOs - the only place this class touches hardware

    bool FrontWallPresent() const { return front; }
    bool LeftWallPresent() const { return leftInner || leftMiddle || leftOuter; }
    bool RightWallPresent() const { return rightInner || rightMiddle || rightOuter; }

    // Centered/DriftLeft/DriftRight/MouseError, from the last Read(). MouseError covers both
    // "no wall on either side to reference" and an unreadable/contradictory sensor pattern.
    MouseState GetDriftState() const;
};
