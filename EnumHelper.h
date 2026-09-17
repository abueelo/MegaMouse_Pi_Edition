#pragma once

enum MotorDirections
{
    Clockwise,
    CounterClockwise
};

inline MotorDirections Opposite(MotorDirections dir)
{
    return dir == Clockwise ? CounterClockwise : Clockwise;
}

enum CardinalDirections
{
    North,
    East,
    South,
    West
};

enum CellState
{
    CellError,
    DeadEnd,
    CrossRoads,
    RightTurn,
    LeftTurn,
    Corridor,
    TJunction,
    RightTurnTJunction,
    LeftTurnTJunction
};

enum MouseState
{
    MouseError,
    Centered,
    DriftRight,
    DriftLeft
};

enum MouseInstructions
{
    Forward,
    RotateRight,
    RotateLeft,
    RotateToNorth,
    RotateToEast,
    RotateToSouth,
    RotateToWest
};

// Top-level run state, driven by the main loop
enum RobotPhase
{
    CenteringInStartCell, // one-off half-cell move so the mouse's center matches the cell center
    Discovering,          // flood-fill exploration, walls being learned as we go
    ReturningToStart,     // walking the best known path back to (0,0)
    FastRunning,          // dead-reckoning straight to the goal, no sensing at all
    Finished
};