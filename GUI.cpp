#include "GUI.hpp"
#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>


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
        case WM_MOUSEMOVE:
            ShowCursor(FALSE);  // Hide the cursor
            break;
        case WM_MOUSELEAVE:
            ShowCursor(TRUE);  // Show the cursor when it leaves the window
            break;
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
            if (!window->isDragging) {
                window->drawField();
            }
            break;
        case WM_ENTERSIZEMOVE: //start dragging
            window->isDragging = true;
            break;
        case WM_EXITSIZEMOVE: //stop dragging
            window->isDragging = false;
            window->setRedraw();
            break;
        case WM_SIZE: //must be minimize/fullsize etc
            window->setRedraw();
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

Window::Window(HINSTANCE hInstance, const char* title, int width, int height, COLORREF** field, const int wField, const int hField, COLORREF backgroundX, int fpsX)
    : fieldW(wField), fieldH(hField), background(backgroundX), fps(fpsX)
{
    /*
    //TODO: Use lpcwstring! 
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

    hwnd = CreateWindowEx(0, CLASS_NAME, title, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, this);

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    //TODO: init this in main and make the gui-init faster!
    this->field = field;
    this->last = new COLORREF*[hField];
    for (int h=0; h<hField; h++) {
        this->last[h] = new COLORREF[wField];
        for (int w=0; w<wField; w++) this->last[h][w] = field[h][w];
    }   
    this->redraw = false; //ignore last field and just redraw all?
    this->isDragging = false; //stop drawing while dragging (performance)

    this->hwnd = hwnd; //store simulation canvas

    ShowWindow(hwnd, SW_MAXIMIZE);
    UpdateWindow(hwnd);
}

void Window::setRedraw() {
    this->redraw = true;
}

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
    float blockW = width/(float)fieldW;
    float blockH = height/(float)fieldH;

    // draw the field
    for (int y=0; y<fieldH; y++) {
        for (int x=0; x<fieldW; x++) {
            COLORREF col = this->field[y][x]; //get stored color
            if (this->redraw || last[y][x]!=col) { //only redraw new things
                last[y][x] = col;
                // Select the pen into the device context
                HPEN hPen = CreatePen(PS_SOLID, 0, col); //border
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                // Select the brush into the device context
                HBRUSH hBrush = CreateSolidBrush(col); //fill
                SelectObject(hdc, hBrush);
                Rectangle(hdc, round(x*blockW), round(y*blockH), round(x*blockW+blockW), round(y*blockH+blockH)); // Draws a square with top-left corner at (50,50) and bottom-right corner at (100,100)
                //Clear up used objects
                DeleteObject(hPen);
                DeleteObject(hBrush);
            }
        }
    }
    if (this->redraw) this->redraw = false;
    // Clean up
    EndPaint(hwnd, &ps);
}

void Window::drawLoop() {
    int nanos = std::chrono::nanoseconds(std::chrono::seconds(1)).count(); 
    int secs = 5;
    int sample = secs*fps; //n * sec for each meassurement
    int n = 0; //count in range [0,sample)
    auto meassureStart = std::chrono::high_resolution_clock::now(); //store start of each meassurement
    auto lastTime = std::chrono::high_resolution_clock::now(); //store start of each while iteration
    auto waitNanos = std::chrono::duration<long long, std::nano>(std::chrono::nanoseconds(nanos/fps));

    while (true) { // or some condition for your simulation
        // Invalidate the window rect to trigger a WM_PAINT message
        InvalidateRect(this->hwnd , NULL, TRUE);

        // Sleep for the desired interval
        //get time now and check how long to Nanos
        auto now = std::chrono::high_resolution_clock::now();
        n = (n+1)%sample;
        if (n==0) { //the measurement is complete!
            //how many nanoseconds needed from first measurement
            auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(now-meassureStart).count();
            auto deltaSec = delta/secs;

            std::cout << "Rendered: " << fps*(nanos/(float)deltaSec) << "Fps ("<< 100*(nanos/(float)deltaSec) <<"%) Simulated: ";
            meassureStart = now; //reset for next now
        }

        auto nextStep = lastTime+waitNanos; //when is the next execution?
        if (nextStep>now) std::this_thread::sleep_for(nextStep-now); //Nanos for it

        lastTime = nextStep; //last time for next iteration
    }
}

void Window::startDrawThread() {
    // init thread to refresh screen
    std::thread drawThread(&Window::drawLoop, this);
    //async execution
    drawThread.detach(); 

}