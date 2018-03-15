
#include "event.h"

    float startTime, duration, endTime;
    int effectType;
    float currentX, currentZ;
    float r, g, b;
    
    typedef struct{
        GLfloat x,y,z;
        GLfloat xaccel,yaccel,zaccel;
    } particle;
    
    particle particleArray[1000];
   
//constructor
event::event()
{
    startTime = 0.0;
    effectType = 1;
    duration = 0.5;
    endTime = startTime + duration;
    r = 1;
    b = 0;
    g = 0;
    
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
        setupFountain();
    }
    r = 1;
    b = 0;
    g = 0;
}

void event::setColour(float red, float blue, float green)
{
    r = red;
    b = blue;
    g = green;
}

void event::eventAnimate()
{
    switch(effectType){
    case 1: //fountain
        percussionFountain();
        break;
    case 2:
        //tempo
        break;
    case 3:
        //zerocrossings {vocals}
        drawSpheres();
        break;
    }
}

void event::percussionFountain()
{
    glColor3f(r,g,b);
    
    for(int i = 0; i < 999; i ++)
    {
        glBegin(GL_POINTS);
            glVertex3f(particleArray[i].x,particleArray[i].y,particleArray[i].z);
        glEnd;
        
        particleArray[i].x += particleArray[i].xaccel;
        particleArray[i].y += particleArray[i].yaccel;
        particleArray[i].z += particleArray[i].zaccel;
    }
}

void event::setupFountain()
{
    for(int i = 0; i < 999; i++)
    {
        particleArray[i].x = currentX;
        particleArray[i].z = currentZ;
        particleArray[i].y = 0;
    }
}

void event::drawSpheres()
{
    glColor3f(r,g,b);
    glutSolidSphere(20,10,10);
}

//destructor
event::~event()
{
    
}

