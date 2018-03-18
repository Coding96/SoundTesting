
#ifndef TIMER_H
#define TIMER_H

#include <time.h>


class timer {
public:
    unsigned long begTime;
    void start();
    float elapsedTime();
    bool isTimeout(unsigned long seconds);
    timer();
    timer(const timer& orig);
    virtual ~timer();
private:

};

#endif /* TIMER_H */

