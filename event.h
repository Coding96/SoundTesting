#ifndef EVENT_H
#define EVENT_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL2/SDL.h>

class event {
public:
    float startTime, duration, endTime;
    int effectType;
    event();
    event(float sTime, int eType, float dur);
    void setColour(float red, float blue, float green);
    void eventAnimate();
    virtual ~event();
private:
    int currentX, currentZ;
    void percussionFountain();
    void setupFountain();
    void drawSpheres();
    float myRandom();
};

#endif 

