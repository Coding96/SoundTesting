
#include "event.h"

    float startTime, duration;
    int effectType;
    
    
    
//constructor
event::event()
{
}

//constructor
event::event(float sTime, int eType, float dur)
{
    startTime = sTime;
    effectType = eType;
    duration = dur;
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

