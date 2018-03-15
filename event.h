#ifndef EVENT_H
#define EVENT_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

class event {
public:
    float startTime, duration, endTime;
    int effectType;
    event();
    event(float sTime, int eType, float dur);
    void eventAnimate();
    virtual ~event();
private:
    int currentX, currentZ;
    void percussionFountain();
    void setupFountain();
    void drawSpheres();
};

#endif 

