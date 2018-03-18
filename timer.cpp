

#include "timer.h"

unsigned long begTime;

timer::timer()
{
}

void timer::start()
{
    begTime = clock();
}

float timer::elapsedTime()
{
    return ((float)((unsigned long) clock() - begTime) / (float)CLOCKS_PER_SEC)*60;
}

bool timer::isTimeout(unsigned long seconds)
{
    return seconds >= elapsedTime();
}

timer::~timer()
{
}

