#include "Motors.h"
#include "hardware/irq.h"

Motor *Motor::SliceOwner[NUM_PWM_SLICES] = {};
bool Motor::IrqHandlerInstalled = false;

void Motor::PwmWrapIrqHandler()
{
    uint32_t status = pwm_get_irq_status_mask();
    for (uint slice = 0; slice < NUM_PWM_SLICES; slice++)
    {
        if (status & (1u << slice))
        {
            pwm_clear_irq(slice);
            if (SliceOwner[slice] != nullptr)
            {
                SliceOwner[slice]->StepTickerFunc();
            }
        }
    }
}

uint32_t Motor::WrapForRate(int StepsPerSecond)
{
    if (StepsPerSecond <= 0)
    {
        return 65535u;
    }

    uint32_t wrap = (1000000u / static_cast<uint32_t>(StepsPerSecond));
    if (wrap == 0)
    {
        wrap = 1;
    }
    wrap -= 1;
    if (wrap > 65535u) // 1MHz counter can't go this slow - clamp rather than let it silently truncate
    {
        wrap = 65535u;
    }
    return wrap;
}

// Linear step from fromWrap towards toWrap, doneSteps/totalSteps of the way there.
uint32_t Motor::InterpolateWrap(uint32_t fromWrap, uint32_t toWrap, uint32_t doneSteps, int totalSteps)
{
    if (fromWrap == toWrap || totalSteps <= 0)
    {
        return toWrap;
    }

    uint32_t span = fromWrap > toWrap ? fromWrap - toWrap : toWrap - fromWrap;
    uint32_t offset = static_cast<uint32_t>((static_cast<uint64_t>(span) * doneSteps) / static_cast<uint32_t>(totalSteps));

    return fromWrap > toWrap ? fromWrap - offset : fromWrap + offset;
}

void Motor::ApplyWrap(uint32_t wrap)
{
    pwm_set_wrap(pwmSlice, wrap);
    pwm_set_chan_level(pwmSlice, pwmChannel, (wrap + 1) / 2); // 50% duty
}

Motor::Motor(uint stepPin_, uint dirPin_, uint msPin_) : stepPin(stepPin_), dirPin(dirPin_), msPin(msPin_)
{
    gpio_init(dirPin);
    gpio_set_dir(dirPin, GPIO_OUT);
    gpio_init(msPin);
    gpio_set_dir(msPin, GPIO_OUT);
    gpio_put(msPin, false);

    Steps = 0;
    FinishedStepFlag = true;
    CruiseWrap = 1999; // ~500Hz default, until SetStepRate() picks a real rate
    StartWrap = 0;
    RampUpStepsRemaining = 0;
    RampUpStepsTotal = 0;
    RampDownStepsTotal = 0;

    gpio_set_function(stepPin, GPIO_FUNC_PWM);
    pwmSlice = pwm_gpio_to_slice_num(stepPin);
    pwmChannel = pwm_gpio_to_channel(stepPin);

    SliceOwner[pwmSlice] = this;

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f); // 125MHz sys clock / 125 == 1MHz PWM counter tick
    pwm_config_set_wrap(&config, CruiseWrap);
    pwm_init(pwmSlice, &config, false); // false == leave it disabled until SetSteps() starts it
    pwm_set_chan_level(pwmSlice, pwmChannel, (CruiseWrap + 1) / 2); // 50% duty

    pwm_clear_irq(pwmSlice);
    pwm_set_irq_enabled(pwmSlice, true);

    if (!IrqHandlerInstalled)
    {
        irq_set_exclusive_handler(PWM_IRQ_WRAP, PwmWrapIrqHandler);
        irq_set_enabled(PWM_IRQ_WRAP, true);
        IrqHandlerInstalled = true;
    }
}

void Motor::SetStepRate(int StepsPerSecond)
{
    if (StepsPerSecond <= 0)
    {
        pwm_set_enabled(pwmSlice, false);
        return;
    }

    CruiseWrap = WrapForRate(StepsPerSecond);
    if (RampUpStepsRemaining <= 0) // not mid ramp-up - apply straight away
    {
        ApplyWrap(CruiseWrap);
    }
}

void Motor::SetStepDirection(MotorDirections dir)
{
    gpio_put(dirPin, static_cast<bool>(dir));
}

void Motor::SetSteps(int steps_, int rampSteps_, int startStepsPerSecond_)
{
    Steps = steps_;

    StartWrap = (rampSteps_ > 0 && startStepsPerSecond_ > 0) ? WrapForRate(startStepsPerSecond_) : 0;

    if (rampSteps_ > 0 && steps_ > 0 && StartWrap > CruiseWrap) // start rate must be slower than cruise, i.e. a bigger wrap
    {
        // split the ramp between the start and the end of the move, without ever asking for
        // more ramp than the move has steps for - the up- and down-ramps can meet in the
        // middle but must never overlap
        int rampEachEnd = rampSteps_ < (steps_ / 2) ? rampSteps_ : (steps_ / 2);

        RampUpStepsTotal = rampEachEnd;
        RampUpStepsRemaining = RampUpStepsTotal;
        RampDownStepsTotal = rampEachEnd;

        ApplyWrap(RampUpStepsTotal > 0 ? StartWrap : CruiseWrap);
    }
    else
    {
        RampUpStepsRemaining = 0;
        RampDownStepsTotal = 0;
        ApplyWrap(CruiseWrap);
    }

    pwm_set_counter(pwmSlice, 0);
    pwm_set_enabled(pwmSlice, Steps > 0);
}

void Motor::StepTickerFunc()
{
    // Steps is decremented here once per real, physically-generated pulse, and nowhere
    // else - ramping only ever changes the wrap value applied for the *next* pulse below,
    // never whether this decrement happens, so the total pulse count out of stepPin for
    // this move is always exactly the steps_ passed to SetSteps regardless of ramp state.
    if (Steps > 0)
    {
        Steps--;
    }

    if (Steps <= 0)
    {
        FinishedStepFlag = true;
        pwm_set_enabled(pwmSlice, false);
        return;
    }

    // Steps now holds how many pulses remain, including the one about to be scheduled below.
    if (RampUpStepsRemaining > 0)
    {
        RampUpStepsRemaining--;
        uint32_t doneSteps = static_cast<uint32_t>(RampUpStepsTotal - RampUpStepsRemaining);
        ApplyWrap(InterpolateWrap(StartWrap, CruiseWrap, doneSteps, RampUpStepsTotal));
    }
    else if (RampDownStepsTotal > 0 && Steps <= RampDownStepsTotal)
    {
        if (Steps == 1)
        {
            ApplyWrap(StartWrap); // guarantee the very last pulse lands exactly on the start speed
        }
        else
        {
            uint32_t doneSteps = static_cast<uint32_t>(RampDownStepsTotal - Steps);
            ApplyWrap(InterpolateWrap(CruiseWrap, StartWrap, doneSteps, RampDownStepsTotal));
        }
    }
}
