
#include "event.h"

    float startTime, duration, endTime;
    int effectType;
    
    
    
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
        percussionFountain(0,0);
        break;
    case 2:
        //tempo
        break;
    case 3:
        //zerocrossings
        break;
    }
}

void event::percussionFountain(int x, int y)
{
    
}

//destructor
event::~event()
{
}

