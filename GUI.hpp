// GUI.hpp
#ifndef GUI_HPP
    #define GUI_HPP

    #include <windows.h>
    #include <vector>
    #include <unordered_map>

    class Window {
    private:
        std::unordered_map<COLORREF, std::tuple<HPEN, HBRUSH>> brushAndPen;
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
        //draw a single box
        void updateBoxIfNeeded(int y, int x, HDC hdc, int top, int left, int bottom, int right);
        //(re)draw field
        void drawField();
        //threaded timed execution of draws
        void drawLoop();
        //ignore last field and redraw all positions
        void setRedraw();
    public:
        Window(HINSTANCE hInstance, const char* title, int width, int height, COLORREF** fieldX, const int wField, const int hField, COLORREF backgroundX, int fpsX, std::vector<COLORREF> cols);
        void startDrawThread();
    };
#endif // GUI_HPP