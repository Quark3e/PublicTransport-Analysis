
#include <main_subprocess.hpp>



StopID_refrSorted subProcess_loadFile__stop_times(
    std::string filename,
    std::vector<std::vector<std::string>>& returVecRef,
    std::vector<std::string> tripFilter
) {
    size_t maxThreadCount = 1; //std::thread::hardware_concurrency();
    if(maxThreadCount==0) {
        std::cerr << "std::thread::hardware_concurrency() return 0\n";
        program_running = false;
        exit(1);
    }

    std::ifstream in_stream(filename);
    size_t columnCount = 10;
    Useful::PrintOut("Checking number of lines in stop_times refr. file..", dim_terminal.x, "left","",true,false,false,1,1,&terminalCursorPos);
    size_t stop_times_max_lineCount = std::count_if(std::istreambuf_iterator<char>{in_stream}, {}, [](char c) { return c == '\n'; });
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, std::string("[maxThreadCount:")+std::to_string(maxThreadCount)+", stop_times file line count:"+Useful::formatNumber(stop_times_max_lineCount,dim_terminal.x+1,1,"right",false,true)+"]");
    terminalCursorPos.y++;
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
    std::advance(itr__result_threadTask, 1);

    auto lambdaFunc_parse_csv__stop_times = [tripFilter](std::string _line, bool* _tripFound=nullptr) {
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
    
    threadTask_loadFile__startIdxCnt = 0;

    /// Initialise the different threads.
    for(size_t id_thread=1; id_thread<maxThreadCount; id_thread++) {
        threadObjects.emplace_back([&] {
            threadTask_loadFile<std::vector<std::string>>(
                returVecRef,
                filename,
                lambdaFunc_parse_csv__stop_times,
                id_thread,
                maxThreadCount,
                &*itr__result_threadTask
            );
        });
        std::advance(itr__result_threadTask, 1);
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

    terminalCursorPos.y++;
    // std::list<size_t>& ref_refrFoundIdx = *refrFoundIdx.begin();
    for(size_t lineCount=0; std::getline(fileToRead, _line, '\n'); lineCount++) {
        if(_line.size()==0) continue;
        bool _tripIdFromStopTimesFound = false;
        try {
            returVecRef.at(lineCount) = lambdaFunc_parse_csv__stop_times(_line, &_tripIdFromStopTimesFound);
            //thread issues????
            
            // if(_tripIdFromStopTimesFound) {
            //     ref_refrFoundIdx.push_back(lineCount);
            // }
        }
        catch(const std::exception& e) {
            *itr__result_threadTask = false;
            fileToRead.close();
            break;
        }
        
        for(size_t _ign=0; _ign<maxThreadCount-1; _ign++) { //thread ID based line skipping.
            fileToRead.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            if(fileToRead.rdstate()==std::ios::eofbit) break;
            lineCount++;
        }

        time_t2 = std::chrono::steady_clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(interval_dur=(time_t2-time_t1)).count()>=ms_interval || lineCount>=stop_times_max_lineCount) {
            size_t delta_lineCount = lineCount-delta_lineCount_t1;
            double speed = static_cast<double>(delta_lineCount)/std::chrono::duration_cast<std::chrono::seconds>(interval_dur).count();
            double ETA_seconds = double(stop_times_max_lineCount-lineCount)/speed;
            std::string _printStr = "curr. lineCount:"+ Useful::formatNumber(lineCount,9,1,"right",false,true)+" / "+Useful::formatNumber(stop_times_max_lineCount,9,3,"right",false,true);
            _printStr += " , process rate:"+Useful::formatNumber(speed,5,0)+" p/s , ETA:";
            double _temp = 0;
            if(ETA_seconds>3600) {
                _printStr += std::to_string(int(std::floor(ETA_seconds/3600)))+" hours ";
                ETA_seconds = std::modf(ETA_seconds/3600, &_temp)*3600;
            }
            if(ETA_seconds>60) {
                _printStr += std::to_string(int(std::floor(ETA_seconds/60)))+" minutes ";
            }
            _printStr += std::to_string(int(std::modf(ETA_seconds/60, &_temp)*60)) + " seconds.";

            Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, _printStr, false);

            time_t1 = time_t2;
            delta_lineCount_t1 = lineCount;
        }
    }
    terminalCursorPos.y+= 2;
    terminalCursorPos.x = 0;

    fileToRead.close();
    *itr__result_threadTask = true;
    ///--- Main thread task end ---

    for(size_t i=0; i<threadObjects.size(); i++) {
        if(threadObjects.at(i).joinable()) threadObjects.at(i).join();
    }
    Useful::PrintOut("finished main thread threadTask.",dim_terminal.x+1,"left","\n",true,false,false,1,1,&terminalCursorPos);
    
    // returVecRef vector cleanup
    bool emptyVectorFound = false;
    Useful::PrintOut(std::string("Performing vector cleanup. "),dim_terminal.x+1,"left","",true,false,false,1,1,&terminalCursorPos);
    size_t cleanupDiff = returVecRef.size();

    std::vector<std::vector<std::string>> _tempCopy;
    for(auto itr_threadID=refrFoundIdx.begin(); itr_threadID!=refrFoundIdx.end(); ++itr_threadID) {
        for(auto itr_found=itr_threadID->begin(); itr_found!=itr_threadID->end(); ++itr_found) {
            _tempCopy.push_back(returVecRef.at(*itr_found));
            Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, std::string("total found size: ")+std::to_string(_tempCopy.size())+"   ", false);
        }
    }
    returVecRef = _tempCopy;
    cleanupDiff -= returVecRef.size();

    StopID_refrSorted refrRetur;
    for(size_t i=0; i<returVecRef.size(); i++) {
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


std::vector<parseException_DebugString> subProcess_processEntries(std::list<std::string> entriesToProcess, StopID_refrSorted& refrTree) {
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, "processing entries:");
    terminalCursorPos.y++;

    std::string progressBar = "";
    size_t numTotalTripsRead = 0;
    size_t entryPathOpenFailures = 0;
    auto entryPathStr_itr = entriesToProcess.begin();
    
    std::vector<TrpUpd> storedData;
    std::vector<STU_refd> storedData_tripDelays_idx;

    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, "", true);


    std::vector<parseException_DebugString> vecExceptions_DebugString;

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

