#include "Drivetrain.h"
#include "Config.h"

Drivetrain::Drivetrain(uint leftStepPin, uint leftDirPin, uint leftMsPin,
                        uint rightStepPin, uint rightDirPin, uint rightMsPin)
    : left(leftStepPin, leftDirPin, leftMsPin),
      right(rightStepPin, rightDirPin, rightMsPin),
      forwardMoveInProgress(false)
{
}

void Drivetrain::BeginForwardCell()
{
    left.SetStepDirection(LeftForwardDirection);
    right.SetStepDirection(RightForwardDirection);
    left.SetStepRate(ForwardCruiseStepsPerSecond);
    right.SetStepRate(ForwardCruiseStepsPerSecond);
    left.SetSteps(StepsPerCell, ForwardRampSteps, ForwardRampStartStepsPerSecond);
    right.SetSteps(StepsPerCell, ForwardRampSteps, ForwardRampStartStepsPerSecond);
    forwardMoveInProgress = true;
}

void Drivetrain::BeginForwardHalfCell()
{
    left.SetStepDirection(LeftForwardDirection);
    right.SetStepDirection(RightForwardDirection);
    left.SetStepRate(ForwardCruiseStepsPerSecond);
    right.SetStepRate(ForwardCruiseStepsPerSecond);
    left.SetSteps(StepsPerHalfCell, ForwardRampSteps, ForwardRampStartStepsPerSecond);
    right.SetSteps(StepsPerHalfCell, ForwardRampSteps, ForwardRampStartStepsPerSecond);
    forwardMoveInProgress = true;
}

void Drivetrain::BeginTurnRight()
{
    // spin on the spot: left wheel keeps turning "forward", right wheel reverses
    left.SetStepDirection(LeftForwardDirection);
    right.SetStepDirection(Opposite(RightForwardDirection));
    left.SetStepRate(TurnCruiseStepsPerSecond);
    right.SetStepRate(TurnCruiseStepsPerSecond);
    left.SetSteps(StepsPerQuarterTurn, TurnRampSteps, TurnRampStartStepsPerSecond);
    right.SetSteps(StepsPerQuarterTurn, TurnRampSteps, TurnRampStartStepsPerSecond);
    forwardMoveInProgress = false;
}

void Drivetrain::BeginTurnLeft()
{
    left.SetStepDirection(Opposite(LeftForwardDirection));
    right.SetStepDirection(RightForwardDirection);
    left.SetStepRate(TurnCruiseStepsPerSecond);
    right.SetStepRate(TurnCruiseStepsPerSecond);
    left.SetSteps(StepsPerQuarterTurn, TurnRampSteps, TurnRampStartStepsPerSecond);
    right.SetSteps(StepsPerQuarterTurn, TurnRampSteps, TurnRampStartStepsPerSecond);
    forwardMoveInProgress = false;
}

bool Drivetrain::IsBusy() const
{
    return !left.FinishedStepFlag || !right.FinishedStepFlag;
}

void Drivetrain::ApplyDriftCorrection(MouseState state)
{
    if (!forwardMoveInProgress || !IsBusy())
    {
        forwardMoveInProgress = false;
        return;
    }

    // A differential drive turns towards whichever wheel is slower, like a tank turning towards
    // its slower track - so correcting "too close to a wall" means slowing the wheel on that
    // side (and speeding up the other) to steer back away from it.
    switch (state)
    {
    case Centered:
        left.SetStepRate(ForwardCruiseStepsPerSecond);
        right.SetStepRate(ForwardCruiseStepsPerSecond);
        break;
    case DriftLeft: // too close to the left wall - slow the right wheel to steer back right
        left.SetStepRate(ForwardCruiseStepsPerSecond + DriftCorrectionNudgeStepsPerSecond);
        right.SetStepRate(ForwardCruiseStepsPerSecond - DriftCorrectionNudgeStepsPerSecond);
        break;
    case DriftRight: // too close to the right wall - slow the left wheel to steer back left
        left.SetStepRate(ForwardCruiseStepsPerSecond - DriftCorrectionNudgeStepsPerSecond);
        right.SetStepRate(ForwardCruiseStepsPerSecond + DriftCorrectionNudgeStepsPerSecond);
        break;
    case MouseError:
        // no reliable reading - hold whatever rates are already commanded rather than snapping
        // back to nominal on a single bad/ambiguous sample
        break;
    }
}
