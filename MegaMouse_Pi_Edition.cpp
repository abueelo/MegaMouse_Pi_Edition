#include <cstring>
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
    logger.Init();

    SensorArray sensors(FrontSensorPin,
                         LeftSensorInnerPin, LeftSensorMiddlePin, LeftSensorOuterPin,
                         RightSensorInnerPin, RightSensorMiddlePin, RightSensorOuterPin);

    Drivetrain drivetrain(LeftMotorStepPin, LeftMotorDirPin, LeftMotorMsPin,
                           RightMotorStepPin, RightMotorDirPin, RightMotorMsPin);

    Maze maze;
    MazeSolver solver(maze);

    RobotPhase phase = Idle;
    uint8_t mouseX = 0;
    uint8_t mouseY = 0;
    CardinalDirections heading = North;

    logger.Log("MegaMouse ready - send \"start\" to begin the discovery run");

    while (true)
    {
        logger.Update();

        if (phase == Idle || phase == AwaitingFastRunCommand)
        {
            if (logger.HasCommand())
            {
                bool isStart = strcmp(logger.TakeCommand(), "start") == 0;
                if (isStart && phase == Idle)
                {
                    logger.Log("starting discovery run");
                    drivetrain.BeginForwardHalfCell();
                    phase = CenteringInStartCell;
                }
                else if (isStart && phase == AwaitingFastRunCommand)
                {
                    logger.Log("starting fast run");
                    mouseX = 0;
                    mouseY = 0;
                    // heading is whatever the return run left the mouse facing - the planner
                    // below re-orients from there automatically, no explicit "face North" needed
                    phase = FastRunning;
                }
            }
            continue;
        }

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
                    logger.Log("back at start, send \"start\" to begin the fast run");
                    phase = AwaitingFastRunCommand;
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
