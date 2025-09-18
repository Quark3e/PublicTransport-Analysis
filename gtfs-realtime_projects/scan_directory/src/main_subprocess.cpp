
#include <main_subprocess.hpp>



std::unordered_map<uint32_t, std::map<std::string, std::string>> subProcess_loadFile__stop_times(
    std::string filename,
    std::vector<std::vector<std::string>>& returVecRef,
    std::vector<std::string> tripFilter
) {
    size_t maxThreadCount = (setThreadLim>=0? setThreadLim : std::thread::hardware_concurrency());
    if(maxThreadCount==0) {
        std::cerr << "std::thread::hardware_concurrency() return 0\n";
        program_running = false;
        exit(1);
    }


    std::ifstream in_stream;
    std::ifstream file__cleanedStopTimes("dataFile_cleanedStopTimes.csv");
    if(use_cleanedStopTimes && file__cleanedStopTimes.is_open()) {
        Useful::PrintOut("Using dataFile_cleanedStopTimes.csv file.",std::string::npos, "left","\n",true,false,false,1,1,&terminalCursorPos);
        in_stream.swap(file__cleanedStopTimes);
        filename = "dataFile_cleanedStopTimes.csv";
    }
    else {
        Useful::PrintOut("Is NOT using dataFile_cleanedStopTimes.csv file.",std::string::npos, "left","\n",true,false,false,1,1,&terminalCursorPos);
        in_stream.open(filename);
    }

    size_t columnCount = 10;
    Useful::PrintOut("Checking number of lines in stop_times refr. file..", std::string::npos, "left","",true,false,false,1,1,&terminalCursorPos);
    size_t stop_times_max_lineCount = std::count_if(std::istreambuf_iterator<char>{in_stream}, {}, [](char c) { return c == '\n'; })+1;
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, std::string("[maxThreadCount:")+std::to_string(maxThreadCount)+", stop_times file line count:"+Useful::formatNumber(stop_times_max_lineCount,0,1,"right",false,true)+"]");
    
    terminalCursorPos.y +=2;
    terminalCursorPos.x = 0;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if(stop_times_max_lineCount<1000) {
        std::cerr << "stop_times file returned <1000 number of lines somehow\n.";
        program_running = false;
        exit(1);
    }
    
    returVecRef = std::vector<std::vector<std::string>>(stop_times_max_lineCount, std::vector<std::string>(columnCount, ""));
    std::list<std::list<size_t>> refrFoundIdx(maxThreadCount, std::list<size_t>());
    std::vector<std::thread> threadObjects;
    std::list<bool> result_threadTask(maxThreadCount, false);
    auto itr__result_threadTask = result_threadTask.begin();
    auto itr__foundIdx_threadTask = refrFoundIdx.begin();
    // std::advance(itr__result_threadTask, 1);


    
    threadTask_loadFile__startIdxCnt = 0;

    std::unique_lock<std::mutex> u_lck_cout(mtx_cout, std::defer_lock);

    if(maxThreadCount>1) Useful::PrintOut("Starting threads to load stop_times file.",std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);
    std::string _tempStr = "|";
    // for(size_t i=0; i<maxThreadCount; i++) _tempStr += "     |";
        
    std::vector<std::vector<size_t>> idx_lim;
    // std::vector<atomwrapper<size_t>> idx_progr;
    std::list<size_t> idx_progr(maxThreadCount, 0);
    auto itr__idx_progr_threadTask = idx_progr.begin();

    for(size_t _thr=0; _thr<maxThreadCount; _thr++) {
        size_t idx_gap = std::ceil(float(stop_times_max_lineCount)/float(maxThreadCount));
        size_t idx_start = _thr*idx_gap;
        idx_lim.push_back({idx_start, idx_gap}); //{start, gap, progress}
        // idx_progr.push_back(0);
    }

    std::chrono::system_clock::time_point times_threadTasks_start = std::chrono::system_clock::now();
    auto times_threadTasks_start_str = std::chrono::system_clock::to_time_t(times_threadTasks_start);
    Useful::PrintOut(std::string("Process: threadTask: started : ")+std::ctime(&times_threadTasks_start_str),std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);
    Useful::PrintOut(std::string("TripFilter size: ")+std::to_string(tripFilter.size()),std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);



    /// Initialise the different threads.
    for(size_t id_thread=1; id_thread<maxThreadCount; id_thread++) {
        // if(id_thread+1<maxThreadCount) {
            std::advance(itr__result_threadTask, 1);
            std::advance(itr__foundIdx_threadTask, 1);
            std::advance(itr__idx_progr_threadTask, 1);
        // }
        threadObjects.emplace_back([&, id_thread, filename, itr__foundIdx_threadTask] {
            threadTask_loadFile<std::vector<std::string>>(
                std::ref(returVecRef),
                std::ref(*itr__foundIdx_threadTask),
                tripFilter,
                filename,
                id_thread,
                maxThreadCount,
                stop_times_max_lineCount,
                std::ref(idx_lim.at(id_thread)),
                std::ref(*itr__idx_progr_threadTask),
                std::ref(*itr__result_threadTask)
            );
        });
    }

    
    auto time_start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point time_t1=time_start, time_t2;
    
    ///--- Main thread task start --- ///!! NOTE: NEED TO IMPLEMENT CHANGES BEFORE USING MULTITHREADING

    std::ifstream fileToRead(filename);
    itr__result_threadTask = result_threadTask.begin();
    if(!fileToRead.is_open()) {
        *itr__result_threadTask = false;
    }
    std::string _line = "";

    
    size_t ms_interval = 1000;
    std::chrono::duration<double> interval_dur;
    size_t delta_lineCount_t1 = 0;


    for(size_t _ign=0; _ign<idx_lim.at(0).at(0); _ign++) fileToRead.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    
    u_lck_cout.lock();
    Useful::ANSI_mvprint(4,terminalCursorPos.y+2,Useful::formatNumber(0, 2)+" : ["+Useful::formatNumber(idx_lim.at(0).at(0), 9,1,"right",false,true)+":"+Useful::formatNumber(idx_lim.at(0).at(0)+idx_lim.at(0).at(1), 9,1,"right",false,true)+"] : ");
    u_lck_cout.unlock();

    
    double progr_percent = 0;

    std::list<size_t>& ref_refrFoundIdx = *refrFoundIdx.begin();

    for(size_t lineCount=0; lineCount<idx_lim.at(0).at(1); lineCount++) {
        std::getline(fileToRead, _line, '\n');

        if(_line.size()==0) continue;
        bool _tripIdFromStopTimesFound = false;
        try {
            returVecRef.at(lineCount) = lambdaFunc_parse_csv__stop_times(_line, &_tripIdFromStopTimesFound, tripFilter);
            //thread issues????
            
            if(_tripIdFromStopTimesFound) {
                ref_refrFoundIdx.push_back(idx_lim.at(0).at(0)+lineCount);
            }
        }
        catch(const std::exception& e) {
            *itr__result_threadTask = false;
            u_lck_cout.lock();
            Useful::ANSI_mvprint(31,terminalCursorPos.y+2, std::string("<<< [ THREAD TERMINATED; EXCEPTION MET:")+e.what()+" ] >>>",true);
            u_lck_cout.unlock();
            fileToRead.close();
            break;
        }


        time_t2 = std::chrono::steady_clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(interval_dur=(time_t2-time_t1)).count()>=ms_interval || lineCount>=idx_lim.at(0).at(1)) {
            size_t sum_lineCount = 0;
            std::string printStr_sum_lineCount("thread progresses: {");
            progr_percent = double(lineCount)/idx_lim.at(0).at(1);
            *idx_progr.begin() = lineCount;

            #if use_multiThreadDataDisplay
                u_lck_cout.lock();
                for(auto idx_itr=idx_progr.begin(); idx_itr!=idx_progr.end(); ++idx_itr) {
                    sum_lineCount += *idx_itr;
                    printStr_sum_lineCount += Useful::formatNumber(*idx_itr,8,1,"right",false,true) + ",";
                }
                printStr_sum_lineCount+="}";
                u_lck_cout.unlock();
            #else
                sum_lineCount = lineCount;
                printStr_sum_lineCount += Useful::formatNumber(lineCount,8,1,"right",false,true) + "}";
                Useful::ANSI_mvprint(0, terminalCursorPos.y+maxThreadCount+3, printStr_sum_lineCount, false);
            #endif
            
            
            size_t delta_lineCount = sum_lineCount-delta_lineCount_t1;
            double speed = static_cast<double>(delta_lineCount)/std::chrono::duration_cast<std::chrono::seconds>(interval_dur).count();
            std::chrono::duration<double> ETA_seconds(
                (use_multiThreadDataDisplay? (double(stop_times_max_lineCount-sum_lineCount)/speed) : (double(std::ceil(double(stop_times_max_lineCount)/maxThreadCount)-lineCount)/speed))
            );
            std::string _printStr = "curr. single lineCount:"+ Useful::formatNumber(sum_lineCount,9,1,"right",false,true)+" / "+Useful::formatNumber(double(std::ceil(double(stop_times_max_lineCount)/maxThreadCount)-lineCount),9,3,"right",false,true);
            _printStr += " , process rate:"+Useful::formatNumber(speed,5,0)+" p/s , ETA:";
            _printStr += Useful::formatDuration(ETA_seconds);
            
            u_lck_cout.lock();
            Useful::ANSI_mvprint(50, terminalCursorPos.y+maxThreadCount+3, std::string("delta_lineCount_t1: ")+Useful::formatNumber(delta_lineCount_t1, 5), false);
            Useful::ANSI_mvprint(
                31,terminalCursorPos.y+2,
                Useful::formatNumber(progr_percent*100, 5, 1)+"% | foundIdx_list.size(): "+Useful::formatNumber(ref_refrFoundIdx.size(),8,1,"right",false,true)+" | lineCount: "+Useful::formatNumber(lineCount,8,1,"right",false,true),
                false
            );
            Useful::ANSI_mvprint(0, terminalCursorPos.y+1+maxThreadCount+3, _printStr, false);
            u_lck_cout.unlock();


            time_t1 = time_t2;
            delta_lineCount_t1 = sum_lineCount;
        }
    }
    
    fileToRead.close();
    *itr__result_threadTask = true;
    ///--- Main thread task end ---///
    

    u_lck_cout.lock();
    Useful::ANSI_mvprint(
        31,terminalCursorPos.y+2,
        Useful::formatNumber(progr_percent*100, 5, 1)+"% | foundIdx_list.size(): "+Useful::formatNumber(ref_refrFoundIdx.size(),8,1,"right",false,true)+" | lineCount: "+Useful::formatNumber(idx_lim.at(0).at(1),8,1,"right",false,true),
        false
    );
    Useful::ANSI_mvprint(0, terminalCursorPos.y+1+maxThreadCount+2, std::string("finished main thread's process: waiting for other threads"));
    u_lck_cout.unlock();
    for(size_t i=0; i<threadObjects.size(); i++) {
        if(threadObjects.at(i).joinable()) threadObjects.at(i).join();
    }
    terminalCursorPos.x = 0;
    terminalCursorPos.y+= maxThreadCount+7;
    Useful::PrintOut("finished main thread threadTask.",dim_terminal.x+1,"left","\n",true,false,false,1,1,&terminalCursorPos);

    std::chrono::system_clock::time_point times_threadTasks_end = std::chrono::system_clock::now();
    auto times_threadTasks_end_str = std::chrono::system_clock::to_time_t(times_threadTasks_end);
    Useful::PrintOut(std::string("Process: threadTask: finished: ")+std::ctime(&times_threadTasks_end_str),std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);
    std::chrono::duration<double> times_threadTasks_delta = times_threadTasks_end-times_threadTasks_start;
    Useful::PrintOut(std::string("total duration: ")+Useful::formatDuration(times_threadTasks_delta),std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);


    terminalCursorPos.y+= 1;
    terminalCursorPos.x = 0;
    
    /// returVecRef vector cleanup
    bool emptyVectorFound = false;
    Useful::PrintOut(std::string("Performing vector cleanup. "),std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);
    size_t cleanupDiff = returVecRef.size();

    Useful::PrintOut(std::string("refrFoundIdx: "+std::to_string(refrFoundIdx.size())),std::string::npos,"left","",true,false,false,1,1,&terminalCursorPos);
    std::vector<std::vector<std::string>> _tempCopy;
    for(auto itr_threadID=refrFoundIdx.begin(); itr_threadID!=refrFoundIdx.end(); ++itr_threadID) {
        for(auto itr_found=(*itr_threadID).begin(); itr_found!=(*itr_threadID).end(); ++itr_found) {
            _tempCopy.push_back(returVecRef.at(*itr_found));
            Useful::ANSI_mvprint(terminalCursorPos.x+10, terminalCursorPos.y, std::string("total found size: ")+Useful::formatNumber(_tempCopy.size(),6,0,"right",false,true), false);
        }
    }
    
    returVecRef = _tempCopy;
    cleanupDiff -= returVecRef.size();
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, std::string("cleanup diff: ")+std::to_string(cleanupDiff), false);
    
    terminalCursorPos.y++;
    terminalCursorPos.x = 0;

    ///--- save the cleaned up file as a temporary

    if(filename != "dataFile_cleanedStopTimes.csv") {
        terminalCursorPos.y += 1;
        Useful::PrintOut("Saving cleaned up stop_times file into a temporary file \"dataFile_cleanedStopTimes.csv\"",std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);
    
        std::ofstream file_cleanedStopTimes("dataFile_cleanedStopTimes.csv");
        if(file_cleanedStopTimes.is_open()) {
            file_cleanedStopTimes << "trip_id,arrival_time,departure_time,stop_id,stop_sequence,stop_headsign,pickup_type,drop_off_type,shape_dist_traveled,timepoint\n";
            for(size_t i=0; i<returVecRef.size(); i++) {
                for(size_t ii=0; ii<returVecRef.at(i).size(); ii++) {
                    file_cleanedStopTimes << returVecRef.at(i).at(ii) << (ii+1<returVecRef.at(i).size()? "," : "\n");
                }
            }
        }
        file_cleanedStopTimes.close();
        Useful::PrintOut("Finished saving file.",std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);
    }
    ///---

    
    
    terminalCursorPos.y+=1;
    Useful::PrintOut("Performing refrSorted std::map tree fill.",std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);
    terminalCursorPos.y+=1;
    
    // StopID_refrSorted refrRetur;
    std::unordered_map<uint32_t, std::map<std::string, std::string>> refrRetur;
    try {
        // for(size_t i=0; i<returVecRef.size(); i++) {
        //     Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y-1, std::string("returVecRef.at(")+std::to_string(i)+").size() : "+Useful::formatNumber(returVecRef.at(i).size(),2));
        //     uint32_t _stop_seq = (returVecRef.at(i).at(4).size()>0? std::stoi(returVecRef.at(i).at(4)) : 0);
        //     std::string _trip_id = returVecRef.at(i).at(0);
        //     std::string _stop_id = returVecRef.at(i).at(3);
        //     bool found__stop_seq    = false;
        //     bool found__trip_id     = false;
        //     for(size_t ii=0; ii<refrRetur.vec.size(); ii++) { //check every stop_seq
        //         if(refrRetur.vec.at(ii).stop_sequence == _stop_seq) {
        //             found__stop_seq = true;
        //             found__trip_id = false;
        //             for(size_t iii=0; iii<refrRetur.vec.at(ii).stop_seq_vec.size(); iii++) { //check every trip_id
        //                 if(refrRetur.vec.at(ii).stop_seq_vec.at(iii).trip_id==_trip_id) {
        //                     found__trip_id = true;
        //                     refrRetur.vec.at(ii).stop_seq_vec.at(iii).stop_id = _stop_id;
        //                     break;
        //                 }
        //             }
        //             if(!found__trip_id) {
        //                 refrRetur.vec.at(ii).stop_seq_vec.push_back({_trip_id, {_stop_id}});
        //             }
        //             break;
        //         }
        //     }
        //     if(!found__stop_seq) {
        //         refrRetur.vec.push_back({_stop_seq, {{_trip_id, {_stop_id}}}});
        //     }
        // }
        for(size_t i=0; i<returVecRef.size(); i++) {
            Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y-1, std::string("returVecRef.at(")+std::to_string(i)+").size() : "+Useful::formatNumber(returVecRef.at(i).size(),2));
            uint32_t _stop_seq = (returVecRef.at(i).at(4).size()>0? std::stoi(returVecRef.at(i).at(4)) : 0);
            std::string _trip_id = returVecRef.at(i).at(0);
            std::string _stop_id = returVecRef.at(i).at(3);
            bool found__stop_seq    = false;
            bool found__trip_id     = false;
            for(auto& itr_stopSeq : refrRetur) {
                if(itr_stopSeq.first==_stop_seq) {
                    found__stop_seq = true;
                    found__trip_id  = false;
                    for(auto& itr_tripId : itr_stopSeq.second) {
                        if(itr_tripId.first==_trip_id) {
                            found__trip_id = true;
                            itr_tripId.second = _stop_id;
                            break;
                        }
                    }
                    if(!found__trip_id) {
                        refrRetur[_stop_seq][_trip_id] = _stop_id;
                        break;
                    }
                }
            }
            if(!found__stop_seq) {
                refrRetur.insert(std::make_pair(_stop_seq, std::map<std::string, std::string>()));
                refrRetur[_stop_seq].insert(std::make_pair(_trip_id, _stop_id));
            }
        }
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        exit(1);
    }
    

    terminalCursorPos.y++;
    terminalCursorPos.x = 0;

    std::string _taskSuccess="";
    for(auto itr=result_threadTask.begin(); itr!=result_threadTask.end(); ++itr) {
        _taskSuccess+=Useful::formatNumber(*itr, 5, 1, "left") + " ";
    }
    Useful::PrintOut(std::string("Finished reading file. Found:")+std::to_string(returVecRef.size())+" lines. (cleaned up:"+std::to_string(cleanupDiff)+")",dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    Useful::PrintOut(std::string("threadTask:{")+_taskSuccess+"}", dim_terminal.x, "left","\n",true,false,false,1,1,&terminalCursorPos);

    Useful::PrintOut(
        std::string("   total time: ") + Useful::formatDuration((std::chrono::steady_clock::now()-time_start)),
        dim_terminal.x, "left","\n",true,false,false,1,1,&terminalCursorPos
    );
    return refrRetur;
}


std::vector<parseException_DebugString> subProcess_processEntries(
    std::list<std::string> entriesToProcess,
    std::unordered_map<uint32_t, std::map<std::string, std::string>> refrTree,
    size_t& numTotalTripsRead,
    size_t& entryPathOpenFailures, //ignored
    std::vector<STU_refd>& storedData_tripDelays_idx
) {
    size_t maxThreadCount = (setThreadLim>=0? setThreadLim : std::thread::hardware_concurrency());
    if(maxThreadCount==0) {
        std::cerr << "std::thread::hardware_concurrency() return 0\n";
        program_running = false;
        exit(1);
    }


    Useful::PrintOut("subProcess: processing entries",std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);
    Useful::PrintOut(" - loading and parsing tripUpdates from entries to get total S.T.U. size:",std::string::npos,"left","",true,false,false,1,1,&terminalCursorPos);
    transit_realtime::TripUpdate tempTripUpdHolder;
    // std::vector<TrpUpd> LoadedTripUpdates;
    size_t num_failedIstreams = 0;
    size_t tot_numSTU = 0;
    auto itr_entry = entriesToProcess.begin();
    size_t stringLen_maxEntries = Useful::FormatWithSymbol(entriesToProcess.size(), "'").length();
    for(size_t i=0; i<entriesToProcess.size(); i++) {
        std::ifstream entryFile(*itr_entry, std::ios::in | std::ios::binary);
        if(!entryFile.is_open()) {
            num_failedIstreams++;
            std::advance(itr_entry, 1);
            continue;
        }
        tempTripUpdHolder.ParseFromIstream(&entryFile);
        tot_numSTU += tempTripUpdHolder.stop_time_update_size();
        // for(size_t i_upd=0; i_upd<tempTripUpdHolder.stop_time_update_size(); i_upd++) {
        //     LoadedTripUpdates.push_back(ParseDebugString(tempTripUpdHolder.stop_time_update(i_upd).DebugString()));
        // }
        Useful::ANSI_mvprint(
            terminalCursorPos.x+2, terminalCursorPos.y,
            std::string("read entry #")+Useful::formatNumber(i,stringLen_maxEntries,0,"right",false,true)+" / "+Useful::formatNumber(entriesToProcess.size(),stringLen_maxEntries,0,"right",false,true)+
            " | total num StopTimeUpdates: "+Useful::formatNumber(tot_numSTU,10,0,"right",false,true)+
            " | \""+*itr_entry+"\"",
            false
        );
        tempTripUpdHolder.Clear();
        entryFile.close();
        std::advance(itr_entry, 1);
    }
    // exit(0);
    terminalCursorPos.x = 0;
    terminalCursorPos.y+=2;

    
    auto itr_entriesToProcess = entriesToProcess.begin();
    std::vector<std::vector<size_t>> idx_lim;
    for(size_t _thr=0; _thr<maxThreadCount; _thr++) {
        size_t idx_gap = std::ceil(float(entriesToProcess.size())/float(maxThreadCount));
        size_t idx_start = _thr*idx_gap;
        idx_lim.push_back({idx_start, idx_gap}); //{start, gap, progress}
    }
    size_t strLen_totMaxEntries = Useful::formatNumber(idx_lim.at(0).at(1),0,0,"right",false,true).length();
    
    std::list<std::list<parseException_DebugString>> refReturned_caughtExcepts(maxThreadCount, std::list<parseException_DebugString>());
    std::list<std::list<TrpUpd>> refReturned_tripUpdates(maxThreadCount, std::list<TrpUpd>());
    std::list<std::list<STU_refd>> refReturned_tripDelays(maxThreadCount, std::list<STU_refd>());
    std::list<size_t> refReturned_numTotalTripsRead(maxThreadCount, 0);
    auto itr_refRet_caughtExcepts = refReturned_caughtExcepts.begin();
    auto itr_refRet_tripUpdates = refReturned_tripUpdates.begin();
    auto itr_refRet_tripDelays = refReturned_tripDelays.begin();
    auto itr_refRet_numTotalTripsRead = refReturned_numTotalTripsRead.begin();

    auto progressBar_startDate = std::chrono::system_clock::now();

    std::unique_lock<std::mutex> u_lck_cout(mtx_cout, std::defer_lock);

    /// Starting sub-threads
    std::vector<std::thread> threadObjects;
    for(size_t id_thread=1; id_thread<maxThreadCount; id_thread++) {
        u_lck_cout.lock();
        Useful::ANSI_mvprint(42,terminalCursorPos.y+id_thread+1,std::string("init thread: ")+Useful::formatNumber(id_thread));
        u_lck_cout.unlock();
        // if(id_thread+1<maxThreadCount) {
            std::advance(itr_refRet_caughtExcepts, 1);
            std::advance(itr_refRet_tripUpdates, 1);
            std::advance(itr_refRet_tripDelays, 1);
            std::advance(itr_refRet_numTotalTripsRead, 1);
        // }
        
        u_lck_cout.lock();
        Useful::ANSI_mvprint(42,terminalCursorPos.y+id_thread+1,std::string("init thread: ")+Useful::formatNumber(id_thread)+" | 1");
        u_lck_cout.unlock();

        threadObjects.emplace_back([&, id_thread, itr_refRet_caughtExcepts, itr_refRet_tripUpdates, itr_refRet_tripDelays, itr_refRet_numTotalTripsRead] {
            threadTask_parseDelaysFromEntry(
                id_thread,
                entriesToProcess,
                idx_lim.at(id_thread),
                refrTree,
                &(*itr_refRet_caughtExcepts),
                &(*itr_refRet_tripUpdates),
                &(*itr_refRet_tripDelays),
                &(*itr_refRet_numTotalTripsRead)
            );
        });

        u_lck_cout.lock();
        Useful::ANSI_mvprint(42,terminalCursorPos.y+id_thread+1,std::string("init thread: ")+Useful::formatNumber(id_thread)+" | 1:2");
        u_lck_cout.unlock();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    u_lck_cout.lock();
    Useful::ANSI_mvprint(60,terminalCursorPos.y+1,"uno");
    u_lck_cout.unlock();

    itr_refRet_caughtExcepts = refReturned_caughtExcepts.begin();
    itr_refRet_tripUpdates = refReturned_tripUpdates.begin();
    itr_refRet_tripDelays = refReturned_tripDelays.begin();
    itr_refRet_numTotalTripsRead = refReturned_numTotalTripsRead.begin();

    //threadTask_parseDelaysFromEntry

    u_lck_cout.lock();
    Useful::ANSI_mvprint(60,terminalCursorPos.y+1,"dos");
    u_lck_cout.unlock();

    /// ---------- main thread task start ---------- 

    std::vector<std::string> progressBarStr(5, "");
    double progressBar_speed = 0;
    std::chrono::duration<double> progressBar_ETA(0);
    Useful::basicProgressBar(0, tot_numSTU/maxThreadCount, 0, 100,&progressBar_speed,&progressBar_ETA);
    progressBarStr.at(0) = "Start date: ["+Useful::formatDate(progressBar_startDate)+"]";

    size_t strLen_totMaxSTUs = Useful::formatNumber(tot_numSTU,0,1,"right",false,true).size();
    // size_t strLen_totMaxSTUs = Useful::formatNumber(idx_lim.at(0).at(1),0,1,"right",false,true).size();
    u_lck_cout.lock();
    Useful::ANSI_mvprint(42,terminalCursorPos.y+1,std::string("init thread: ")+Useful::formatNumber(0)+" | 2");
    Useful::ANSI_mvprint(4,terminalCursorPos.y+1,Useful::formatNumber(0, 2)+" : ["+Useful::formatNumber(idx_lim.at(0).at(0), 9,1,"right",false,true)+":"+Useful::formatNumber(idx_lim.at(0).at(0)+idx_lim.at(0).at(1), 9,1,"right",false,true)+"] : ");
    u_lck_cout.unlock();

    size_t read_numSTU = 0;
    auto entryPathStr_itr = entriesToProcess.begin();


    ThrdPrf::ThreadPerf perfObj_entryParsing{1000, {
        "Total entry process duration",
        "ParseFromIstream",
        "parse_epochTime_fromFilename",
        "ParseDebugString",
        "tripToSearch_check",
        "StopTimesFile_stopID_search"
    }};
    ThrdPrf::ThreadPerf perfObj_ParseDebugString(1000);
    
    perfObj_entryParsing.set_T_start("Total entry process duration");
    // terminalCursorPos.y+=2;
    for(size_t i=0; i<idx_lim.at(0).at(1); i++) {
        std::fstream entryFile(*entryPathStr_itr, std::ios::in |std::ios::binary);
        
        std::string entry_filename = entryPathStr_itr->substr(Useful::findSubstr("sl-tripupdates-", *entryPathStr_itr));
        int64_t filename_epoch = 0;

        if(!entryFile) {
            // entryPathOpenFailures+=1;
            itr_refRet_caughtExcepts->push_back({"entryFile.isOpen()==false", std::string("\"file:'")+entry_filename+"'\",\"i_upd:-1\",\"exceptCatch_processID:"+std::to_string(-1)+"\""});
            std::advance(entryPathStr_itr, 1);
            continue;
        }
        
        assert(perfObj_entryParsing.set_T_start("ParseFromIstream")>=0);
        transit_realtime::TripUpdate trpUpdate;
        trpUpdate.ParseFromIstream(&entryFile);
        perfObj_entryParsing.set_T_end("ParseFromIstream").multiplier+=1;

        //---------- process start ----------
        
        assert(perfObj_entryParsing.set_T_start("parse_epochTime_fromFilename")>=0);
        try {
            filename_epoch = parse_epochTime_fromFilename(entry_filename);
        }
        catch(const std::exception& e) {
            u_lck_cout.lock();
            Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+maxThreadCount+2, std::string("Failed to parse filename")+e.what()+"["+entry_filename+"]",true);
            u_lck_cout.unlock();
            terminalCursorPos.y++;
        }
        perfObj_entryParsing.set_T_end("parse_epochTime_fromFilename").multiplier+=1;
        
        std::string _tempString__readingEntry = std::string("reading entry #")+Useful::formatNumber(i, 3)+": \""+entry_filename+"\"; S.T.U. size:"+Useful::formatNumber(trpUpdate.stop_time_update_size(), 4)+"  ";
        u_lck_cout.lock();
        Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+maxThreadCount+2, _tempString__readingEntry);
        u_lck_cout.unlock();
        size_t i_upd = 0;
        size_t exceptCatch_processID = 0;
        try {
            for(; i_upd<trpUpdate.stop_time_update_size(); i_upd++) {

                std::string __temp_toParse = trpUpdate.stop_time_update(i_upd).DebugString();
                assert(perfObj_entryParsing.set_T_start("ParseDebugString")>=0);
                TrpUpd parsedStrVar = ParseDebugString(__temp_toParse/*, &perfObj_ParseDebugString*/);
                exceptCatch_processID = 0;
                perfObj_entryParsing.set_T_end("ParseDebugString").multiplier+=1;
                itr_refRet_tripUpdates->push_back(parsedStrVar);

                assert(perfObj_entryParsing.set_T_start("tripToSearch_check")>=0);
                exceptCatch_processID = 1;
                auto tempTrp = itr_refRet_tripUpdates->back();
                bool tripToSearch = false;
                (*itr_refRet_numTotalTripsRead)+=1;
                // for(std::string itr_trip : trip_id__found) {
                //     if(tempTrp.trip.trip_id==itr_trip) {
                //         tripToSearch = true;
                //         break;
                //     }
                // }
                auto ptr_trip = &trip_id__found[0];
                for(size_t i_ptrtrip=0; i_ptrtrip<trip_id__found.size(); i_ptrtrip++) {
                    if(*(ptr_trip+i_ptrtrip) == tempTrp.trip.trip_id) {
                        tripToSearch = true;
                        break;
                    }
                }

                perfObj_entryParsing.set_T_end("tripToSearch_check").multiplier+=1;
                
                if(!tripToSearch) continue;
                
                exceptCatch_processID = 2;
                for(size_t i_stu=0; i_stu<tempTrp.stop_time_updates.size(); i_stu++) {
                    auto tempSTU = tempTrp.stop_time_updates.at(i_stu);
                    if(!(tempSTU.arrival.delay==0 && tempSTU.departure.delay==0)) {
                        #if useStopTimesFile
                        if(tempSTU.stop_id=="" && (tempTrp.trip.trip_id.size()>0 && tempSTU.stop_sequence!=0)) { //use stop_sequence and trip_id to find missing stop_id for this S.T.U
                            assert(perfObj_entryParsing.set_T_start("StopTimesFile_stopID_search")>=0);
                            // Useful::ANSI_mvprint(terminalCursorPos.x+2+_tempString__readingEntry.size(), terminalCursorPos.y+maxThreadCount+1, std::string("stop_id empty. Searching stop_times refr vector:"), false);
                            exceptCatch_processID = 3;
                            
                            bool matchFound = false;
                            auto itrTree = refrTree.find(tempSTU.stop_sequence);
                            if(itrTree!=refrTree.end()) {
                                auto itrTree2 = itrTree->second.find(tempTrp.trip.trip_id);
                                if(itrTree2!=itrTree->second.end()) {
                                    tempSTU.stop_id = itrTree2->second;
                                    matchFound = true;
                                }
                            }

                            u_lck_cout.lock();
                            if(matchFound) Useful::ANSI_mvprint(terminalCursorPos.x+2+_tempString__readingEntry.size()+49, terminalCursorPos.y+maxThreadCount+1, "Match found.  ");
                            else  Useful::ANSI_mvprint(terminalCursorPos.x+2+_tempString__readingEntry.size()+49, terminalCursorPos.y+maxThreadCount+1, "Match not found.");
                            u_lck_cout.unlock();

                            perfObj_entryParsing.set_T_end("StopTimesFile_stopID_search").multiplier+=1;
                        }
                        #endif
                        exceptCatch_processID = 4;
                        itr_refRet_tripDelays->push_back(STU_refd{tempSTU, tempTrp.trip.trip_id, static_cast<uint32_t>(i_stu), filename_epoch});
                    }
                }
                // Useful::ANSI_mvprint(0, 8, Useful::formatNumber(i_upd));
            }
        }
        catch(const std::exception& e) {
            itr_refRet_caughtExcepts->push_back({e.what(), std::string("\"file:'")+entry_filename+"'\",\"i_upd:"+std::to_string(i_upd)+"\",\"exceptCatch_processID:"+std::to_string(exceptCatch_processID)+"\""});
        }

        read_numSTU += i_upd;
        //---------- process end   ----------

        try {
            progressBarStr.at(1) = Useful::formatNumber(read_numSTU, strLen_totMaxSTUs,1,"right",false,true)+"/"+Useful::formatNumber(tot_numSTU, strLen_totMaxSTUs,1,"right",false,true);
            progressBarStr.at(2) = Useful::basicProgressBar(read_numSTU,tot_numSTU/maxThreadCount,1,100,&progressBar_speed,&progressBar_ETA,std::chrono::duration<double>(1));
            // progressBarStr.at(2) = Useful::basicProgressBar(i,idx_lim.at(0).at(1),1,100,&progressBar_speed,&progressBar_ETA,std::chrono::duration<double>(1));
            progressBarStr.at(3) = "rate: "+Useful::formatNumber(progressBar_speed,8,2,"right",false,true)+" S.T.U./sec";
            progressBarStr.at(4) = "ETA: "+Useful::formatDuration(progressBar_ETA);
        }
        catch(const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
        

        u_lck_cout.lock();
        try {
            Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+maxThreadCount+3, std::string("num successfully added delay: ")+std::to_string(itr_refRet_tripDelays->size()), false);
            std::string str_vecExcepts = std::string("num vecExceptions: ")+std::to_string(itr_refRet_caughtExcepts->size());
            if(itr_refRet_caughtExcepts->size()>0) str_vecExcepts += "\n - latest:"+itr_refRet_caughtExcepts->back().what + " , where: " + itr_refRet_caughtExcepts->back().where;
            Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+maxThreadCount+4, str_vecExcepts+"     ", false);
    
            // progressBar = Useful::progressBar(i, idx_lim.at(0).at(1));
            Useful::ANSI_mvprint(0, terminalCursorPos.y+maxThreadCount+5, progressBarStr.at(0),false);
            Useful::ANSI_mvprint(5, terminalCursorPos.y+maxThreadCount+6, progressBarStr.at(1),false);
    
            Useful::ANSI_mvprint(progressBarStr.at(1).size()+6, terminalCursorPos.y+maxThreadCount+6, "", false);
            fmt::print(progressBarStr.at(2));
    
            Useful::ANSI_mvprint(progressBarStr.at(1).size()+10+110, terminalCursorPos.y+maxThreadCount+6, progressBarStr.at(3),false);
            Useful::ANSI_mvprint(0, terminalCursorPos.y+maxThreadCount+7, Useful::formatNumber(progressBarStr.at(4),50,0,"left"),false);
            
            Useful::ANSI_mvprint(progressBarStr.at(4).size()+1, terminalCursorPos.y+maxThreadCount+7, std::string(50-progressBarStr.at(4).size(), ' '), false);
        }
        catch(const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
        
        u_lck_cout.unlock();

        std::advance(entryPathStr_itr, 1);
        
        entryFile.close();
        trpUpdate.Clear();

        perfObj_entryParsing.set_T_end("Total entry process duration").multiplier+=1;
        std::string printStr_memLeft="";
        printStr_memLeft += Useful::formatNumber(Useful::HumanReadable{Useful::getTotalAvailMemory()},  3+1+3+2, 2);
        printStr_memLeft += " / ";
        printStr_memLeft += Useful::formatNumber(Useful::HumanReadable{Useful::getTotalSystemMemory()}, 3+1+3+2, 2);

        std::string printStr_perfObj="";
        size_t maxStrLength__printStr_perfObj = 0;
        for(size_t perfIdx=0; perfIdx<perfObj_entryParsing.size(); perfIdx++) {
            auto& perfObj_el = perfObj_entryParsing[perfIdx];
            double el_delay = std::chrono::duration<double, std::milli>(perfObj_entryParsing.get_avgDuration(perfIdx)).count();
            std::string tempPrintStr = Useful::formatNumber(perfObj_el.label, 30,0,"left") + " : " + (
                perfObj_el.durations.size()==0? 
                "0.000" : Useful::formatNumber(el_delay*perfObj_el.multiplier,8,2) //  std::chrono::duration<double, std::milli>
            ) +" ms | "+Useful::formatNumber(perfObj_el.multiplier, 6, 0, "right", false, true)+" | "+ Useful::formatNumber(el_delay,8,2,"right",false,true);
            if(tempPrintStr.size() > maxStrLength__printStr_perfObj) maxStrLength__printStr_perfObj = tempPrintStr.size();
            printStr_perfObj += tempPrintStr += "\n";
            perfObj_el.multiplier = 0;
        }

        u_lck_cout.lock();
        // std::string printStr_perfObj_ParseDebugString = "";
        // for(size_t perfIdx=0; perfIdx<perfObj_ParseDebugString.size(); perfIdx++) {
        //     auto& perfObj_el = perfObj_ParseDebugString[perfIdx];
        //     double el_delay = std::chrono::duration<double, std::milli>(perfObj_entryParsing.get_avgDuration(perfIdx)).count();
        //     std::string tempPrintStr = Useful::formatNumber(perfObj_el.label, 30,0,"left") + " : " + (
        //         perfObj_el.durations.size()==0? 
        //         "0.000" : Useful::formatNumber(el_delay*perfObj_el.multiplier,8,2) //  std::chrono::duration<double, std::milli>
        //     ) +" ms | "+Useful::formatNumber(perfObj_el.multiplier, 6, 0, "right", false, true)+" | "+ Useful::formatNumber(el_delay,8,2,"right",false,true) +"\n";

        //     Useful::ANSI_mvprint(maxStrLength__printStr_perfObj+1, terminalCursorPos.y+maxThreadCount+9+perfIdx, std::string(" || | || ")+tempPrintStr);
        //     perfObj_el.multiplier = 0;
        // }
        
        Useful::ANSI_mvprint(29+4,terminalCursorPos.y+1, Useful::formatNumber(i,strLen_totMaxEntries,0,"right",false,true)+"/"+Useful::formatNumber(idx_lim.at(0).at(1),strLen_totMaxEntries,0,"right",false,true));
        Useful::ANSI_mvprint(0, terminalCursorPos.y+maxThreadCount+9, printStr_perfObj);
        Useful::ANSI_mvprint(55, terminalCursorPos.y+maxThreadCount+7, printStr_memLeft);
        u_lck_cout.unlock();
        perfObj_entryParsing.set_T_start("Total entry process duration");

    }
    // terminalCursorPos.y += 9 + perfObj_entryParsing.size();

    /// ---------- main thread task end   ---------- 

    if(maxThreadCount>1) {
        u_lck_cout.lock();
        Useful::ANSI_mvprint(0, terminalCursorPos.y+maxThreadCount+1+14, std::string("finished main thread's process: waiting for other threads"));
        u_lck_cout.unlock();
        for(size_t i=0; i<threadObjects.size(); i++) {
            if(threadObjects.at(i).joinable()) threadObjects.at(i).join();
        }
    }
    else if(threadObjects.size()>0) throw std::runtime_error("threadObjects.size()>0 && maxThreadCount==0");
    

    terminalCursorPos.x = 0;
    terminalCursorPos.y += maxThreadCount+15;

    Useful::PrintOut("Saving individual thread's saved data into collective container.",std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);

    std::vector<parseException_DebugString> vecExceptions_DebugString;
    // std::vector<TrpUpd> storedData_tripUpdates;
    storedData_tripDelays_idx.clear();
    numTotalTripsRead = 0;

    Useful::PrintOut("   -saving refReturned_tripDelays    : ",std::string::npos,"left"," ",true,false,false,1,1,&terminalCursorPos);
    for(auto itr_refRet : refReturned_tripDelays) {
        for(auto itr_temp : itr_refRet) {
            storedData_tripDelays_idx.push_back(itr_temp);
            
            std::string printStr_memLeft="";
            printStr_memLeft += Useful::formatNumber(Useful::HumanReadable{Useful::getTotalAvailMemory()}, 10, 1);
            printStr_memLeft += " / ";
            printStr_memLeft += Useful::formatNumber(Useful::HumanReadable{Useful::getTotalSystemMemory()}, 10, 1);

            Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, printStr_memLeft);
        }
    }
    terminalCursorPos.x = 0;
    terminalCursorPos.y+= 1;
    Useful::PrintOut("   -saving vecExceptions_DebugString : ",std::string::npos,"left"," ",true,false,false,1,1,&terminalCursorPos);
    for(auto itr_refRet : refReturned_caughtExcepts) {
        for(auto itr_temp : itr_refRet) {
            vecExceptions_DebugString.push_back(itr_temp);
            
            std::string printStr_memLeft="";
            printStr_memLeft += Useful::formatNumber(Useful::HumanReadable{Useful::getTotalAvailMemory()}, 10, 1);
            printStr_memLeft += " / ";
            printStr_memLeft += Useful::formatNumber(Useful::HumanReadable{Useful::getTotalSystemMemory()}, 10, 1);

            Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, printStr_memLeft);
        }
    }
    terminalCursorPos.x = 0;
    terminalCursorPos.y+= 1;
    for(auto itr_temp : refReturned_numTotalTripsRead) {
        numTotalTripsRead += itr_temp;
    }

    for(size_t id_thread=0; id_thread<maxThreadCount; id_thread++) {
        // Useful::ANSI_mvprint(dim_terminal.y-15, terminalCursorPos.y, std::string(" {check:[ 2:0"));

        // Useful::ANSI_mvprint(dim_terminal.y-15, terminalCursorPos.y, std::string(" {check:[ 2:0"));
        // for(auto itr_temp : *itr_refRet_caughtExcepts) vecExceptions_DebugString.push_back(itr_temp);
        // std::advance(itr_refRet_caughtExcepts, 1);
        
        // Useful::ANSI_mvprint(dim_terminal.y-15, terminalCursorPos.y, std::string(" {check:[ 2:1"));
        // for(auto itr_temp : *itr_refRet_tripDelays) storedData_tripDelays_idx.push_back(itr_temp);
        // std::advance(itr_refRet_tripDelays, 1);
        
        // Useful::ANSI_mvprint(dim_terminal.y-15, terminalCursorPos.y, std::string(" {check:[ 2:2"));
        // for(auto itr_temp : *itr_refRet_tripUpdates) storedData_tripUpdates.push_back(itr_temp);
        // std::advance(itr_refRet_tripUpdates, 1);

        // Useful::ANSI_mvprint(dim_terminal.y-15, terminalCursorPos.y, std::string(" {check:[ 2:3"));
        // numTotalTripsRead += *itr_refRet_numTotalTripsRead;
        // std::advance(itr_refRet_numTotalTripsRead, 1);
    }
    // Useful::ANSI_mvprint(dim_terminal.y-15, terminalCursorPos.y, " {check:[ 3]} ");

    Useful::PrintOut("Finished processing entries.", std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);
    Useful::PrintOut(std::string("Total time: ")+Useful::formatDuration(std::chrono::system_clock::now()-progressBar_startDate), std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);

    terminalCursorPos.y++;
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y,  "Total numbers:");
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+1,std::string("    - caughtExceptions : ")+Useful::formatNumber(vecExceptions_DebugString.size(),1,1,"right",false,true));
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+2,std::string("    - tripDelays       : ")+Useful::formatNumber(storedData_tripDelays_idx.size(),1,1,"right",false,true));
    terminalCursorPos.y+=3;

    return vecExceptions_DebugString;
}

