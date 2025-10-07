
#include "PTDA_GUI.hpp"



bool GUINC::init() {
    if(GUINC::__init) {
        // throw std::runtime_error("GUINC::init() : Failed to initialise. Already has been initialised.");
        std::cerr << "GUINC::init() : Failed to initialise. Already has been initialised.\n";
        return false;
    }

    al_init();
    al_install_keyboard();
    al_install_mouse();
    al_init_primitives_addon();
    al_set_new_display_flags(ALLEGRO_RESIZABLE);

    GUINC::__display = al_create_display(dim__program.x, dim__program.y);
    al_set_window_title(GUINC::__display, "PTDA_GUINC");
    GUINC::__queue = al_create_event_queue();
    al_register_event_source(GUINC::__queue, al_get_display_event_source(GUINC::__display));
    al_register_event_source(GUINC::__queue, al_get_keyboard_event_source());
    al_register_event_source(GUINC::__queue, al_get_mouse_event_source());

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    if(!ImGui_ImplAllegro5_Init(GUINC::__display)) {
        std::cerr << "ImGui_ImplAllegro5_Init(ALLEGRO_DISPLAY*) returned false.\n";
        return false;
    }

    GUINC::__running = true;
    GUINC::__init = true;

    return true;
}
bool GUINC::close() {
    if(!GUINC::__init) {
        std::cerr << "GUINC::close() : Failed to the GUINC library. close() called without GUINC being initialised.\n";
        return false;
    }

    ImGui_ImplAllegro5_Shutdown();
    ImGui::DestroyContext();
    al_destroy_event_queue(GUINC::__queue);
    al_destroy_display(GUINC::__display);

    GUINC::__running = false;
    if(GUINC::isDefined_callback_closing) GUINC::callback_closing();
    return true;
}

void GUINC::_newFrame() {
    if(!GUINC::__init)       throw std::runtime_error("ERROR: GUINC::newFrame() : class object has not been initialised.");
    if(GUINC::__frameStarted)throw std::runtime_error("ERROR: GUINC::newFrame() : member function has been called without an endFrame() call before this.");
    if(!GUINC::__running)    throw std::runtime_error("ERROR: GUINC::newFrame() : member function has been called even though the class object isn't running.");

    ALLEGRO_EVENT al_event;
    while(al_get_next_event(GUINC::__queue, &al_event)) {
        ImGui_ImplAllegro5_ProcessEvent(&al_event);
        if(al_event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            GUINC::run = false;
            if(GUINC::isDefined_callback_closing) GUINC::callback_closing();
            return;
        }
        if(al_event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
            ImGui_ImplAllegro5_InvalidateDeviceObjects();
            al_acknowledge_resize(GUINC::__display);
            ImGui_ImplAllegro5_CreateDeviceObjects();
            dim__program.x = al_get_display_width(GUINC::__display);
            dim__program.y = al_get_display_height(GUINC::__display);
        }
    }

    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSizeConstraints(toImVec2(dim__program), toImVec2(dim__program));
    ImGui::Begin("main", NULL, GUINC::winFlags_Main_default);
    ImGui::SetWindowPos(ImVec2(0, 0));

    GUINC::__running = true;
    GUINC::__frameStarted = true;
}
void GUINC::_endFrame() {
    if(!GUINC::__init)       throw std::runtime_error("ERROR: GUINC::endFrame() : class object has not been initialised.");
    if(!GUINC::__frameStarted)throw std::runtime_error("ERROR: GUINC::endFrame() : member function has been called without a newFrame() call before this.");
    if(!GUINC::__running)    throw std::runtime_error("ERROR: GUINC::endFrame() : member function has been called even though the class object isn't running.");

    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetCursorPos(ImVec2(10, dim__program.y-25-ImGui::GetTextLineHeightWithSpacing()*1));
    ImGui::Text("Mouse pos:x:%3.1f y:%3.1f", io.MousePos.x, io.MousePos.y);
    ImGui::SetCursorPosX(10);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0/io.Framerate, io.Framerate);
    ImGui::End();

    ImGui::Render();

    ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
    al_flip_display();

    GUINC::__frameStarted = false;
}




void GUINC::Drive() {

    if(!GUINC::init()) {
        exit(1);
    }

    try {
        
        while(GUINC::run.load()) {
            GUINC::_newFrame();
    
    
            GUINC::_endFrame();
        }
        
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        exit(1);
    }

    if(!GUINC::close()) {
        exit(1);
    }

}


