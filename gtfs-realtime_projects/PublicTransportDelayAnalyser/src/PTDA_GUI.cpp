
#include "PTDA_GUI.hpp"




void GUI_drive() {
    
}


GUICL::GUICL(
    bool _init,
    size_t _programWidth,
    size_t _programHeight
) {
    dim__program.x = _programWidth;
    dim__program.y = _programHeight;

    if(_init) this->init();
}
GUICL::~GUICL() {
    if(!this->__init) throw std::runtime_error("GUICL::~GUICL() : destructor called without the class being initalised");

    ImGui_ImplAllegro5_Shutdown();
    ImGui::DestroyContext();
    al_destroy_event_queue(this->__queue);
    al_destroy_display(this->__display);

    this->__running = false;
    if(isDefined_callback_closing) callback_closing();
}

bool GUICL::init() {
    if(this->__init) {
        std::cerr << "GUINC::init() : Failed to initialised. Object has already called init().\n";
        return false;
    }

    al_init();
    al_install_keyboard();
    al_install_mouse();
    al_init_primitives_addon();
    al_set_new_display_flags(ALLEGRO_RESIZABLE);

    this->__display = al_create_display(dim__program.x, dim__program.y);
    al_set_window_title(this->__display, "PTDA_GUICL");
    this->__queue = al_create_event_queue();
    al_register_event_source(this->__queue, al_get_display_event_source(this->__display));
    al_register_event_source(this->__queue, al_get_keyboard_event_source());
    al_register_event_source(this->__queue, al_get_mouse_event_source());

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    if(!ImGui_ImplAllegro5_Init(this->__display)) {
        std::cerr << "ImGui_ImplAllegro5_Init(ALLEGRO_DISPLAY*) returned false.\n";
        return false;
    }

    this->__running = true;
    this->__init = true;

    return true;
}

void GUICL::newFrame() {
    if(!this->__init)       throw std::runtime_error("ERROR: GUICL::newFrame() : class object has not been initialised.");
    if(this->__frameStarted)throw std::runtime_error("ERROR: GUICL::newFrame() : member function has been called without an endFrame() call before this.");
    if(!this->__running)    throw std::runtime_error("ERROR: GUICL::newFrame() : member function has been called even though the class object isn't running.");

    ALLEGRO_EVENT al_event;
    while(al_get_next_event(this->__queue, &al_event)) {
        ImGui_ImplAllegro5_ProcessEvent(&al_event);
        if(al_event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            this->__running = false;
            if(this->isDefined_callback_closing) this->callback_closing();
            return;
        }
        if(al_event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
            ImGui_ImplAllegro5_InvalidateDeviceObjects();
            al_acknowledge_resize(this->__display);
            ImGui_ImplAllegro5_CreateDeviceObjects();
            dim__program.x = al_get_display_width(this->__display);
            dim__program.y = al_get_display_height(this->__display);
        }
    }

    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSizeConstraints(toImVec2(dim__program), toImVec2(dim__program));
    ImGui::Begin("main", NULL, this->winFlags_main);
    ImGui::SetWindowPos(ImVec2(0, 0));

    this->__running = true;
    this->__frameStarted = true;
}
void GUICL::endFrame() {
    if(!this->__init)       throw std::runtime_error("ERROR: GUICL::endFrame() : class object has not been initialised.");
    if(!this->__frameStarted)throw std::runtime_error("ERROR: GUICL::endFrame() : member function has been called without a newFrame() call before this.");
    if(!this->__running)    throw std::runtime_error("ERROR: GUICL::endFrame() : member function has been called even though the class object isn't running.");

    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetCursorPos(ImVec2(10, dim__program.y-25-ImGui::GetTextLineHeightWithSpacing()*1));
    ImGui::Text("Mouse pos:x:%3.1f y:%3.1f", io.MousePos.x, io.MousePos.y);
    ImGui::SetCursorPosX(10);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0/io.Framerate, io.Framerate);
    ImGui::End();

    ImGui::Render();

    ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
    al_flip_display();

    this->__frameStarted = false;
}

void GUICL::setCallback_closing(callbackType_closeSignal _callbackFunc) {
    this->callback_closing = _callbackFunc;
    isDefined_callback_closing = true;
}

bool GUICL::isRunning() const {
    return this->__running;
}
bool GUICL::isInit() const {
    return this->__init;
}


