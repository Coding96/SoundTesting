
#include "event.h"

    float startTime, duration, endTime;
    int effectType;
    int currentX, currentY;
    
    typedef struct {
        float x,y,z;
        float xaccel,yaccel,zaccel;
        float r,g,b;
        
    } particle;
    
    particle particleArray[1000];
    
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
    
    if(effectType == 1)
    {
        initializeFountain();
    }
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
    for (int i = 0; i < 999 ; i++)
    {
        particleArray[i].x += particleArray[i].xaccel;
        particleArray[i].y += particleArray[i].yaccel;
        particleArray[i].z += particleArray[i].zaccel;
       
    }
}

void event::initializeFountain()
{
    for (int i = 0; i < 999 ; i++)
    {
        particleArray[i].x = 0;
        particleArray[i].y = 0;
        particleArray[i].z = 0;
        
        //needs to be changed to random
        particleArray[i].xaccel = 0.1;
        particleArray[i].yaccel = 0.1;
        particleArray[i].zaccel = 0.1;
        
        //needs to change when genre is implemented
        
        particleArray[i].r = 1;
        particleArray[i].b = 0;
        particleArray[i].g = 0;
    }
}

//destructor
event::~event()
{
}

