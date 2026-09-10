#pragma once

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "EnumHelper.h"

// 7.6cm diameter wheels. 18cm corridor. 1.8deg per step. 151 steps to go a cell
class Motor
{
private:
    uint stepPin;
    uint dirPin;
    uint msPin;

    uint pwmSlice;   // which of the RP2040's 8 PWM hardware blocks stepPin belongs to
    uint pwmChannel; // which of that slice's two outputs (A or B) stepPin is on

    volatile int Steps; // pulses still to go - the only thing that decides when the move ends

    uint32_t CruiseWrap; // wrap value for the speed set by SetStepRate()
    uint32_t StartWrap;  // wrap value for the first/last step of a ramped move

    volatile int RampUpStepsRemaining;
    int RampUpStepsTotal;
    int RampDownStepsTotal;

    static Motor *SliceOwner[NUM_PWM_SLICES];
    static bool IrqHandlerInstalled;
    static void PwmWrapIrqHandler();

    static uint32_t WrapForRate(int StepsPerSecond);
    static uint32_t InterpolateWrap(uint32_t fromWrap, uint32_t toWrap, uint32_t doneSteps, int totalSteps);
    void ApplyWrap(uint32_t wrap); // sets wrap + matching 50% duty level on this pwmSlice

public:
    Motor(uint stepPin_, uint dirPin_, uint msPin_);

    volatile bool FinishedStepFlag;

    void StepTickerFunc(); // called once per PWM wrap, i.e. once per real step pulse

    void SetMicroStep(bool OnOff) // off is low, on is high
    {
        gpio_put(msPin, OnOff);
    }
    void SetStepRate(int StepsPerSecond); // cruise speed
    void SetStepDirection(MotorDirections dir);

    // rampSteps_/startStepsPerSecond_ ramp the first rampSteps_ steps of the move up from
    // startStepsPerSecond_ to the cruise rate set by SetStepRate(), and mirror that back down
    // over the last rampSteps_ steps so the move ends at the same slow speed it started at.
    // In between, the PWM just free-runs at cruise speed with no CPU involvement. If rampSteps_
    // would be more than half the move, it's shrunk so the up- and down-ramps meet in the
    // middle without overlapping. None of this changes the pulse count: Steps is decremented
    // once per real completed pulse regardless of ramp state, so the total number of steps
    // (and therefore distance) put out is always exactly steps_, ramped or not.
    void SetSteps(int steps_, int rampSteps_ = 0, int startStepsPerSecond_ = 0);

    void AddSteps(int stepsToAdd)
    {
        if (stepsToAdd > 0)
        {
            Steps += stepsToAdd;
        }
    }
};
