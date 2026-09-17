#pragma once

#include "pico/stdlib.h"

// Every physical pin the mouse uses, gathered in one place. All values below are placeholders -
// the real wiring isn't finalised yet, so update them here once it is and nothing else needs to
// change.

// Drivetrain - see Motor for what each pin does
constexpr uint LeftMotorStepPin = 3;
constexpr uint LeftMotorDirPin = 2;
constexpr uint LeftMotorMsPin = 19;

constexpr uint RightMotorStepPin = 1;
constexpr uint RightMotorDirPin = 0;
constexpr uint RightMotorMsPin = 20;

// IR sensors - all digital, active-high (HIGH = wall/reflection detected)
constexpr uint FrontSensorPin = 10;

// Left/right arrays, ordered inner (closest to center) to outer
constexpr uint LeftSensorInnerPin = 4;
constexpr uint LeftSensorMiddlePin = 5;
constexpr uint LeftSensorOuterPin = 6;

constexpr uint RightSensorInnerPin = 7;
constexpr uint RightSensorMiddlePin = 8;
constexpr uint RightSensorOuterPin = 9;
