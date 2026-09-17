#include "pico/stdlib.h"

#include "Pins.h"
#include "Config.h"
#include "EnumHelper.h"
#include "SensorArray.h"
#include "Drivetrain.h"
#include "Maze.h"
#include "MazeSolver.h"
#include "Logger.h"

namespace
{
    constexpr uint8_t GoalX = MazeWidth - 1;
    constexpr uint8_t GoalY = MazeHeight - 1;

    // Moves (x, y) one cell in whichever direction heading now points - only valid to call
    // right after issuing a Forward instruction.
    void AdvancePosition(uint8_t &x, uint8_t &y, CardinalDirections heading)
    {
        switch (heading)
        {
        case North: y++; break;
        case East:  x++; break;
        case South: y--; break;
        case West:  x--; break;
        }
    }

    // Issues one instruction to the drivetrain and updates the tracked (x, y, heading) to match.
    // Position/heading update the moment a move is issued, not when it completes - this is
    // open-loop dead reckoning, so a commanded move is always assumed to land exactly as asked.
    void ExecuteInstruction(MouseInstructions instr, Drivetrain &drivetrain,
                             uint8_t &x, uint8_t &y, CardinalDirections &heading)
    {
        heading = MazeSolver::NextHeadingAfterInstruction(heading, instr);

        switch (instr)
        {
        case Forward:
            drivetrain.BeginForwardCell();
            AdvancePosition(x, y, heading);
            break;
        case RotateRight:
            drivetrain.BeginTurnRight();
            break;
        case RotateLeft:
            drivetrain.BeginTurnLeft();
            break;
        default:
            break; // RotateToNorth/East/South/West aren't used by the maze-solving path executor
        }
    }
}

int main()
{
    stdio_init_all();

    Logger logger;

    SensorArray sensors(FrontSensorPin,
                         LeftSensorInnerPin, LeftSensorMiddlePin, LeftSensorOuterPin,
                         RightSensorInnerPin, RightSensorMiddlePin, RightSensorOuterPin);

    Drivetrain drivetrain(LeftMotorStepPin, LeftMotorDirPin, LeftMotorMsPin,
                           RightMotorStepPin, RightMotorDirPin, RightMotorMsPin);

    Maze maze;
    MazeSolver solver(maze);

    uint8_t mouseX = 0;
    uint8_t mouseY = 0;
    CardinalDirections heading = North;

    logger.Logf("MegaMouse booting - discovery run starts in %u seconds", StartDelayMs / 1000);
    sleep_ms(StartDelayMs);
    logger.Log("starting discovery run");
    drivetrain.BeginForwardHalfCell();
    RobotPhase phase = CenteringInStartCell;

    while (true)
    {
        if (phase == CenteringInStartCell)
        {
            if (!drivetrain.IsBusy())
            {
                phase = Discovering;
            }
            continue;
        }

        if (phase == Discovering || phase == ReturningToStart)
        {
            if (drivetrain.IsBusy())
            {
                // sensed continuously through the move, purely to keep the mouse running straight
                sensors.Read();
                drivetrain.ApplyDriftCorrection(sensors.GetDriftState());
                continue;
            }

            // just arrived (or just turned) - take a fresh, deliberate reading of this cell
            sensors.Read();

            if (phase == Discovering)
            {
                bool leftOpen = !sensors.LeftWallPresent();
                bool frontOpen = !sensors.FrontWallPresent();
                bool rightOpen = !sensors.RightWallPresent();
                solver.RecordWalls(mouseX, mouseY, heading, leftOpen, frontOpen, rightOpen);

                if (mouseX == GoalX && mouseY == GoalY)
                {
                    logger.Logf("goal reached at (%d,%d), returning to start", mouseX, mouseY);
                    phase = ReturningToStart;
                    continue;
                }

                MouseInstructions instr = solver.GetNextInstruction(mouseX, mouseY, heading, GoalX, GoalY);
                ExecuteInstruction(instr, drivetrain, mouseX, mouseY, heading);
            }
            else // ReturningToStart - path already known, just walk it and stay straight
            {
                if (mouseX == 0 && mouseY == 0)
                {
                    logger.Logf("back at start - fast run starts in %u seconds", StartDelayMs / 1000);
                    sleep_ms(StartDelayMs);
                    logger.Log("starting fast run");
                    // mouseX/mouseY are already (0,0) here; heading is whatever the return run
                    // left the mouse facing - the planner below re-orients from there
                    // automatically, no explicit "face North" needed
                    phase = FastRunning;
                    continue;
                }

                MouseInstructions instr = solver.GetNextInstruction(mouseX, mouseY, heading, 0, 0);
                ExecuteInstruction(instr, drivetrain, mouseX, mouseY, heading);
            }
            continue;
        }

        if (phase == FastRunning)
        {
            if (drivetrain.IsBusy())
            {
                continue; // no sensing at all - dead reckoning only, exactly as planned
            }

            if (mouseX == GoalX && mouseY == GoalY)
            {
                logger.Log("fast run complete");
                phase = Finished;
                continue;
            }

            MouseInstructions instr = solver.GetNextInstruction(mouseX, mouseY, heading, GoalX, GoalY);
            ExecuteInstruction(instr, drivetrain, mouseX, mouseY, heading);
            continue;
        }

        // Finished - nothing left to do
    }
}
