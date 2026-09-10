#pragma once

#include "pico/stdlib.h"

// Every physical pin the mouse uses, gathered in one place. All values below are placeholders -
// the real wiring isn't finalised yet, so update them here once it is and nothing else needs to
// change.

// Drivetrain - see Motor for what each pin does
constexpr uint LeftMotorStepPin = 2;
constexpr uint LeftMotorDirPin = 3;
constexpr uint LeftMotorMsPin = 4;

constexpr uint RightMotorStepPin = 5;
constexpr uint RightMotorDirPin = 6;
constexpr uint RightMotorMsPin = 7;

// IR sensors - all digital, active-high (HIGH = wall/reflection detected)
constexpr uint FrontSensorPin = 8;

// Left/right arrays, ordered inner (closest to center) to outer
constexpr uint LeftSensorInnerPin = 9;
constexpr uint LeftSensorMiddlePin = 10;
constexpr uint LeftSensorOuterPin = 11;

constexpr uint RightSensorInnerPin = 12;
constexpr uint RightSensorMiddlePin = 13;
constexpr uint RightSensorOuterPin = 14;
