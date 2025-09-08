
#include <lazyimgui.hpp>
#include <includes.hpp>


int main(int argc, char** argv) {

    GUINC::lazyimgui guiwin(false, 1000, 700);

    if(!guiwin.init()) {
        std::cerr << "could not init lazyimgui instance.\n";
        return 1;
    }
    while(guiwin.running()) {
        guiwin.newFrame();

        

        guiwin.endFrame();
    }
    return 0;
}