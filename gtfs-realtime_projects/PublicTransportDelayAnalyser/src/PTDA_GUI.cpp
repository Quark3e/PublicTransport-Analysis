
#include "PTDA_GUI.hpp"




void GUICL::mainDrive() {

}


GUICL::GUICL(
    bool _init,
    size_t _width,
    size_t _height
) {

    if(_init) this->init();
}
GUICL::~GUICL() {

}

bool GUICL::init() {

}

void GUICL::newFrame() {

}
void GUICL::endFrame() {

}

const bool GUICL::running() {
    return this->__running;
}

void GUICL::exit() {

}

void GUICL::setCallback_closing(callbackType_closeSignal _callbackFunc) {
    this->callback_closing = _callbackFunc;
    isDefined_callback_closing = true;
}
