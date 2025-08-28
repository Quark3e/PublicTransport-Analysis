
#include <main_subprocess.hpp>



StopID_refrSorted subProcess_loadFile__stop_times(
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

    std::ifstream in_stream(filename);
    size_t columnCount = 10;
    Useful::PrintOut("Checking number of lines in stop_times refr. file..", std::string::npos, "left","",true,false,false,1,1,&terminalCursorPos);
    size_t stop_times_max_lineCount = std::count_if(std::istreambuf_iterator<char>{in_stream}, {}, [](char c) { return c == '\n'; });
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, std::string("[maxThreadCount:")+std::to_string(maxThreadCount)+", stop_times file line count:"+Useful::formatNumber(stop_times_max_lineCount,0,1,"right",false,true)+"]");
    
    terminalCursorPos.y +=4;
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
        size_t idx_gap = float(stop_times_max_lineCount)/float(maxThreadCount);
        size_t idx_start = _thr*idx_gap;
        idx_lim.push_back({idx_start, idx_gap}); //{start, gap, progress}
        // idx_progr.push_back(0);
    }

    std::chrono::system_clock::time_point times_threadTasks_start = std::chrono::system_clock::now();
    auto times_threadTasks_start_str = std::chrono::system_clock::to_time_t(times_threadTasks_start);
    Useful::PrintOut(std::string("Process: threadTask: started : ")+std::ctime(&times_threadTasks_start_str),std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);

    /// Initialise the different threads.
    for(size_t id_thread=1; id_thread<maxThreadCount; id_thread++) {
        std::advance(itr__result_threadTask, 1);
        std::advance(itr__foundIdx_threadTask, 1);
        std::advance(itr__idx_progr_threadTask, 1);
        threadObjects.emplace_back([&, id_thread, filename] {
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
                &*itr__result_threadTask
            );
        });
    }

    ///--- Main thread task start --- ///!! NOTE: NEED TO IMPLEMENT CHANGES BEFORE USING MULTITHREADING

    std::ifstream fileToRead(filename);
    itr__result_threadTask = result_threadTask.begin();
    if(!fileToRead.is_open()) {
        *itr__result_threadTask = false;
    }
    std::string _line = "";

    auto time_start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point time_t1=time_start, time_t2;
    
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

            if(_tripIdFromStopTimesFound) ref_refrFoundIdx.push_back(idx_lim.at(0).at(0)+lineCount);
        }
        catch(const std::exception& e) {
            *itr__result_threadTask = false;
            fileToRead.close();
            break;
        }


        time_t2 = std::chrono::steady_clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(interval_dur=(time_t2-time_t1)).count()>=ms_interval || lineCount>=stop_times_max_lineCount/maxThreadCount) {
            size_t sum_lineCount = 0;
            std::string printStr_sum_lineCount("thread progresses: {");
            progr_percent = double(lineCount)/idx_lim.at(0).at(1);
            *idx_progr.begin() = lineCount;
            u_lck_cout.lock();
            for(auto idx_itr=idx_progr.begin(); idx_itr!=idx_progr.end(); ++idx_itr) {
                sum_lineCount += *idx_itr;
                printStr_sum_lineCount += Useful::formatNumber(*idx_itr,8,1,"right",false,true) + ",";
            }
            printStr_sum_lineCount+="}";
            // for(size_t cnt=0; cnt<idx_progr.size(); cnt++) sum_lineCount += idx_progr.at(cnt)._a.load();
            Useful::ANSI_mvprint(0, terminalCursorPos.y+maxThreadCount+3, printStr_sum_lineCount, false);
            u_lck_cout.unlock();
            
            
            size_t delta_lineCount = sum_lineCount-delta_lineCount_t1;
            double speed = static_cast<double>(delta_lineCount)/std::chrono::duration_cast<std::chrono::seconds>(interval_dur).count();
            double ETA_seconds = double(stop_times_max_lineCount-sum_lineCount)/speed;
            std::string _printStr = "curr. sum_lineCount:"+ Useful::formatNumber(sum_lineCount,9,1,"right",false,true)+" / "+Useful::formatNumber(stop_times_max_lineCount,9,3,"right",false,true);
            _printStr += " , process rate:"+Useful::formatNumber(speed,5,0)+" p/s , ETA:";
            
            _printStr += Useful::formatDuration(std::chrono::duration<double>(ETA_seconds));
            
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
    Useful::ANSI_mvprint(0, terminalCursorPos.y+1+maxThreadCount+5, std::string("finished main thread's process: waiting for other threads"));
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
    Useful::PrintOut(std::string("Performing vector cleanup. "),dim_terminal.x+1,"left","\n",true,false,false,1,1,&terminalCursorPos);
    size_t cleanupDiff = returVecRef.size();

    Useful::PrintOut(std::string("refrFoundIdx: "+std::to_string(refrFoundIdx.size())),dim_terminal.x+1,"left","\n",true,false,false,1,1,&terminalCursorPos);
    std::vector<std::vector<std::string>> _tempCopy;
    for(auto itr_threadID=refrFoundIdx.begin(); itr_threadID!=refrFoundIdx.end(); ++itr_threadID) {
        for(auto itr_found=itr_threadID->begin(); itr_found!=itr_threadID->end(); ++itr_found) {
            _tempCopy.push_back(returVecRef.at(*itr_found));
            Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, std::string("total found size: ")+std::to_string(_tempCopy.size())+"   ", false);
        }
    }
    
    returVecRef = _tempCopy;
    cleanupDiff -= returVecRef.size();
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+1, std::string("cleanup diff: ")+std::to_string(cleanupDiff), false);
    
    ///--- save the cleaned up file as a temporary

    terminalCursorPos.y += 1;

    Useful::PrintOut("Saving cleaned up stop_times file into a temporary file \"returVecRef_temp.csv\"",std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);
    std::ofstream file_returVecRef_temp("returVecRef_temp.csv");
    if(file_returVecRef_temp.is_open()) {
        for(size_t i=0; i<returVecRef.size(); i++) {
            for(size_t ii=0; ii<returVecRef.at(i).size(); ii++) {
                file_returVecRef_temp << returVecRef.at(i).at(ii) << (ii+1<returVecRef.at(i).size()? "," : "\n");
            }
        }
    }
    file_returVecRef_temp.close();
    Useful::PrintOut("Finished saving file.",std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);
    
    ///---

    
    
    terminalCursorPos.y+=2;

    Useful::PrintOut("Performing refrSorted tree fill.",std::string::npos,"left","\n",true,false,false,1,1,&terminalCursorPos);

    Useful::PrintOut("");
    StopID_refrSorted refrRetur;
    try {
        for(size_t i=0; i<returVecRef.size(); i++) {
            Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y-1, std::string("returVecRef.at(")+std::to_string(i)+").size() : "+Useful::formatNumber(returVecRef.at(i).size(),2));
            uint32_t _stop_seq = (returVecRef.at(i).at(4).size()>0? std::stoi(returVecRef.at(i).at(4)) : 0);
            std::string _trip_id = returVecRef.at(i).at(0);
            std::string _stop_id = returVecRef.at(i).at(3);
            bool found__stop_seq    = false;
            bool found__trip_id     = false;
            for(size_t ii=0; ii<refrRetur.vec.size(); ii++) { //check every stop_seq
                if(refrRetur.vec.at(ii).stop_sequence == _stop_seq) {
                    found__stop_seq = true;
                    found__trip_id = false;
                    for(size_t iii=0; iii<refrRetur.vec.at(ii).stop_seq_vec.size(); iii++) { //check every trip_id
                        if(refrRetur.vec.at(ii).stop_seq_vec.at(iii).trip_id==_trip_id) {
                            found__trip_id = true;
                            refrRetur.vec.at(ii).stop_seq_vec.at(iii).stop_id = _stop_id;
    
                            break;
                        }
                    }
                    if(!found__trip_id) {
                        refrRetur.vec.at(ii).stop_seq_vec.push_back({_trip_id, {_stop_id}});
                    }
                    break;
                }
            }
            if(!found__stop_seq) {
                refrRetur.vec.push_back({_stop_seq, {{_trip_id, {_stop_id}}}});
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
    std::chrono::duration<double> time_totalThreadTask_elapsed = (std::chrono::steady_clock::now()-time_start);
    auto time_totalThreadTask__hr   = std::chrono::duration_cast<std::chrono::hours>(time_totalThreadTask_elapsed);
    auto time_totalThreadTask__mins = std::chrono::duration_cast<std::chrono::minutes>(time_totalThreadTask_elapsed - time_totalThreadTask__hr);
    auto time_totalThreadTask__secs = std::chrono::duration_cast<std::chrono::seconds>(time_totalThreadTask_elapsed - time_totalThreadTask__hr - time_totalThreadTask__mins);
    Useful::PrintOut(
        std::string("   total time: ") + 
        std::to_string(time_totalThreadTask__hr.count())+" hours, " + 
        std::to_string(time_totalThreadTask__mins.count())+" minutes, " + 
        std::to_string(time_totalThreadTask__secs.count())+" seconds",
        dim_terminal.x, "left","\n",true,false,false,1,1,&terminalCursorPos
    );
    return refrRetur;
}


std::vector<parseException_DebugString> subProcess_processEntries(
    std::list<std::string> entriesToProcess,
    StopID_refrSorted& refrTree,
    size_t& numTotalTripsRead,
    size_t& entryPathOpenFailures,
    std::vector<STU_refd>& storedData_tripDelays_idx
) {
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, "processing entries:");
    terminalCursorPos.y++;

    std::string progressBar = "";
    std::vector<TrpUpd> storedData;
    std::vector<parseException_DebugString> vecExceptions_DebugString;
    
    auto entryPathStr_itr = entriesToProcess.begin();

    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, "", true);


    terminalCursorPos.y+=2;
    for(size_t i=0; i<entriesToProcess.size(); i++) {
        std::fstream entryFile(*entryPathStr_itr, std::ios::in |std::ios::binary);
        if(!entryFile) {
            entryPathOpenFailures+=1;
            std::advance(entryPathStr_itr, 1);
            continue;
        }
        
        transit_realtime::TripUpdate trpUpdate;
        trpUpdate.ParseFromIstream(&entryFile);
        //---------- process start ----------
        
        std::string entry_filename = entryPathStr_itr->substr(Useful::findSubstr("sl-tripupdates-", *entryPathStr_itr));
        int64_t filename_epoch = 0;
        try {
            filename_epoch = parse_epochTime_fromFilename(entry_filename);
        } catch(const std::exception& e) {
            Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, std::string("Failed to parse filename")+e.what()+"["+entry_filename+"]",true);
            terminalCursorPos.y++;
        }
        
        std::string _tempString__readingEntry = std::string("reading entry #")+Useful::formatNumber(i, 3)+": \""+entry_filename+"\"; S.T.U. size:"+Useful::formatNumber(trpUpdate.stop_time_update_size(), 4)+"  ";
        Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, _tempString__readingEntry);
        size_t i_upd = 0;
        size_t exceptCatch_processID = 0;
        try {
            for(; i_upd<trpUpdate.stop_time_update_size(); i_upd++) {
                exceptCatch_processID = 0;
                storedData.push_back(ParseDebugString(trpUpdate.stop_time_update(i_upd).DebugString()));
                exceptCatch_processID = 1;
                auto tempTrp = storedData.back();
                bool tripToSearch = false;
                numTotalTripsRead+=1;
                for(std::string itr_trip : trip_id__found) {
                    if(tempTrp.trip.trip_id==itr_trip) {
                        tripToSearch = true;
                        break;
                    }
                }
                exceptCatch_processID = 2;
                if(!tripToSearch) continue;
                for(size_t i_stu=0; i_stu<tempTrp.stop_time_updates.size(); i_stu++) {
                    auto tempSTU = tempTrp.stop_time_updates.at(i_stu);
                    if(!(tempSTU.arrival.delay==0 && tempSTU.departure.delay==0)) {
                        #if useStopTimesFile
                        if(tempSTU.stop_id=="" && (tempTrp.trip.trip_id.size()>0 && tempSTU.stop_sequence!=0)) { //use stop_sequence and trip_id to find missing stop_id for this S.T.U
                            Useful::ANSI_mvprint(terminalCursorPos.x+2+_tempString__readingEntry.size(), terminalCursorPos.y, std::string("stop_id empty. Searching stop_times refr vector:"), false);
                            exceptCatch_processID = 3;
                            bool _refrMatchFound = false;
                            
                            for(auto i_stopseq : refrTree.vec) {
                                if(tempSTU.stop_sequence==i_stopseq.stop_sequence) {
                                    for(auto i_tripid : i_stopseq.stop_seq_vec) {
                                        if(tempTrp.trip.trip_id==i_tripid.trip_id) {
                                            tempSTU.stop_id = i_tripid.stop_id;
                                            _refrMatchFound = true;
                                            break;
                                        }
                                    }
                                }
                                if(_refrMatchFound) break;
                            }
                            if(_refrMatchFound) Useful::ANSI_mvprint(terminalCursorPos.x+2+_tempString__readingEntry.size()+49, terminalCursorPos.y, "Match found.  ");
                            else  Useful::ANSI_mvprint(terminalCursorPos.x+2+_tempString__readingEntry.size()+49, terminalCursorPos.y, "Match not found.");
                        }
                        #endif
                        exceptCatch_processID = 4;
                        storedData_tripDelays_idx.push_back(STU_refd{tempSTU, tempTrp.trip.trip_id, static_cast<uint32_t>(i_stu), filename_epoch});
                    }
                }
                // Useful::ANSI_mvprint(0, 8, Useful::formatNumber(i_upd));
            }
        } catch(const std::exception& e) {
            vecExceptions_DebugString.push_back({e.what(), std::string("\"file:'")+entry_filename+"'\",\"i_upd:"+std::to_string(i_upd)+"\",\"exceptCatch_processID:"+std::to_string(exceptCatch_processID)+"\""});
        }

        //---------- process end   ----------
        Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+2, std::string("num successfully added delay: ")+std::to_string(storedData_tripDelays_idx.size()), false);
        std::string str_vecExcepts = std::string("num vecExceptions: ")+std::to_string(vecExceptions_DebugString.size());
        if(vecExceptions_DebugString.size()>0) str_vecExcepts += "\n - latest:"+vecExceptions_DebugString.back().what + " , where: " + vecExceptions_DebugString.back().where;
        Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+3, str_vecExcepts+"     ", false);

        progressBar = Useful::progressBar(i, entriesToProcess.size());
        Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+5, "", false);
        fmt::print(progressBar);
        std::advance(entryPathStr_itr, 1);
        entryFile.close();
        trpUpdate.Clear();
    }

    return vecExceptions_DebugString;
}

