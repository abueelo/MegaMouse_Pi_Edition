#pragma once

#include <cstdint>
#include "EnumHelper.h"
#include "Config.h"

// A fixed-size MazeWidth x MazeHeight grid of cell walls, plus Lee's algorithm (flood fill) for
// finding the shortest known path to a target cell. No heap allocation - sized entirely from the
// MazeWidth/MazeHeight constants in Config.h. (0,0) is the bottom-left cell; North is +y, East
// is +x.
class Maze
{
private:
    uint8_t walls[MazeHeight][MazeWidth];      // one 4-bit mask per cell, bit indexed by CardinalDirections
    uint16_t distances[MazeHeight][MazeWidth]; // filled in by FloodFill()

    static bool InBounds(int x, int y);
    static void Step(int x, int y, CardinalDirections dir, int &nx, int &ny);

public:
    Maze(); // outer boundary walled, everything interior open/unknown until sensed

    void SetWall(uint8_t x, uint8_t y, CardinalDirections side, bool present); // mirrors onto the neighbour
    bool HasWall(uint8_t x, uint8_t y, CardinalDirections side) const;

    void FloodFill(uint8_t targetX, uint8_t targetY);
    uint16_t GetDistance(uint8_t x, uint8_t y) const;

    // Among the neighbours reachable from (x,y) (no wall in the way), returns the direction
    // towards whichever has the lowest flood-fill distance. Ties favour tieBreakHeading, so the
    // mouse doesn't turn unnecessarily when two options are equally good.
    CardinalDirections GetNextDirectionTowardTarget(uint8_t x, uint8_t y, CardinalDirections tieBreakHeading) const;
};
