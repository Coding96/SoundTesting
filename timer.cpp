

#include "timer.h"

unsigned long begTime;

timer::timer()
{
}

void timer::start()
{
    begTime = clock();
}

unsigned long timer::elapsedTime()
{
    return ((unsigned long) clock() - begTime) / CLOCKS_PER_SEC;
}

bool timer::isTimeout(unsigned long seconds)
{
    return seconds >= elapsedTime();
}

timer::~timer()
{
}

