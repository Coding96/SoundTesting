
#include "event.h"

    float startTime, duration, endTime;
    int effectType;
    int currentX, currentY;
    
    
//constructor
event::event()
{
    startTime = 0.0;
    effectType = 1;
    duration = 0.5;
    endTime = startTime + duration;
    
}

//constructor
event::event(float sTime, int eType, float dur)
{
    startTime = sTime;
    effectType = eType;
    duration = dur;
    endTime = startTime + duration;
}

void event::eventAnimate()
{
    switch(effectType){
    case 1: //fountain
        currentX = 0;
        currentY = 0;
        percussionFountain();
        break;
    case 2:
        //tempo
        break;
    case 3:
        //zerocrossings {vocals}
        break;
    }
}

void event::percussionFountain()
{
    
}

//destructor
event::~event()
{
}

