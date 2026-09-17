#pragma once

// Prints progress messages over USB serial. This is the one place in the codebase that touches
// stdio, so the rest of the robot logic stays free of printf/formatting clutter - main() just
// calls Log()/Logf().
class Logger
{
public:
    void Log(const char *message);
    void Logf(const char *format, ...);
};
