#ifndef _TIMER_H_
#define _TIMER_H_

#include <ctime>

class Timer {
public:
    static void Start() {
        Timer& timer = Timer::getInstance();
        gettimeofday(&timer.m_tv, NULL);
    }

    static double Delta() {
        Timer& timer = Timer::getInstance();

        struct timeval curr;
        gettimeofday(&curr, NULL);

        double dt = static_cast<double>(curr.tv_usec - timer.m_tv.tv_usec);
        dt /= CLOCKS_PER_SEC;
        dt += static_cast<double>(curr.tv_sec - timer.m_tv.tv_sec);

        gettimeofday(&timer.m_tv, NULL);

        return dt;
    }

private:

    Timer() {}
    Timer(const Timer& other) {}
    ~Timer() {}

    inline static Timer& getInstance() {
        static Timer timer;
        return timer;
    }

    struct timeval m_tv;
};

#endif
