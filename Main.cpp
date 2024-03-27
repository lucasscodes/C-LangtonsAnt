#include "GUI.hpp"
#include "Sim.hpp"
#include <windows.h>
#include <cassert>
#include <iostream>
#include <vector>
#include "gperftools-master/src/gperftools/profiler.h"

//start simulation and drawings in parallel/threaded mode
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char winTtl[] = "Langtons Ant";
    //field dimensions
    const int height = 512;
    const int width = 512;
    //background color
    COLORREF background = RGB(255,255,255);
    int rate = 1000; //Hz of ants
    //get fps from monitor or fallback to hardcoded
    DEVMODE dm;
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
    int fps = dm.dmDisplayFrequency;

    // Create a dynamic 2D array
    COLORREF** field = new COLORREF*[height];
    for(int i = 0; i < height; ++i) {
        field[i] = new COLORREF[width];
        // Fill the array
        for (int j = 0; j<width; j++) {
            field[i][j] = background;
        }
    }

    Ant ant;
    ant.y = height/2;
    ant.x = width/2;
    ant.dir = 0;
    ant.col = RGB(255,0,0);

    Ant ant2;
    ant2.y = height/2;
    ant2.x = width/2;
    ant2.dir = 1;
    ant2.col = RGB(0,255,0);

    Ant ant3;
    ant3.y = height/2;
    ant3.x = width/2;
    ant3.dir = 2;
    ant3.col = RGB(0,0,255);

    Ant ant4;
    ant4.y = height/2;
    ant4.x = width/2;
    ant4.dir = 3;
    ant4.col = RGB(0,0,0);

    Ant ants[] = {ant,ant2,ant3,ant4};
    int antsL = 0;
    for (Ant ant : ants) {
        antsL++;
        assert(ant.col!=background);
    }
    std::vector<COLORREF> cols(antsL+1);
    for (int i=0; i<antsL; i++) {
        cols[i] = ants[i].col;
    }
    cols[antsL] = background;

    // Init the simulation
    Sim langtonsAnt(field, width, height, ants, antsL, background, &rate);
    //TODO: Use this profiler to optimize
    //ProfilerStart("./executionProfiler.txt");
    // Init the GUI
    Window window(hInstance, winTtl, 0, 0, field, width, height, background, fps, cols);
    // init thread to refresh screen
    window.startDrawThread();
    //ProfilerStop();
    // init thread to simulate ants
    langtonsAnt.startSimulationThread();


    // Run the message loop, ask GPT how this works^^
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //free memory
    for(int i = 0; i < height; ++i) delete [] field[i];
    delete [] field;

    //TODO: Init "lastField" in main and also clean up used memory

    return 0;
}