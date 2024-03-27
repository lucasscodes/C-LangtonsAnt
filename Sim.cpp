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
    int sample = secs*(*this->rate); //wait 1sec for each meassurement
    int n = 0; //count in range [0,sample)
    auto meassure = std::chrono::high_resolution_clock::now(); //store start of each meassurement
    auto lastTime = std::chrono::high_resolution_clock::now(); //store start of each while iteration

    while (true) { // or some condition for your simulation
        step();
        // Sleep for the desired interval
        int rateNow = *this->rate; //get the dynamic rate
        auto wait = std::chrono::duration<long long, std::nano>(std::chrono::nanoseconds(nanos/rateNow));
        auto sleep = lastTime+wait;
        //get time now and check how long to wait
        auto time = std::chrono::high_resolution_clock::now();
        n = (n+1)%sample;
        if (n==0) {
            //how many nanoseconds needed from first measurement
            auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(time-meassure).count();
            
            std::cout << "Rate:"+(*this->rate) << (float)delta << secs*nanos << "  :  ";
            std::cout << (*this->rate)*((float)delta/secs*nanos) << "Hz\n";
            meassure = time; //reset for next time
        }
        if (sleep>time) std::this_thread::sleep_for(sleep-time);

        lastTime = sleep;
    }
}

void Sim::startSimulationThread() {
    std::thread simThread(&Sim::simulate, this);
    simThread.detach();
}