
#include <lazyimgui.hpp>

#include <string>
#include <fstream>
#include <cmath>

#include <map>
#include <vector>


Pos2d<int> dim_win{1000, 700};
Pos2d<int> dim_subwin_stopIDs{300, 700};
Pos2d<int> dim_subwin_graph{700, 700};

Pos2d<int> pos_subwin_stopIDs{0, 0};
Pos2d<int> pos_subwin_graph{300, 0};


int main(int argc, char** argv) {
    std::string path_delaysFile = "";
    std::ifstream file_delaysFile;

    if(argc>1) {
        path_delaysFile = argv[1];
        // for(size_t i=1; i<argc; i++) {
        // }
    }
    else {
        std::cerr << "invalid program start. Missing delaysFile path arg.\n";
        return 1;
        // path_delaysFile = "";
    }
    file_delaysFile.open(path_delaysFile);
    if(!file_delaysFile.is_open()) {
        std::cerr << "failed to open delaysFile.\n";
        return 1;
    }


    std::map<std::string, std::map<std::string, std::vector<std::string>>> loadedData_main;
    for(std::string _line; std::getline(file_delaysFile, _line);) {
        size_t commaPos = 0, _tempPos = 0;
        std::string str__epoch_time = _line.substr(0, (commaPos=_line.find(',')));
        std::string str__trip_id    = _line.substr(commaPos+1, (_tempPos=_line.find(',', commaPos+1))-commaPos-1);
        commaPos = _tempPos;
        std::string str__STU_idx    = _line.substr(commaPos+1, (_tempPos=_line.find(',', commaPos+1))-commaPos-1);
        commaPos = _tempPos;
        std::string str__stop_seq   = _line.substr(commaPos+1, (_tempPos=_line.find(',', commaPos+1))-commaPos-1);
        commaPos = _tempPos;
        std::string str__stop_id    = _line.substr(commaPos+1, (_tempPos=_line.find(',', commaPos+1))-commaPos-1);
        commaPos = _tempPos;
        std::string str__delay_arr  = _line.substr(commaPos+1, (_tempPos=_line.find(',', commaPos+1))-commaPos-1);
        commaPos = _tempPos;
        std::string str__delay_dep  = _line.substr(commaPos+1, _line.find('\n')-commaPos-1);

        loadedData_main[str__stop_id][str__trip_id] = {str__epoch_time, str__delay_arr, str__delay_dep};
    }


    GUINC::lazyimgui guiwin(false, 1000, 700);
    if(!guiwin.init()) {
        std::cerr << "could not init lazyimgui instance.\n";
        return 1;
    }
    while(guiwin.running()) {
        guiwin.newFrame();

        ImGui::SetCursorPos(ImVec2(pos_subwin_stopIDs.x, pos_subwin_stopIDs.y));
        if(ImGui::BeginChild("child_stop_id", ImVec2(dim_subwin_stopIDs.x, dim_subwin_stopIDs.y), ImGuiChildFlags_Border)) {


            ImGui::EndChild();
        }
        ImGui::SetCursorPos(ImVec2(pos_subwin_graph.x, pos_subwin_graph.y));
        if(ImGui::BeginChild("child_graph", ImVec2(dim_subwin_graph.x, dim_subwin_graph.y), ImGuiChildFlags_Border)) {

            ImGui::EndChild();
        }
        

        guiwin.endFrame();
    }
    return 0;
}