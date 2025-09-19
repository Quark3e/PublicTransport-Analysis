#pragma once
#ifndef HPP__PTDA_GUI
#define HPP__PTDA_GUI

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_allegro5.h>

#include "Pos2d.hpp"


template<typename _VAR>
inline ImVec2 toImVec2(Pos2d<_VAR> toConv) {
    return ImVec2(float(toConv.x), float(toConv.y));
}
inline Pos2d<float> toPos2d(ImVec2 toConv) {
    return Pos2d<float>(toConv.x, toConv.y);
}


Pos2d<float> dim__program{1000, 700};
Pos2d<float> dim_win0_stopID_selects{100, 500};
Pos2d<float> dim_win0_graphWin{900, 500};

Pos2d<float> pos_win0_stopID_selects{0, 0};
Pos2d<float> pos_win0_graphWin{dim_win0_stopID_selects.x, 0};


class GUICL {
    private:
    ALLEGRO_DISPLAY*        __display = nullptr;
    ALLEGRO_EVENT_QUEUE*    __queue = nullptr;

    bool __frameStarted = false;
    bool __init = false;
    bool __running = false;

    public:
    ImGuiWindowFlags winFlags_main = 
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoBringToFrontOnFocus | 
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBackground
    ;

    GUICL(bool _init=true, size_t _width=dim__program.x, size_t _height=dim__program.y);
    ~GUICL();

    bool init();

    void newFrame();
    void endFrame();

    void mainDrive();

    const bool running();

    void exit();

    void (*callback_func__running_exit)(void) = nullptr;


};

#endif //HPP__PTDA_GUI