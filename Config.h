#pragma once

#include <cstdint>
#include "EnumHelper.h"

// Every tunable constant for the mouse, gathered in one place for fine-tuning without hunting
// through the rest of the code.

// Maze size - bump these when testing on a bigger maze, nothing else needs to change
constexpr int MazeWidth = 3;
constexpr int MazeHeight = 3;

// Movement calibration
// 7.6cm wheels, 18cm cell, 1.8deg/step, full step -> ~151 steps/cell (see Motors.h comment).
// Re-measure and adjust if the wheels or cell size change.
constexpr int StepsPerCell = 151;
constexpr int StepsPerHalfCell = StepsPerCell / 2;

// PLACEHOLDER - wheelbase hasn't been measured yet. Tune this on the real chassis until a
// commanded 90 degree turn actually turns 90 degrees.
constexpr int StepsPerQuarterTurn = 75;

// PLACEHOLDER - bring-up-safe conservative values. A stepper has very little torque at a dead
// stop, so starting too fast just stalls/twitches the motor instead of turning it - ramp up to a
// slow 50 sps cruise instead. Once movement is confirmed working, raise these gradually until it
// stalls again, then back off a bit.
constexpr int ForwardCruiseStepsPerSecond = 50;
constexpr int ForwardRampSteps = 20;
constexpr int ForwardRampStartStepsPerSecond = 25;

constexpr int TurnCruiseStepsPerSecond = 50;
constexpr int TurnRampSteps = 10;
constexpr int TurnRampStartStepsPerSecond = 25;

// Which MotorDirections value drives each wheel "forward" is unknown until wired - flip
// whichever one is backwards once bring-up shows which way is which. Turning is derived from
// these with Opposite() rather than needing its own set of constants.
constexpr MotorDirections LeftForwardDirection = Clockwise;
constexpr MotorDirections RightForwardDirection = CounterClockwise;

// How hard a wheel's step rate gets nudged during drift correction
constexpr int DriftCorrectionNudgeStepsPerSecond = 150;

// How long the mouse pauses before each run starts - once when powered on, and again once it's
// back home before the fast run
constexpr uint32_t StartDelayMs = 5000;
