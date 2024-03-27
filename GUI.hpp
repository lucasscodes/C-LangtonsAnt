// GUI.hpp
#ifndef GUI_HPP
    #define GUI_HPP

    #include <windows.h>

    class Window {
    private:
        HWND hwnd;
        COLORREF** field;
        COLORREF** last;
        COLORREF background;
        bool redraw;
        bool isDragging;
        const int fps;
        const int fieldW;
        const int fieldH;
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        //(re)draw field
        void drawField();
        //threaded timed execution of draws
        void drawLoop();
        //ignore last field and redraw all positions
        void setRedraw();
    public:
        Window(HINSTANCE hInstance, const char* title, int width, int height, COLORREF** field, const int fieldW, const int fieldH, COLORREF background, int fps);
        void show();
    };
#endif // GUI_HPP