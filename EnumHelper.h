#pragma once

enum MotorDirections
{
    Clockwise,
    CounterClockwise
};

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