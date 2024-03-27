#include "GUI.hpp"
#include "Sim.hpp"
#include <windows.h>

//start simulation and drawings in parallel/threaded mode
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char winTtl[] = "Langtons Ant";
    const int height = 512;
    const int width = 512;
    COLORREF background = RGB(255,255,255);
    int fps = 30;
    int rate = 50000;

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
    int antsL = sizeof(ants)/sizeof(ants[0]);


    // Init the GUI
    Window window(hInstance, winTtl, 1000, 1000, field, width, height, background, fps);
    // init thread to refresh screen
    window.show();

    //init langtonsAnt
    Sim langtonsAnt(field, width, height, ants, antsL, background, &rate);
    // init thread to simulate ants
    langtonsAnt.startSimulationThread();


    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //free memory
    for(int i = 0; i < height; ++i) delete [] field[i];
    delete [] field;

    return 0;
}