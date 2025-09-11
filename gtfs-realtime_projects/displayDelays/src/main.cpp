
#include <lazyimgui.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <string>
#include <fstream>
#include <cmath>

#include <map>
#include <vector>

#include <chrono>


inline std::vector<int> convert_RGB_HSV(std::vector<int> _RGB);
/**
 * @brief Convert HSV values to RGB values
 * 
 * @param HSV float values of HSV (Hue, Saturation, Value) (degrees, percentage, percentage) [0-360, 0-100, 0-100]
 * @return std::vector<float> of the HSV values [0-255]
 */
inline std::vector<float> convert_HSV_RGB(std::vector<float> HSV);
/**
 * @brief Get the index of a desired value
 * 
 * @tparam varType value type
 * @param toCheck the container to check
 * @param toFind what to find.
 * ```
 * - `0` - biggest value
 * - `1` - smallest value
 * ```
 * @return size_t of the desired element index
 */
template<typename varType>
inline size_t findIdx(std::vector<varType> toCheck, int toFind);


Pos2d<int> dim_win{1500, 1000};
Pos2d<int> dim_subwin_stopIDs{200, 700};
Pos2d<int> dim_subwin_graph{dim_win.x-dim_subwin_stopIDs.x, 700};

Pos2d<int> pos_subwin_stopIDs{0, 0};
Pos2d<int> pos_subwin_graph{dim_subwin_stopIDs.x, 0};


class plotMeth {
    private:

    // px/mm [virtual/real]
    Pos2d<double> axisScalar;

    Pos2d<double> range_min;
    Pos2d<double> range_max;

    Pos2d<float> drawDim;
    Pos2d<float> drawOffsetPos;

    ImColor col_axes        = IM_COL32(20, 20, 20, 180);
    ImColor col_grid        = IM_COL32(80, 80, 80, 180);
    float   thickness_axes  = 1.5;
    float   thickness_grid  = 0.8;

    public:
    plotMeth(Pos2d<float> _drawDim, Pos2d<double> _minRange={-100, -100}, Pos2d<double> _maxRange={100, 100}, Pos2d<float> _posDrawOffset={0, 0}):
        drawDim(_drawDim), range_min(_minRange), range_max(_maxRange), drawOffsetPos(_posDrawOffset)
    {
        axisScalar.x = double(drawDim.x)/(range_max.x-range_min.x);
        axisScalar.y = double(drawDim.y)/(range_max.y-range_min.y);
    }
    ~plotMeth() {}

    void set_xRange(double _min, double _max) {
        range_min.x = _min;
        range_max.x = _max;

        axisScalar.x = double(drawDim.x)/(_max-_min);
    }
    void set_yRange(double _min, double _max) {
        range_min.y = _min;
        range_max.y = _max;

        axisScalar.y = double(drawDim.y)/(_max-_min);
    }
    
    void draw_plot(std::vector<double> x_values, std::vector<double> y_values, size_t plotMethod=0, ImColor plotColour=IM_COL32(200,100,120,170)) {
        if(x_values.size()==0 || y_values.size()==0) throw std::runtime_error("draw_plot : axis plot value containers cannot be empty.");
        if(x_values.size()!=y_values.size()) throw std::runtime_error("draw_plot : x and y values aren't the same size.");
        
        auto drawList = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 posOffs= ImVec2(winPos.x, winPos.y);
        
        ImVec2 prevPlotPos(
            posOffs.x +             (x_values.at(0)-range_min.x)*axisScalar.x, 
            posOffs.y + drawDim.y - (y_values.at(0)-range_min.y)*axisScalar.y
        );
        switch (plotMethod) {
            case 0: // dot/scatter plot
                drawList->AddCircleFilled(prevPlotPos, 5, plotColour, 10);
                break;
            case 1: // line plot
                break;
            case 2: // bar plot
                drawList->AddRectFilled(ImVec2(prevPlotPos.x-2, prevPlotPos.y), ImVec2(prevPlotPos.x+2, winPos.y+drawDim.y-std::abs(range_min.y)*axisScalar.y), plotColour);
                break;
            default:
                throw std::runtime_error("draw_plot : plotMethod value is invalid.");
                break;
        }
        for(size_t i=1; i<x_values.size(); i++) {
            ImVec2 plotPos(
                posOffs.x +             (x_values.at(i)-range_min.x)*axisScalar.x,
                posOffs.y + drawDim.y - (y_values.at(i)-range_min.y)*axisScalar.y
            );
            switch (plotMethod) {
                case 0: // dot/scatter plot
                    drawList->AddCircleFilled(plotPos, 5, plotColour, 10);
                    break;
                case 1: // line plot
                    drawList->AddLine(prevPlotPos, plotPos, plotColour);
                    break;
                case 2: // bar plot
                    drawList->AddRectFilled(ImVec2(plotPos.x-2, plotPos.y), ImVec2(plotPos.x+2, winPos.y+drawDim.y-std::abs(range_min.y)*axisScalar.y), plotColour);
                    break;
                default:
                    throw std::runtime_error("draw_plot : plotMethod value is invalid.");
                    break;
            }
            prevPlotPos = plotPos;
        }

    }
    
    bool beginDraw(/*bool drawGrid=true, */ImColor bgColour=IM_COL32(240,240,240,200)) {
        ImVec2 winPosOffs = ImGui::GetWindowPos();
        ImVec2 posOffs = ImVec2(winPosOffs.x+drawOffsetPos.x, winPosOffs.y+drawOffsetPos.y);

        Pos2d<int>      num_grid_lines{10, 10};
        Pos2d<double>   grid_line_spac = Pos2d<double>{range_max.x-range_min.x, range_max.y-range_min.y}/num_grid_lines.cast<double>();

        ImGui::SetCursorPos(ImVec2(drawOffsetPos.x, drawOffsetPos.y));
        if(!ImGui::BeginChild("win_plotArea", ImVec2(drawDim.x, drawDim.y), ImGuiChildFlags_NavFlattened)) return false;

        auto drawList = ImGui::GetWindowDrawList();
        // if(drawGrid) {
        //     for(double x=0; x<=range_max.x; x+=grid_line_spac.x) {
        //         if(x<range_min.x) x = std::floor(range_min.x/grid_line_spac.x)*grid_line_spac.x;
        //         ImVec2 temp_pos(posOffs.x+(x-range_min.x)*axisScalar.x, posOffs.y);
        //         drawList->AddLine(
        //             ImVec2(temp_pos.x, temp_pos.y),
        //             ImVec2(temp_pos.x, temp_pos.y+drawDim.y),
        //             col_grid, thickness_grid
        //         );
        //     }
        //     for(double x=0; x>=range_min.x; x-=grid_line_spac.x) {
        //         if(x>range_max.x) x = std::ceil(range_max.x/grid_line_spac.x)*grid_line_spac.x;
        //         ImVec2 temp_pos(posOffs.x+(x-range_min.x)*axisScalar.x, posOffs.y);
        //         drawList->AddLine(
        //             ImVec2(temp_pos.x, temp_pos.y),
        //             ImVec2(temp_pos.x, temp_pos.y+drawDim.y),
        //             col_grid, thickness_grid
        //         );
        //     }
        //     for(double y=0; y<=range_max.y; y+=grid_line_spac.y) {
        //         if(y<range_min.y) y = std::floor(range_min.y/grid_line_spac.y)*grid_line_spac.y;
        //         ImVec2 temp_pos(posOffs.x, posOffs.y+drawDim.y-(y-range_min.y)*axisScalar.y);
        //         drawList->AddLine(
        //             ImVec2(temp_pos.x, temp_pos.y),
        //             ImVec2(temp_pos.x+drawDim.x, temp_pos.y),
        //             col_grid, thickness_grid
        //         );
        //     }
        //     for(double y=0; y>=range_min.y; y-=grid_line_spac.y) {
        //         if(y>range_max.y) y = std::ceil(range_max.y/grid_line_spac.y)*grid_line_spac.y;
        //         ImVec2 temp_pos(posOffs.x, posOffs.y+drawDim.y-(y-range_min.y)*axisScalar.y);
        //         drawList->AddLine(
        //             ImVec2(temp_pos.x, temp_pos.y),
        //             ImVec2(temp_pos.x+drawDim.x, temp_pos.y),
        //             col_grid, thickness_grid
        //         );
        //     }
        // }

        drawList->AddLine(
            ImVec2(posOffs.x, posOffs.y+drawDim.y-std::abs(range_min.y)*axisScalar.y),
            ImVec2(posOffs.x+drawDim.x, posOffs.y+drawDim.y-std::abs(range_min.y)*axisScalar.y),
            col_axes, thickness_axes
        );
        drawList->AddLine(
            ImVec2(posOffs.x+std::abs(range_min.x)*axisScalar.x, posOffs.y),
            ImVec2(posOffs.x+std::abs(range_min.x)*axisScalar.x, posOffs.y+drawDim.y),
            col_axes, thickness_axes
        );
        
        drawList->AddLine(
            ImVec2(posOffs.x, posOffs.y),
            ImVec2(posOffs.x, posOffs.y+drawDim.y),
            col_grid, 2
        );
        drawList->AddLine(
            ImVec2(posOffs.x, posOffs.y),
            ImVec2(posOffs.x+drawDim.x, posOffs.y),
            col_grid, 2
        );
        drawList->AddLine(
            ImVec2(posOffs.x+drawDim.x-2, posOffs.y),
            ImVec2(posOffs.x+drawDim.x-2, posOffs.y+drawDim.y),
            col_grid, 2
        );
        drawList->AddLine(
            ImVec2(posOffs.x, posOffs.y+drawDim.y),
            ImVec2(posOffs.x+drawDim.x, posOffs.y+drawDim.y),
            col_grid, 2
        );

        return true;
    }

    void endDraw() {
        
        ImGui::EndChild();
    }


    void draw_xLabels(std::vector<std::string> axis_labels, std::vector<double> axis_value, bool drawGrid=true, float tilt_label=0, ImVec2 axis_offset=ImVec2(0,0)) {
        if(axis_labels.size()==0 || axis_value.size()==0) throw std::runtime_error("draw_xLabels : vector size's cannot be 0.");
        if(axis_labels.size() != axis_value.size()) throw std::runtime_error("draw_xLabels : vector sizes cannot be different.");

        auto drawList = ImGui::GetWindowDrawList();
        ImVec2 winPosOffs = ImGui::GetWindowPos();
        // ImVec2 winPosOffs = ImVec2(0, 0);
        ImVec2 textPos(0, 0);
        for(size_t i=0; i<axis_labels.size(); i++) {
            float xPos = (axis_value.at(i)-range_min.x)*axisScalar.x;
            textPos = ImVec2(
                drawOffsetPos.x + xPos + axis_offset.x,
                drawOffsetPos.y + drawDim.y + axis_offset.y
            );
            if(drawGrid) drawList->AddLine(
                ImVec2(drawOffsetPos.x+winPosOffs.x+xPos, drawOffsetPos.y+winPosOffs.y),
                ImVec2(drawOffsetPos.x+winPosOffs.x+xPos, drawOffsetPos.y+winPosOffs.y+drawDim.y),
                col_grid, thickness_grid
            );
            ImGui::SetCursorPos(textPos);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(10,20,10,250));
            ImGui::Text(axis_labels.at(i).c_str());
            ImGui::PopStyleColor();
        }
    }
    void draw_yLabels(std::vector<std::string> axis_labels, std::vector<double> axis_value, bool drawGrid=true, float tilt_label=0, ImVec2 axis_offset=ImVec2(-5,0)) {
        if(axis_labels.size()==0 || axis_value.size()==0) throw std::runtime_error("draw_yLabels : vector size's cannot be 0.");
        if(axis_labels.size() != axis_value.size()) throw std::runtime_error("draw_yLabels : vector sizes cannot be different.");

        auto drawList = ImGui::GetWindowDrawList();
        ImVec2 winPosOffs = ImGui::GetWindowPos();
        // ImVec2 winPosOffs = ImVec2(0, 0);
        ImVec2 textPos(0, 0);
        for(size_t i=0; i<axis_labels.size(); i++) {
            if(axis_value.at(i)<range_min.y || axis_value.at(i)>range_max.y) continue;
            float yPos = drawDim.y - (axis_value.at(i)-range_min.y)*axisScalar.y;
            textPos = ImVec2(
                drawOffsetPos.x + axis_offset.y,
                drawOffsetPos.y + yPos + axis_offset.y
            );
            if(drawGrid) drawList->AddLine(
                ImVec2(drawOffsetPos.x+winPosOffs.x, drawOffsetPos.y+winPosOffs.y+yPos),
                ImVec2(drawOffsetPos.x+winPosOffs.x+drawDim.x, drawOffsetPos.y+winPosOffs.y+yPos),
                col_grid, thickness_grid
            );
            ImGui::SetCursorPos(textPos);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(10,20,10,250));
            ImGui::Text(axis_labels.at(i).c_str());
            ImGui::PopStyleColor();
        }
    }

};

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

    plotMeth guiPlot(dim_subwin_graph.cast<float>()-Pos2d<float>(50,50), {-100,100}, {100,100}, {25,25});

    Pos2d<int64_t> absRange_x_epoch(0, 0);
    Pos2d<int32_t> absRange_y_delay(0, 0);
    std::map<std::string, std::map<std::string, std::vector<std::string>>> loadedData_main;
    std::map<std::string, bool> show_loadedData;
    struct prepdToPlot_struct {
        std::vector<std::string>    trip_id;
        std::vector<int64_t>        epoch;
        std::vector<int32_t>        arrival;
        std::vector<int32_t>        departure;
    };
    std::vector<ImColor> plotCols;
    /**
     * "stop_id"
     *  - "trip_id"   : {...}
     *  - "epoch"     : {...}
     *  - "delay_arr" : {...}
     *  - "delay_dep" : {...}
     */
    std::map<std::string, prepdToPlot_struct> loadedData_prepdToPlot;
    for(std::string _line; std::getline(file_delaysFile, _line);) {
        if(_line.substr(0, _line.find(','))=="filename_epoch") continue;
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
        show_loadedData[str__stop_id] = false;
    }
    assert(loadedData_main.size() == show_loadedData.size());
    for(auto itr_stopID : loadedData_main) {
        prepdToPlot_struct temp;
        for(auto itr_vals : itr_stopID.second) {
            temp.trip_id.push_back(itr_vals.first);
            temp.epoch.push_back(std::stoll(itr_vals.second.at(0)));
            temp.arrival.push_back(std::stol(itr_vals.second.at(1)));
            temp.departure.push_back(std::stol(itr_vals.second.at(2)));

            if(absRange_x_epoch[0]==0 || temp.epoch.back() < absRange_x_epoch[0]) absRange_x_epoch[0] = temp.epoch.back();
            if(absRange_x_epoch[1]==0 || temp.epoch.back() > absRange_x_epoch[1]) absRange_x_epoch[1] = temp.epoch.back();
            
            if(absRange_y_delay[0]==0 || temp.arrival.back() < absRange_y_delay[0]) absRange_y_delay[0] = temp.arrival.back();
            if(absRange_y_delay[1]==0 || temp.arrival.back() > absRange_y_delay[1]) absRange_y_delay[1] = temp.arrival.back();
            if(absRange_y_delay[0]==0 || temp.departure.back() < absRange_y_delay[0]) absRange_y_delay[0] = temp.departure.back();
            if(absRange_y_delay[1]==0 || temp.departure.back() > absRange_y_delay[1]) absRange_y_delay[1] = temp.departure.back();
            // if(temp.departure.back()>1000 || temp.arrival.back()>1000) {
            //     std::cout << "FOUND A VALUE BGIGER THAN LIM: \n";
            //     std::cout << "stop_id:" << itr_stopID.first << "\n";
            //     std::cout << "   arr: " << temp.arrival.back() << "\n";
            //     std::cout << "   dep: " << temp.departure.back() << "\n";
            // }
        }
        loadedData_prepdToPlot[itr_stopID.first] = temp;
    }
    std::cout << "Loaded " << loadedData_main.size() << " num stop_id's." << std::endl;
    size_t numActive = 0;

    std::vector<std::string> xLabelVector_labels;   //epoch time: hour mark
    std::vector<std::string> yLabelVector_labels;   //delay time: seconds
    std::vector<double> xLabelVector_values;        //epoch time: epoch value + 3600
    std::vector<double> yLabelVector_values;        //delay time: seconds

    Pos2d<double> range(absRange_x_epoch.getDelta(), absRange_y_delay.getDelta());
    absRange_x_epoch[1]+=range.x*0.01;
    absRange_x_epoch[0]-=range.x*0.01;
    absRange_y_delay[1]+=range.y*0.01;
    absRange_y_delay[0]-=range.y*0.01;

    for(int64_t initTime=absRange_x_epoch[0]+(3600-(absRange_x_epoch[0]%3600)); initTime<absRange_x_epoch[1]; initTime+=3600) {
        std::chrono::system_clock::time_point timeVar = std::chrono::system_clock::time_point(std::chrono::seconds(initTime));
        auto timeVar_days   = std::chrono::time_point_cast<std::chrono::duration<int, std::ratio<86400>>>(timeVar);
        auto timeVar_tod    = timeVar-timeVar_days;
        
        auto var_hour   = static_cast<int>(std::roundf(std::chrono::duration_cast<std::chrono::hours>(timeVar_tod).count()));
        std::stringstream ss;
        ss << (var_hour<=9? "0" : "") << var_hour;
        xLabelVector_values.push_back(initTime);
        xLabelVector_labels.push_back(ss.str());
    }
    for(int32_t initDelay=absRange_y_delay[0]+(100-(absRange_y_delay[0]%100)); initDelay<absRange_y_delay[1]; initDelay+=100) {
        std::stringstream ss;
        ss << std::setw(3) << initDelay;
        yLabelVector_values.push_back(initDelay);
        yLabelVector_labels.push_back(ss.str());
    }

    GUINC::lazyimgui guiwin(false, dim_win.x, dim_win.y);
    if(!guiwin.init()) {
        std::cerr << "could not init lazyimgui instance.\n";
        return 1;
    }
    while(guiwin.running()) {
        guiwin.newFrame();
        Pos2d<double> min_range{-100, -100};
        Pos2d<double> max_range{ 100,  100};
        min_range.x = double(absRange_x_epoch[0]);
        max_range.x = double(absRange_x_epoch[1]);
        // min_range.y = double(absRange_y_delay[0]);
        // max_range.y = double(absRange_y_delay[1]);
        ImGui::SetCursorPos(ImVec2(pos_subwin_stopIDs.x, pos_subwin_stopIDs.y));
        if(ImGui::BeginChild("child_stop_id", ImVec2(dim_subwin_stopIDs.x, dim_subwin_stopIDs.y), ImGuiChildFlags_Border)) {
            numActive = 0;
            for(auto& itr_toShow : show_loadedData) {
                bool checkBoxClicked = ImGui::Checkbox(itr_toShow.first.c_str(), &itr_toShow.second);
                if(itr_toShow.second) {
                    for(auto itr_vars : loadedData_main[itr_toShow.first]) {
                        if(int(min_range.x)==-100 || std::stoll(itr_vars.second.at(0)) < min_range.x) min_range.x = std::stoll(itr_vars.second.at(0))*0.98;
                        if(int(min_range.y)==-100 || std::stoll(itr_vars.second.at(1)) < min_range.y) min_range.y = std::stoll(itr_vars.second.at(1))*1.05;
                        if(int(min_range.y)==-100 || std::stoll(itr_vars.second.at(2)) < min_range.y) min_range.y = std::stoll(itr_vars.second.at(2))*1.05;

                        if(int(max_range.x)== 100 || std::stoll(itr_vars.second.at(0)) > max_range.x) max_range.x = std::stoll(itr_vars.second.at(0))*1.05;
                        if(int(max_range.y)== 100 || std::stoll(itr_vars.second.at(1)) > max_range.y) max_range.y = std::stoll(itr_vars.second.at(1))*1.05;
                        if(int(max_range.y)== 100 || std::stoll(itr_vars.second.at(2)) > max_range.y) max_range.y = std::stoll(itr_vars.second.at(2))*1.05;
                    }
                    numActive++;
                }
            }

            ImGui::EndChild();
        }
        // Pos2d<double> range(max_range.x-min_range.x, max_range.y-min_range.y);
        // min_range -= Pos2d<double>(range.x*0.01, range.y*0.01);
        // max_range += Pos2d<double>(range.x*0.01, range.y*0.01);

        guiPlot.set_xRange(min_range.x, max_range.x);
        guiPlot.set_yRange(min_range.y, max_range.y);

        plotCols.clear();
        float hue_gap = 360.0/numActive;
        for(size_t i_col=0; i_col<numActive; i_col++) {
            auto rgbCol = convert_HSV_RGB({i_col*hue_gap, 100, 100});
            plotCols.push_back(IM_COL32(rgbCol.at(0), rgbCol.at(1), rgbCol.at(2), 100));
        }

        // ImGui::SetCursorPos(ImVec2(pos_subwin_graph.x, pos_subwin_graph.y));
        // if(ImGui::BeginChild("child_graph", ImVec2(dim_subwin_graph.x, dim_subwin_graph.y), ImGuiChildFlags_Border)) {
        ImGui::SetNextWindowPos(ImVec2(pos_subwin_graph.x, pos_subwin_graph.y));
        ImGui::SetNextWindowSize(ImVec2(dim_subwin_graph.x, dim_subwin_graph.y));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(240,240,240,200));
        ImGui::Begin("graphWin", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        ImGui::PopStyleColor();
        if(guiPlot.beginDraw()) {
            size_t active_cnt = 0; 
            for(auto itr_toPlot : loadedData_prepdToPlot) {
                if(!show_loadedData[itr_toPlot.first]) continue;
                prepdToPlot_struct& prepPlot = loadedData_prepdToPlot[itr_toPlot.first];
                try {
                    auto plotCol = plotCols.at(active_cnt);
                    guiPlot.draw_plot(std::vector<double>(prepPlot.epoch.begin(), prepPlot.epoch.end()), std::vector<double>(prepPlot.arrival.begin(), prepPlot.arrival.end()), 2, plotCol);
                    auto tempCol_hsv = convert_RGB_HSV({  int(plotCol.Value.x), int(plotCol.Value.y), int(plotCol.Value.z)});
                    auto tempCol_rgb = convert_HSV_RGB({float(tempCol_hsv[0]), float(tempCol_hsv[1]), float(tempCol_hsv[2])});
                    ImColor dep_col(IM_COL32(tempCol_rgb[0], tempCol_rgb[1], tempCol_rgb[0], 100));
                    guiPlot.draw_plot(std::vector<double>(prepPlot.epoch.begin(), prepPlot.epoch.end()), std::vector<double>(prepPlot.departure.begin(), prepPlot.departure.end()), 2, dep_col);
                }
                catch(const std::exception& e) {
                    std::cerr << e.what() << '\n';
                    exit(1);
                }
                
                active_cnt++;
            }
            guiPlot.endDraw();
            guiPlot.draw_xLabels(xLabelVector_labels, xLabelVector_values);
            guiPlot.draw_yLabels(yLabelVector_labels, yLabelVector_values);
        }
        ImGui::End();
        //     ImGui::EndChild();
        // }
        

        guiwin.endFrame();
    }
    return 0;
}




inline std::vector<int> convert_RGB_HSV(
    std::vector<int> _RGB
) {
    std::vector<float> RGB_p{
        static_cast<float>(_RGB[0])/255,
        static_cast<float>(_RGB[1])/255,
        static_cast<float>(_RGB[2])/255
    };
    std::vector<int> HSV(3, 0);
    size_t maxIdx = findIdx<float>(RGB_p, 0);
    size_t minIdx = findIdx<float>(RGB_p, 1);

    int delta = _RGB[maxIdx]-_RGB[minIdx];

    HSV[2] = static_cast<int>(100*RGB_p[maxIdx]);
    HSV[1] = static_cast<int>(100*(HSV[2]==0? 0 : delta/RGB_p[maxIdx]));
    switch (maxIdx) {
        case 0:
            HSV[0] = static_cast<int>(60*(delta==0? 0 : ((RGB_p[1]-RGB_p[2])/(delta)+0)));
            break;
        case 1:
            HSV[0] = static_cast<int>(60*(delta==0? 0 : ((RGB_p[2]-RGB_p[0])/(delta)+2)));
            break;
        case 2:
            HSV[0] = static_cast<int>(60*(delta==0? 0 : ((RGB_p[0]-RGB_p[1])/(delta)+4)));
            break;
    }
    if(HSV[0]<0) HSV[0]+=360;

    return HSV;
}


inline std::vector<float> convert_HSV_RGB(
    std::vector<float> HSV
) {
    std::vector<float> _RGB(3, 0);
    std::vector<float> RGB_p(3, 0);
    std::vector<float> HSV_p{
        static_cast<float>(HSV[0]),
        static_cast<float>(HSV[1])/100,
        static_cast<float>(HSV[2])/100
    };

    float C = HSV_p[2] * HSV_p[1];
    float X = C * float(1 - abs(fmod(HSV_p[0]/60, 2) -1));
    float m = HSV_p[2] - C;

    // std::cout<<"{"<<C<<", "<<X<<", "<<m<<"}\n";

    if(HSV_p[0] < 60) {
        RGB_p[0] = C;
        RGB_p[1] = X;
        RGB_p[2] = 0;
    }
    else if(HSV_p[0] < 120) {
        RGB_p[0] = X;
        RGB_p[1] = C;
        RGB_p[2] = 0;
    }
    else if(HSV_p[0] < 180) {
        RGB_p[0] = 0;
        RGB_p[1] = C;
        RGB_p[2] = X;
    }
    else if(HSV_p[0] < 240) {
        RGB_p[0] = 0;
        RGB_p[1] = X;
        RGB_p[2] = C;
    }
    else if(HSV_p[0] < 300) {
        RGB_p[0] = X;
        RGB_p[1] = 0;
        RGB_p[2] = C;
    }
    else {
        RGB_p[0] = C;
        RGB_p[1] = 0;
        RGB_p[2] = X;
    }

    _RGB[0] = (RGB_p[0]+m)*static_cast<float>(255);
    _RGB[1] = (RGB_p[1]+m)*static_cast<float>(255);
    _RGB[2] = (RGB_p[2]+m)*static_cast<float>(255);

    return _RGB;
}


template<typename varType>
inline size_t findIdx(std::vector<varType> toCheck, int toFind) {
    size_t index = 0;
    for(size_t i=1; i<toCheck.size(); i++) {
        switch (toFind) {
        case 0: //max
            if(toCheck[i]>toCheck[index]) {index = i;}
            break;
        case 1: //min
            if(toCheck[i]<toCheck[index]) {index = i;}
            break;
        default:
            throw std::invalid_argument("findIdx: invalid `toFind` argument.");
            break;
        }
    }
    return index;
}
