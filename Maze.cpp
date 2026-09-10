#include "Maze.h"

namespace
{
    constexpr uint16_t Unreachable = 0xFFFF;
    constexpr CardinalDirections AllDirections[4] = {North, East, South, West};

    constexpr uint8_t WallBit(CardinalDirections dir)
    {
        return static_cast<uint8_t>(1u << dir);
    }

    CardinalDirections OppositeDirection(CardinalDirections dir)
    {
        switch (dir)
        {
        case North: return South;
        case East:  return West;
        case South: return North;
        case West:  return East;
        }
        return North; // unreachable
    }
}

Maze::Maze()
{
    for (int y = 0; y < MazeHeight; y++)
    {
        for (int x = 0; x < MazeWidth; x++)
        {
            walls[y][x] = 0;
            distances[y][x] = Unreachable;
        }
    }

    // the mouse can never sense its way past the outer boundary, so it's always walled
    for (int x = 0; x < MazeWidth; x++)
    {
        walls[0][x] |= WallBit(South);
        walls[MazeHeight - 1][x] |= WallBit(North);
    }
    for (int y = 0; y < MazeHeight; y++)
    {
        walls[y][0] |= WallBit(West);
        walls[y][MazeWidth - 1] |= WallBit(East);
    }
}

bool Maze::InBounds(int x, int y)
{
    return x >= 0 && x < MazeWidth && y >= 0 && y < MazeHeight;
}

void Maze::Step(int x, int y, CardinalDirections dir, int &nx, int &ny)
{
    nx = x;
    ny = y;
    switch (dir)
    {
    case North: ny = y + 1; break;
    case East:  nx = x + 1; break;
    case South: ny = y - 1; break;
    case West:  nx = x - 1; break;
    }
}

void Maze::SetWall(uint8_t x, uint8_t y, CardinalDirections side, bool present)
{
    if (!InBounds(x, y))
    {
        return;
    }

    if (present)
    {
        walls[y][x] |= WallBit(side);
    }
    else
    {
        walls[y][x] &= static_cast<uint8_t>(~WallBit(side));
    }

    // a wall is shared between two cells' records - mirror it onto the neighbour
    int nx, ny;
    Step(x, y, side, nx, ny);
    if (InBounds(nx, ny))
    {
        CardinalDirections opposite = OppositeDirection(side);
        if (present)
        {
            walls[ny][nx] |= WallBit(opposite);
        }
        else
        {
            walls[ny][nx] &= static_cast<uint8_t>(~WallBit(opposite));
        }
    }
}

bool Maze::HasWall(uint8_t x, uint8_t y, CardinalDirections side) const
{
    if (!InBounds(x, y))
    {
        return true; // out of bounds counts as walled off
    }
    return (walls[y][x] & WallBit(side)) != 0;
}

void Maze::FloodFill(uint8_t targetX, uint8_t targetY)
{
    for (int y = 0; y < MazeHeight; y++)
    {
        for (int x = 0; x < MazeWidth; x++)
        {
            distances[y][x] = Unreachable;
        }
    }

    if (!InBounds(targetX, targetY))
    {
        return;
    }

    // fixed-size array-backed queue, big enough for every cell in the maze - no heap
    uint8_t queueX[MazeWidth * MazeHeight];
    uint8_t queueY[MazeWidth * MazeHeight];
    int head = 0;
    int tail = 0;

    distances[targetY][targetX] = 0;
    queueX[tail] = targetX;
    queueY[tail] = targetY;
    tail++;

    while (head < tail)
    {
        uint8_t x = queueX[head];
        uint8_t y = queueY[head];
        head++;

        for (CardinalDirections dir : AllDirections)
        {
            if (HasWall(x, y, dir))
            {
                continue;
            }

            int nx, ny;
            Step(x, y, dir, nx, ny);
            if (!InBounds(nx, ny))
            {
                continue;
            }

            if (distances[ny][nx] == Unreachable)
            {
                distances[ny][nx] = static_cast<uint16_t>(distances[y][x] + 1);
                queueX[tail] = static_cast<uint8_t>(nx);
                queueY[tail] = static_cast<uint8_t>(ny);
                tail++;
            }
        }
    }
}

uint16_t Maze::GetDistance(uint8_t x, uint8_t y) const
{
    if (!InBounds(x, y))
    {
        return Unreachable;
    }
    return distances[y][x];
}

CardinalDirections Maze::GetNextDirectionTowardTarget(uint8_t x, uint8_t y, CardinalDirections tieBreakHeading) const
{
    CardinalDirections best = tieBreakHeading;
    uint16_t bestDistance = Unreachable;

    // tieBreakHeading is checked first, and the loop below only replaces it on a strictly
    // smaller distance, so it naturally wins ties without any special-casing
    if (!HasWall(x, y, tieBreakHeading))
    {
        int nx, ny;
        Step(x, y, tieBreakHeading, nx, ny);
        if (InBounds(nx, ny))
        {
            bestDistance = GetDistance(nx, ny);
        }
    }

    for (CardinalDirections dir : AllDirections)
    {
        if (dir == tieBreakHeading || HasWall(x, y, dir))
        {
            continue;
        }

        int nx, ny;
        Step(x, y, dir, nx, ny);
        if (!InBounds(nx, ny))
        {
            continue;
        }

        uint16_t distance = GetDistance(nx, ny);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = dir;
        }
    }

    return best;
}
