#include "Sim.hpp"
#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>

Sim::Sim(COLORREF** fieldX, const int fieldW, const int fieldH, Ant* startAnts, int antSize, COLORREF backgroundX, int* rateX) 
    : width(fieldW), height(fieldH), background(backgroundX)
{
    this->field = fieldX;
    this->rate = rateX;

    this->ants = startAnts;
    this->antsL = antSize;
}

void Sim::step() {
    for (int i=0; i<this->antsL; i++) {
        int y = ants[i].y;
        int x = ants[i].x;
        COLORREF col = ants[i].col;
        ants[i].dir += field[y][x]==background?1:3;
        ants[i].dir %= 4;
        field[y][x] = field[y][x]==background?col:background;
        switch (ants[i].dir) {
            case 0:
                y += height-1;
                break;
            case 1:
                x += 1;
                break;
            case 2:
                y += 1;
                break;
            case 3: 
                x += width-1;
                break;
            default: 
                std::cout << "BROKEN ANT DIR!\n";
        }
        y %= height;
        x %= width;
        ants[i].y = y;
        ants[i].x = x;
    }
}

void Sim::simulate() {
    int nanos = std::chrono::nanoseconds(std::chrono::seconds(1)).count(); 
    int secs = 5;
    int sample = secs*(*this->rate); //n * sec for each meassurement
    int n = 0; //count in range [0,sample)
    auto meassureStart = std::chrono::high_resolution_clock::now(); //store start of each meassurement
    auto lastTime = std::chrono::high_resolution_clock::now(); //store start of each while iteration

    while (true) { // or some condition for your simulation
        step();
        // Sleep for the desired interval
        int rateNow = *this->rate; //get the dynamic rate
        auto waitNanos = std::chrono::duration<long long, std::nano>(std::chrono::nanoseconds(nanos/rateNow));
        //get time now and check how long to Nanos
        auto now = std::chrono::high_resolution_clock::now();
        n = (n+1)%sample;
        if (n==0) { //the measurement is complete!
            //how many nanoseconds needed from first measurement
            auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(now-meassureStart).count();
            auto deltaSec = delta/secs;

            std::cout << (*this->rate)*(nanos/(float)deltaSec) << "Hz ("<< 100*(nanos/(float)deltaSec) <<"%)\n";
            meassureStart = now; //reset for next now
        }

        auto nextStep = lastTime+waitNanos; //when is the next execution?
        if (nextStep>now) std::this_thread::sleep_for(nextStep-now); //Nanos for it

        lastTime = nextStep; //last time for next iteration
    }
}

void Sim::startSimulationThread() {
    std::thread simThread(&Sim::simulate, this);
    simThread.detach();
}