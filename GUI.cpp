#include "GUI.hpp"
#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <omp.h>
#include <unordered_map>
#include <vector>
#include <cassert>


LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Window *window = nullptr;

    if (msg == WM_CREATE) {
        CREATESTRUCT *createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = static_cast<Window*>(createStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    switch(msg) {
        //Disable cursor
        case WM_MOUSEMOVE:
            ShowCursor(FALSE);  // Hide the cursor
            break;
        case WM_MOUSELEAVE:
            ShowCursor(TRUE);  // Show the cursor when it leaves the window
            break;
        //Exit screensaver with ESC
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)  // VK_ESCAPE is the virtual key code for the Escape key
            {
                PostMessage(hwnd, WM_CLOSE, 0, 0);  // Send a WM_CLOSE message to the window
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_PAINT:
            window->drawField();
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

Window::Window(HINSTANCE hInstance, const char* title, int width, int height, COLORREF** fieldX, 
               const int wField, const int hField, COLORREF backgroundX, int fpsX, std::vector<COLORREF> cols)
    : fieldW(wField), fieldH(hField), background(backgroundX), fps(fpsX)
{
    /*
    //TODO: Use lpcwstring!?
    const char test[] = "abc";
    std::string str(test);
    std::wstring wstr(str.begin(), str.end());
    LPCWSTR testLpcwstr = wstr.c_str(); 
    */
    const char CLASS_NAME[] = "Simulation Frame";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return;
    }


    //TODO: init this in main and make the gui-init faster!
    this->field = fieldX;
    this->last = new COLORREF*[hField];
    for (int h=0; h<hField; h++) {
        this->last[h] = new COLORREF[wField];
        for (int w=0; w<wField; w++) this->last[h][w] = field[h][w];
    }   
    
    //TODO: Why is all breaking apart without this initially?
    this->redraw = true; //ignore last field and just redraw all?


    //This initializes brushes+pens for all colors
    std::unordered_map<COLORREF, std::tuple<HPEN, HBRUSH>> map;
    this->brushAndPen = map;
    for (COLORREF col : cols) {
        assert(this->brushAndPen.find(col) == this->brushAndPen.end()); //we only want to add colors once
        HPEN hPen = CreatePen(PS_SOLID, 0, col); //border
        HBRUSH hBrush = CreateSolidBrush(col); //fill
        std::tuple<HPEN, HBRUSH> e = {hPen, hBrush};
        this->brushAndPen.insert({col, e});
    }

    hwnd = CreateWindowEx(0, CLASS_NAME, title, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, this);

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    this->hwnd = hwnd; //store simulation canvas
}

//usable for resizable windows of this exe, but slow and bad like the init draw
void Window::setRedraw() {
    this->redraw = true;
}

//draw the box if needed
void Window::updateBoxIfNeeded(int y, int x, HDC hdc, int top, int left, int bottom, int right) {
    if (this->redraw || this->last[y][x]!=this->field[y][x]) { //only redraw new things
        COLORREF col = this->field[y][x]; //get stored color
        //store in buffer to only draw once
        this->last[y][x] = col;

        //tried to not creade/destroy brush+pen, but was still slow (or no deletions were still inside but the pen+brush NOT MISSING!)
        
        auto entry = *(this->brushAndPen.find(col));
        std::tuple<HPEN, HBRUSH> pair = entry.second;
        HPEN hPen = std::get<0>(pair);
        HBRUSH hBrush = std::get<1>(pair); 
        
        // Select the pen into the device context
        //SelectObject(hdc, hPen);
        // Select the brush into the device context
        SelectObject(hdc, hBrush);

        // Draws a square with top-left corner at (50,50) and bottom-right corner at (100,100)
            // Define the rectangle
        RECT rect;
        rect.left = left;
        rect.top = top;
        rect.right = right;
        rect.bottom = bottom;
            // Fill the rectangle
        FillRect(hdc, &rect, hBrush);

    }
};

void Window::drawField() {
    HWND hwnd = this->hwnd;
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    // Set the color of the brush
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    // Now you can use width and height
    double blockW = width/(float)fieldW;
    double blockH = height/(float)fieldH;
    // draw the field
    //TODO: get it in parallel with, but something else is slower...
    //#pragma omp parallel for
    for (int y=0; y<fieldH; y++) {
        for (int x=0; x<fieldW; x++) {
            updateBoxIfNeeded(y,x,hdc,round(y * blockH), round(x * blockW), round((y + 1) * blockH), round((x + 1) * blockW));
        }
    }

    this->redraw = false;

    // Clean up
    EndPaint(hwnd, &ps);
}

//TODO: This is the same as the simulation loop, extract lambda/functions inside and rate
//Then move to own class and use zentral implementation
void Window::drawLoop() {
    int nanos = std::chrono::nanoseconds(std::chrono::seconds(1)).count(); 
    int secs = 5;
    int n = 0; //count in range [0,sample)
    auto meassureStart = std::chrono::high_resolution_clock::now(); //store start of each meassurement
    auto lastTime = std::chrono::high_resolution_clock::now(); //store start of each while iteration
    auto waitNanos = std::chrono::duration<long long, std::nano>(std::chrono::nanoseconds(nanos/fps));

    while (true) { // or some condition for your simulation
        InvalidateRect(hwnd, NULL, false); //only ask for maximal fps given by system

        // Sleep for the desired interval
        //get time now and check how long to Nanos
        auto now = std::chrono::high_resolution_clock::now();
        auto nextStep = lastTime+waitNanos; //when is the next execution?
        if (nextStep>now) std::this_thread::sleep_for(nextStep-now); //Nanos for it

        lastTime = nextStep; //last time for next iteration
    }
}

void Window::startDrawThread() {
    ShowWindow(hwnd, SW_MAXIMIZE);
    UpdateWindow(hwnd);
    // init thread to refresh screen
    std::thread drawThread(&Window::drawLoop, this);
    //async execution
    drawThread.detach(); 
}