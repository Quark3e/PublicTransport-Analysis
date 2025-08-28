#pragma once
#ifndef HPP_PROCESS
#define HPP_PROCESS

#include <includes.hpp>


inline std::vector<std::string> lambdaFunc_parse_csv__stop_times(std::string _line, bool* _tripFound, std::vector<std::string>& tripFilter) {
    std::vector<std::string> csv_line;
    for(std::string _trip_id : tripFilter) {
        if(_line.substr(0, _line.find(','))==_trip_id) {
            if(_tripFound) *_tripFound = true;
            size_t comma_idx = std::string::npos;
            do {
                csv_line.push_back(_line.substr(comma_idx+1, _line.find(',', comma_idx+1)-comma_idx-1));
                comma_idx = _line.find(',', comma_idx+1);
            } while (comma_idx!=std::string::npos);
            break;
        }
    }
    return csv_line;
};

inline std::atomic<size_t> threadTask_loadFile__startIdxCnt{0};
template<typename _vecType>
inline void threadTask_loadFile(
    std::vector<_vecType>& returnVec,
    std::list<size_t>& foundIdx_list,
    std::vector<std::string> trip_filter,
    std::string filename,
    size_t thread_ID,
    size_t totalNumThreads,
    size_t totalNumFilelines,
    std::vector<size_t>& line_lim,
    // atomwrapper<size_t>& progr,
    size_t& progr,
    bool* success = nullptr
) {
    std::unique_lock<std::mutex> u_lck_cout(mtx_cout, std::defer_lock);
    

    // while(threadTask_loadFile__startIdxCnt.load()!=thread_ID && program_running.load()); 
    std::ifstream fileToRead(filename);
    if(!fileToRead.is_open() || totalNumFilelines==0 || totalNumThreads==0 || line_lim.size()!=2) {
        if(success) *success = false;
        return;
    }

    // size_t idx_gap = float(totalNumFilelines)/float(totalNumThreads);
    // size_t idx_start = thread_ID*idx_gap;
    for(size_t _ign=0; _ign<line_lim.at(0); _ign++) fileToRead.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');

    u_lck_cout.lock();
    Useful::ANSI_mvprint(4,terminalCursorPos.y+thread_ID+2,Useful::formatNumber(thread_ID, 2)+" : ["+Useful::formatNumber(line_lim.at(0), 9,1,"right",false,true)+":"+Useful::formatNumber(line_lim.at(0)+line_lim.at(1), 9,1,"right",false,true)+"] : ");
    /// terminalCursorPos = {4+27, +thread_ID+2}
    u_lck_cout.unlock();

    double progr_percent = 0;
    // double progr_rate = 0;
    
    u_lck_cout.lock();
    Useful::ANSI_mvprint(31,terminalCursorPos.y+thread_ID+2,"init. read");
    u_lck_cout.unlock();

    std::string _line = "";

    size_t cnt = 0, cnt_lim = 100;
    for(size_t lineCount=0; lineCount<line_lim.at(1); lineCount++) {
        std::getline(fileToRead, _line, '\n');
        
        if(_line.size()==0) continue;
        bool lineFound = false;
        try {
            returnVec.at(lineCount) = lambdaFunc_parse_csv__stop_times(_line, &lineFound, trip_filter);

            if(lineFound) foundIdx_list.push_back(line_lim.at(0)+lineCount);
        }
        catch(const std::exception& e) {
            if(success) *success = false;
            fileToRead.close();
            return;
        }

        if(cnt>=cnt_lim) {
            u_lck_cout.lock();
            progr = lineCount;
            progr_percent = double(lineCount)/line_lim.at(1);
            Useful::ANSI_mvprint(
                31,terminalCursorPos.y+thread_ID+2,
                Useful::formatNumber(progr_percent*100, 5, 1)+"% | foundIdx_list.size(): "+Useful::formatNumber(foundIdx_list.size(),8,1,"right",false,true)+" | lineCount: "+Useful::formatNumber(lineCount,8,1,"right",false,true),
                false
            );
            u_lck_cout.unlock();

            cnt=0;
        }
        else {
            cnt++;
        }
    }
    progr_percent = 1;
    u_lck_cout.lock();
    progr = line_lim.at(1);
    Useful::ANSI_mvprint(31,terminalCursorPos.y+thread_ID+2,Useful::formatNumber(progr_percent*100, 5, 1)+"%     ",false);
    u_lck_cout.unlock();


    u_lck_cout.lock();
    Useful::ANSI_mvprint(31,terminalCursorPos.y+thread_ID+2,"fin. exit.");
    u_lck_cout.unlock();

    fileToRead.close();
    if(success) *success = true;
}

StopID_refrSorted subProcess_loadFile__stop_times(
    std::string filename,
    std::vector<std::vector<std::string>>& returVecRef,
    std::vector<std::string> tripFilter
);




std::vector<parseException_DebugString> subProcess_processEntries(
    std::list<std::string> entriesToProcess,
    StopID_refrSorted& refrTree,
    size_t& numTotalTripsRead,
    size_t& entryPathOpenFailures,
    std::vector<STU_refd>& storedData_tripDelays_idx
);


#endif //HPP_PROCESS