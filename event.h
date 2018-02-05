#ifndef EVENT_H
#define EVENT_H

class event {
public:
    float startTime, duration;
    int effectType;
    event();
    event(float sTime, int eType, float dur);
    void eventAnimate();
    void percussionFountain(int x, int y);
    virtual ~event();
private:

};

#endif 

