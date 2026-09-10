#include "SensorArray.h"
#include "hardware/gpio.h"

SensorArray::SensorArray(uint frontPin_,
                          uint leftInnerPin_, uint leftMiddlePin_, uint leftOuterPin_,
                          uint rightInnerPin_, uint rightMiddlePin_, uint rightOuterPin_)
    : frontPin(frontPin_),
      leftInnerPin(leftInnerPin_), leftMiddlePin(leftMiddlePin_), leftOuterPin(leftOuterPin_),
      rightInnerPin(rightInnerPin_), rightMiddlePin(rightMiddlePin_), rightOuterPin(rightOuterPin_),
      front(false),
      leftInner(false), leftMiddle(false), leftOuter(false),
      rightInner(false), rightMiddle(false), rightOuter(false)
{
    const uint pins[] = {frontPin, leftInnerPin, leftMiddlePin, leftOuterPin,
                          rightInnerPin, rightMiddlePin, rightOuterPin};
    for (uint pin : pins)
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
    }
}

void SensorArray::Read()
{
    front = gpio_get(frontPin);
    leftInner = gpio_get(leftInnerPin);
    leftMiddle = gpio_get(leftMiddlePin);
    leftOuter = gpio_get(leftOuterPin);
    rightInner = gpio_get(rightInnerPin);
    rightMiddle = gpio_get(rightMiddlePin);
    rightOuter = gpio_get(rightOuterPin);
}

MouseState SensorArray::GetDriftState() const
{
    // Which of the 3 sensors on a side lights up as the mouse nears/leaves that wall depends on
    // the array's physical spacing, which hasn't been measured on the real chassis yet - this
    // is a placeholder to calibrate once, not a finished algorithm. Assumed for now: inner
    // (closest to the mouse's body) triggering means we've drifted towards that wall, outer
    // means we've drifted away from it, middle-only means centered against it.
    if (LeftWallPresent())
    {
        if (leftInner) return DriftLeft;
        if (leftOuter) return DriftRight;
        if (leftMiddle) return Centered;
    }

    if (RightWallPresent())
    {
        // mirror image of the left side
        if (rightInner) return DriftRight;
        if (rightOuter) return DriftLeft;
        if (rightMiddle) return Centered;
    }

    return MouseError; // no wall on either side to reference, or an unreadable pattern
}
