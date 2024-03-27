// Sim.hpp
#ifndef SIM_HPP
    #define SIM_HPP

    #include "windows.h"

    struct Ant {
        int y;
        int x;
        int dir;
        COLORREF col;
    };

    class Sim {
    private:
        Ant* ants;
        int antsL;
        int* rate;
        COLORREF** field;
        const int width;
        const int height;
        const COLORREF background; 
        void step();
        void simulate();
    public:
        Sim(COLORREF** field, const int fieldW, const int fieldH, Ant* startAnts, int antSize, COLORREF background, int* rate);
        void startSimulationThread();
    };
#endif // SIM_HPP