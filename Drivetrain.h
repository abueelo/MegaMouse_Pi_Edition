#pragma once

#include "Motors.h"
#include "EnumHelper.h"

// Wraps the two drive motors: cell/half-cell forward moves, in-place 90 degree turns, and the
// drift-correction control loop. Once a move is armed, this class only ever calls
// Motor::SetStepRate on it - never Motor::SetSteps again - and SetStepRate never touches the
// step countdown, so both wheels always complete the exact same number of steps for a given
// move no matter how much correction was applied mid-move.
class Drivetrain
{
private:
    Motor left;
    Motor right;
    bool forwardMoveInProgress;

public:
    Drivetrain(uint leftStepPin, uint leftDirPin, uint leftMsPin,
               uint rightStepPin, uint rightDirPin, uint rightMsPin);

    void BeginForwardCell();     // arms both motors for StepsPerCell, ramped
    void BeginForwardHalfCell(); // StepsPerHalfCell - used once, to center in the start cell
    void BeginTurnRight();       // in-place 90 degree turn, StepsPerQuarterTurn
    void BeginTurnLeft();

    bool IsBusy() const;

    // Nudges one wheel's step rate relative to the other to correct lateral drift. A no-op
    // unless a forward move is currently in flight (BeginTurnRight/Left clear that flag), so
    // callers don't need to gate this themselves.
    void ApplyDriftCorrection(MouseState state);
};
