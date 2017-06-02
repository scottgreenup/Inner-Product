#ifndef _TIMER_H_
#define _TIMER_H_

#include <ctime>
#include <iostream>
#include <map>
#include <string>

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

    static void DeltaPrint(std::string name) {
        Timer& timer = Timer::getInstance();
        std::cout << name << ": " << timer.Delta() << std::endl;
    }

    static void DeltaRemember(std::string name) {
        Timer& timer = Timer::getInstance();
        timer.m_deltas[name] = timer.Delta();
    }

    static void PrintDelta() {
        Timer& timer = Timer::getInstance();
        for (auto& it : timer.m_deltas) {
            std::cout << it.first << ": " << it.second << std::endl;
        }
    }

private:

    Timer() {
        srand(time(NULL));
    }
    Timer(const Timer& other) {}
    ~Timer() {}

    inline static Timer& getInstance() {
        static Timer timer;
        return timer;
    }



    struct timeval m_tv;

    std::map<std::string, double> m_deltas;
};

#endif
