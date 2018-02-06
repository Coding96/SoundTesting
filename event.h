#ifndef EVENT_H
#define EVENT_H

class event {
public:
    float startTime, duration, endTime;
    int effectType;
    event();
    event(float sTime, int eType, float dur);
    void eventAnimate();
    virtual ~event();
private:
    int currentX, currentY;
    void percussionFountain(int x, int y);
};

#endif 

