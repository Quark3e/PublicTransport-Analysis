#pragma once
#ifndef HPP__PTDA_GUI
#define HPP__PTDA_GUI

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_allegro5.h>

#include "Pos2d.hpp"

namespace GUINC {

    template<typename _VAR>
    inline ImVec2 toImVec2(Pos2d<_VAR> toConv) {
        return ImVec2(float(toConv.x), float(toConv.y));
    }
    inline Pos2d<float> toPos2d(ImVec2 toConv) {
        return Pos2d<float>(toConv.x, toConv.y);
    }

    ALLEGRO_DISPLAY*        __display = nullptr;
    ALLEGRO_EVENT_QUEUE*    __queue = nullptr;

    

};

#endif //HPP__PTDA_GUI