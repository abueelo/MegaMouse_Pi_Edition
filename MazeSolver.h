#pragma once

#include "Maze.h"
#include "EnumHelper.h"

// Sensed-walls -> Maze::SetWall, and (position, heading, target) -> the next MouseInstructions
// to execute. One instance is reused unmodified across discovery, the return-to-start walk, and
// the fast run - only which target is passed in, and whether RecordWalls gets called, differs
// between them.
class MazeSolver
{
private:
    Maze &maze;

    static CardinalDirections TurnLeftFrom(CardinalDirections heading);
    static CardinalDirections TurnRightFrom(CardinalDirections heading);

public:
    explicit MazeSolver(Maze &maze_);

    // Classifies a cell's exits, relative to the heading it was entered from, into the
    // CellState vocabulary from EnumHelper.h.
    static CellState ClassifyCell(bool leftOpen, bool straightOpen, bool rightOpen);

    // Translates locally-sensed left/straight/right openings (relative to heading) into
    // absolute wall records on the maze. The direction behind the mouse is never touched - it's
    // where it just came from, already known.
    void RecordWalls(uint8_t x, uint8_t y, CardinalDirections heading,
                      bool leftOpen, bool straightOpen, bool rightOpen);

    // Re-floods towards (targetX, targetY) and converts the best next neighbour into a single
    // relative instruction. Call again after each completed move - a 180 degree turnaround
    // naturally falls out as two consecutive RotateRight calls with no special-casing needed.
    MouseInstructions GetNextInstruction(uint8_t x, uint8_t y, CardinalDirections heading,
                                          uint8_t targetX, uint8_t targetY) const;

    static CardinalDirections NextHeadingAfterInstruction(CardinalDirections current, MouseInstructions instr);
};
