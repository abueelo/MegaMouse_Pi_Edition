#include "MazeSolver.h"

MazeSolver::MazeSolver(Maze &maze_) : maze(maze_)
{
}

CardinalDirections MazeSolver::TurnLeftFrom(CardinalDirections heading)
{
    // North, East, South, West is already a clockwise rotation order
    return static_cast<CardinalDirections>((heading + 3) % 4);
}

CardinalDirections MazeSolver::TurnRightFrom(CardinalDirections heading)
{
    return static_cast<CardinalDirections>((heading + 1) % 4);
}

CellState MazeSolver::ClassifyCell(bool leftOpen, bool straightOpen, bool rightOpen)
{
    // The enum's declared order doesn't line up numerically with a 3-bit open/closed pattern,
    // so this is a lookup, never a cast.
    if (!straightOpen && !leftOpen && !rightOpen) return DeadEnd;
    if (straightOpen && !leftOpen && !rightOpen) return Corridor;
    if (!straightOpen && leftOpen && !rightOpen) return LeftTurn;
    if (!straightOpen && !leftOpen && rightOpen) return RightTurn;
    if (straightOpen && leftOpen && !rightOpen) return LeftTurnTJunction;
    if (straightOpen && !leftOpen && rightOpen) return RightTurnTJunction;
    if (!straightOpen && leftOpen && rightOpen) return TJunction;
    return CrossRoads; // straightOpen && leftOpen && rightOpen
}

void MazeSolver::RecordWalls(uint8_t x, uint8_t y, CardinalDirections heading,
                              bool leftOpen, bool straightOpen, bool rightOpen)
{
    maze.SetWall(x, y, heading, !straightOpen);
    maze.SetWall(x, y, TurnLeftFrom(heading), !leftOpen);
    maze.SetWall(x, y, TurnRightFrom(heading), !rightOpen);
}

MouseInstructions MazeSolver::GetNextInstruction(uint8_t x, uint8_t y, CardinalDirections heading,
                                                  uint8_t targetX, uint8_t targetY) const
{
    maze.FloodFill(targetX, targetY);
    CardinalDirections desired = maze.GetNextDirectionTowardTarget(x, y, heading);

    if (desired == heading) return Forward;
    if (desired == TurnRightFrom(heading)) return RotateRight;
    if (desired == TurnLeftFrom(heading)) return RotateLeft;
    return RotateRight; // 180 degrees needed - first of two turns, resolved on the next call
}

CardinalDirections MazeSolver::NextHeadingAfterInstruction(CardinalDirections current, MouseInstructions instr)
{
    switch (instr)
    {
    case RotateRight:    return TurnRightFrom(current);
    case RotateLeft:     return TurnLeftFrom(current);
    case RotateToNorth:  return North;
    case RotateToEast:   return East;
    case RotateToSouth:  return South;
    case RotateToWest:   return West;
    case Forward:
    default:
        return current;
    }
}
