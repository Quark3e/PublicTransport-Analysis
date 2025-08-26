#pragma once
#ifndef HPP_PROCESS
#define HPP_PROCESS

#include <includes.hpp>


inline std::atomic<size_t> threadTask_loadFile__startIdxCnt{0};
template<typename _vecType>
inline void threadTask_loadFile(
    std::vector<_vecType>& returnVec,
    std::string filename,
    std::function<_vecType(std::string file_line)> modifierFunc,
    size_t thread_ID,
    size_t totalNumThreads,
    bool* success = nullptr
) {

    while(threadTask_loadFile__startIdxCnt.load()!=thread_ID && program_running.load()); 

    std::ifstream fileToRead(filename);
    if(!fileToRead.is_open()) {
        if(success) *success = false;
        fileToRead.close();
        return;
    }

    

    threadTask_loadFile__startIdxCnt+=1;


    std::string _line = "";
    for(size_t _=0; _<thread_ID; _++) fileToRead.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    for(size_t lineCount=0; std::getline(fileToRead, _line, '\n'); lineCount++) {
        
        
        if(_line.size()==0) continue;
        try {
            returnVec.at(lineCount) = modifierFunc(_line);
        }
        catch(const std::exception& e) {
            if(success) *success = false;
            fileToRead.close();
            return;
        }
        
        // for(size_t _ign=0; _ign<totalNumThreads-1; _ign++) {
        //     fileToRead.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        //     if(fileToRead.rdstate()==std::ios::eofbit) break;
        //     lineCount++;
        // }


    }

    fileToRead.close();
    if(success) *success = true;
}

StopID_refrSorted subProcess_loadFile__stop_times(
    std::string filename,
    std::vector<std::vector<std::string>>& returVecRef,
    std::vector<std::string> tripFilter
);




std::vector<parseException_DebugString> subProcess_processEntries(std::list<std::string> entriesToProcess, StopID_refrSorted& refrTree);


#endif //HPP_PROCESS