#pragma once
#ifndef HPP_PROCESS
#define HPP_PROCESS

#include <includes.hpp>


inline std::vector<std::string> lambdaFunc_parse_csv__stop_times(std::string _line, bool* _tripFound, std::vector<std::string>& tripFilter) {
    std::vector<std::string> csv_line;
    std::string _temp_tripID = "";
    for(std::string _trip_id : tripFilter) {
        // Useful::ANSI_mvprint(0, terminalCursorPos.y+1, std::string("Matching: ")+_temp_tripID+" | "+_trip_id+" ?: ", true);
        if((_temp_tripID=_line.substr(0, _line.find(',')))==_trip_id) {
            // Useful::ANSI_mvprint(50, terminalCursorPos.y+1, "MATCHED ", true); std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if(_tripFound) *_tripFound = true;
            size_t comma_idx = std::string::npos;
            do {
                csv_line.push_back(_line.substr(comma_idx+1, _line.find(',', comma_idx+1)-comma_idx-1));
                comma_idx = _line.find(',', comma_idx+1);
            } while (comma_idx!=std::string::npos);

            break;
        }
        else {
            // Useful::ANSI_mvprint(50, terminalCursorPos.y+1, "NO MATCH", true);
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
    bool& success
) {
    std::unique_lock<std::mutex> u_lck_cout(mtx_cout, std::defer_lock);
    

    // while(threadTask_loadFile__startIdxCnt.load()!=thread_ID && program_running.load()); 
    std::ifstream fileToRead(filename);
    if(!fileToRead.is_open() || totalNumFilelines==0 || totalNumThreads==0 || line_lim.size()!=2) {
        success = false;
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
            returnVec.at(lineCount+line_lim.at(0)) = lambdaFunc_parse_csv__stop_times(_line, &lineFound, trip_filter);

            if(lineFound) foundIdx_list.push_back(line_lim.at(0)+lineCount);
        }
        catch(const std::exception& e) {
            success = false;
            u_lck_cout.lock();
            Useful::ANSI_mvprint(31,terminalCursorPos.y+thread_ID+2, std::string("<<< [ THREAD TERMINATED; EXCEPTION MET:")+e.what()+" ] >>>",true);
            u_lck_cout.unlock();
            fileToRead.close();
            return;
        }

        #if use_multiThreadDataDisplay
            if(cnt>=cnt_lim || lineCount>=line_lim.at(1)) {
                u_lck_cout.lock();
                progr = lineCount;
                progr_percent = double(lineCount)/line_lim.at(1);
                Useful::ANSI_mvprint(
                    31,terminalCursorPos.y+thread_ID+2,
                    Useful::formatNumber(progr_percent*100, 5, 1)+"% | foundIdx_list.size(): "+
                    Useful::formatNumber(foundIdx_list.size(),8,1,"right",false,true)+" | lineCount: "+
                    Useful::formatNumber(lineCount,8,1,"right",false,true)+" | [foundIdx_list]:ptr: "+
                    Useful::formatNumber(&foundIdx_list) + " | reading line:"+
                    Useful::formatNumber(line_lim.at(0),6,1,"right",false,true) + " + "+Useful::formatNumber(lineCount,6,1,"right",false,true)+" = "+
                    Useful::formatNumber(lineCount+line_lim.at(0),6,1,"right",false,true),
                    false
                );
                u_lck_cout.unlock();

                cnt=0;
            }
            else {
                cnt++;
            }
        #endif
    }
    progr_percent = 1;
    u_lck_cout.lock();
    progr = line_lim.at(1);
    Useful::ANSI_mvprint(
        31,terminalCursorPos.y+thread_ID+2,
        Useful::formatNumber(progr_percent*100, 5, 1)+"% | foundIdx_list.size(): "+
        Useful::formatNumber(foundIdx_list.size(),8,1,"right",false,true)+" | lineCount: "+
        Useful::formatNumber(line_lim.at(1)-1,8,1,"right",false,true),
        false
    );
    
    u_lck_cout.unlock();

    success = true;
    fileToRead.close();
}


inline bool subTask__StopTimesFile_stopID_search(std::unordered_map<uint32_t, std::map<std::string, std::string>> refrTree, std::string refr_tripID, STU& fill_STU) {

    // for(auto i_stopseq : refrTree.vec) {
    //     if(fill_STU.stop_sequence==i_stopseq.stop_sequence) {
    //         for(auto i_tripid : i_stopseq.stop_seq_vec) {
    //             if(refr_tripID==i_tripid.trip_id) {
    //                 fill_STU.stop_id = i_tripid.stop_id;
    //                 return true;
    //             }
    //         }
    //     }
    // }

    return false;
}

inline void threadTask_parseDelaysFromEntry(
    size_t threadID,
    std::list<std::string> _entryFilenames,
    std::vector<size_t> idx_lim,
    std::unordered_map<uint32_t, std::map<std::string, std::string>> refrTree,
    std::vector<parseException_DebugString>& retur_caughtExcepts,
    std::vector<TrpUpd>& retur_storedData_tripupds,
    std::vector<STU_refd>& retur_storedData_tripdelays,
    size_t& retur_numTotalTripsRead
) {
    size_t strLen_totMaxEntries = Useful::formatNumber(idx_lim.at(1),0,0,"right",false,true).length();

    std::unique_lock<std::mutex> u_lck_cout(mtx_cout, std::defer_lock);
    u_lck_cout.lock();
    Useful::ANSI_mvprint(4,terminalCursorPos.y+threadID+1,Useful::formatNumber(threadID, 2)+" : ["+Useful::formatNumber(idx_lim.at(0), 9,1,"right",false,true)+":"+Useful::formatNumber(idx_lim.at(0)+idx_lim.at(1), 9,1,"right",false,true)+"] : ");
    u_lck_cout.unlock();

    std::vector<TrpUpd> storedData_tripUpds;
    std::vector<STU_refd> storedData_tripDelays;
    std::vector<parseException_DebugString> vecExceptions_DebugString;
    
    auto entryPathStr_itr = _entryFilenames.begin();

    try {
        
        std::advance(entryPathStr_itr, idx_lim.at(0));
    }
    catch(const std::exception& e) {
        u_lck_cout.lock();
        Useful::ANSI_mvprint(4,terminalCursorPos.y+threadID+2,e.what());
        u_lck_cout.unlock();
        return;
    }
    
    try {
        
        for(size_t i=0; i<idx_lim.at(1); i++) {
            std::fstream entryFile(*entryPathStr_itr, std::ios::in |std::ios::binary);

            std::string entry_filename = entryPathStr_itr->substr(Useful::findSubstr("sl-tripupdates-", *entryPathStr_itr));
            int64_t filename_epoch = 0;

            if(!entryFile) {
                vecExceptions_DebugString.push_back({"entryFile.isOpen()==false", std::string("\"file:'")+entry_filename+"'\",\"i_upd:-1\",\"exceptCatch_processID:"+std::to_string(-1)+"\""});
                std::advance(entryPathStr_itr, 1);
                continue;
            }
            
            transit_realtime::TripUpdate trpUpdate;
            trpUpdate.ParseFromIstream(&entryFile);

            //---------- process start ----------
            
            try {
                filename_epoch = parse_epochTime_fromFilename(entry_filename);
            }
            catch(const std::exception& e) {

            }
            
            size_t i_upd = 0;
            size_t exceptCatch_processID = 0;
            try {
                for(; i_upd<trpUpdate.stop_time_update_size(); i_upd++) {

                    exceptCatch_processID = 0;
                    storedData_tripUpds.push_back(ParseDebugString(trpUpdate.stop_time_update(i_upd).DebugString()));

                    exceptCatch_processID = 1;
                    auto tempTrp = storedData_tripUpds.back();
                    bool tripToSearch = false;
                    retur_numTotalTripsRead+=1;
                    for(std::string itr_trip : trip_id__found) {
                        if(tempTrp.trip.trip_id==itr_trip) {
                            tripToSearch = true;
                            break;
                        }
                    }
                    
                    if(!tripToSearch) continue;
                    
                    exceptCatch_processID = 2;
                    for(size_t i_stu=0; i_stu<tempTrp.stop_time_updates.size(); i_stu++) {
                        auto tempSTU = tempTrp.stop_time_updates.at(i_stu);
                        if(!(tempSTU.arrival.delay==0 && tempSTU.departure.delay==0)) {
                            #if useStopTimesFile
                            if(tempSTU.stop_id=="" && (tempTrp.trip.trip_id.size()>0 && tempSTU.stop_sequence!=0)) { //use stop_sequence and trip_id to find missing stop_id for this S.T.U
                                exceptCatch_processID = 3;
                                
                                // bool _refrMatchFound = subTask__StopTimesFile_stopID_search(refrTree, tempTrp.trip.trip_id, tempSTU);
                                bool matchFound = false;
                                auto itrTree = refrTree.find(tempSTU.stop_sequence);
                                if(itrTree!=refrTree.end()) {
                                    auto itrTree2 = itrTree->second.find(tempTrp.trip.trip_id);
                                    if(itrTree2!=itrTree->second.end()) {
                                        tempSTU.stop_id = itrTree2->second;
                                        matchFound = true;
                                    }
                                }
                            }
                            #endif
                            exceptCatch_processID = 4;
                            storedData_tripDelays.push_back(STU_refd{tempSTU, tempTrp.trip.trip_id, static_cast<uint32_t>(i_stu), filename_epoch});
                        }
                    }
                    // Useful::ANSI_mvprint(0, 8, Useful::formatNumber(i_upd));
                }
            } catch(const std::exception& e) {
                vecExceptions_DebugString.push_back({e.what(), std::string("\"file:'")+entry_filename+"'\",\"i_upd:"+std::to_string(i_upd)+"\",\"exceptCatch_processID:"+std::to_string(exceptCatch_processID)+"\""});
            }

            //---------- process end   ----------

            std::advance(entryPathStr_itr, 1);
            entryFile.close();
            trpUpdate.Clear();

            u_lck_cout.lock();
            Useful::ANSI_mvprint(29+4,terminalCursorPos.y+threadID+1, Useful::formatNumber(i,strLen_totMaxEntries,0,"right",false,true)+"/"+Useful::formatNumber(idx_lim.at(1),strLen_totMaxEntries,0,"right",false,true));
            u_lck_cout.unlock();
        }
    }
    catch(const std::exception& e) {
        u_lck_cout.lock();
        Useful::ANSI_mvprint(0, terminalCursorPos.y+3, e.what());
        u_lck_cout.unlock();
    }
    

    retur_caughtExcepts.swap(vecExceptions_DebugString);
    retur_storedData_tripupds.swap(storedData_tripUpds);
    retur_storedData_tripdelays.swap(storedData_tripDelays);
}



std::unordered_map<uint32_t, std::map<std::string, std::string>> subProcess_loadFile__stop_times(
    std::string filename,
    std::vector<std::vector<std::string>>& returVecRef,
    std::vector<std::string> tripFilter
);

std::vector<parseException_DebugString> subProcess_processEntries(
    std::list<std::string> entriesToProcess,
    std::unordered_map<uint32_t, std::map<std::string, std::string>> refrTree,
    size_t& numTotalTripsRead,
    size_t& entryPathOpenFailures,
    std::vector<STU_refd>& storedData_tripDelays_idx
);




#endif //HPP_PROCESS