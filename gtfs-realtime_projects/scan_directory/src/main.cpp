
#include <includes.hpp>


Pos2d<size_t>   terminalCursorPos{0, 1};
Pos2d<int>      dim_terminal{0, 0};

void func_depthSearch(std::string _dirPath, std::list<std::string> *_storeResult, int _maxDepth, size_t *_numDirSearched=nullptr);
void func_depthSearch(std::string _dirPath, std::list<std::string> *_storeResult, int _maxDepth, size_t *_numDirSearched) {
    // int _depthCount = 0;
    for(auto _entry : std::filesystem::directory_iterator(_dirPath)) {
        if(std::filesystem::is_directory(_entry.path()) && _maxDepth>0) {
            if(_numDirSearched) (*_numDirSearched) += 1;
            func_depthSearch(_entry.path().string(), _storeResult, _maxDepth-1, _numDirSearched);
        }
        else if(_entry.path().extension()==".pb") {
            _storeResult->push_back(_entry.path().string());
        }
    }
};

#include <functional>
#include <numeric>


// #define useTripFilter
#define useStopTimesFile true

struct StopID_refrSorted__trip_id {
    std::string trip_id;
    std::vector<std::string> trip_id_vec;
};
struct StopID_refrSorted__stop_seq {
    uint32_t stop_sequence;
    std::vector<StopID_refrSorted__trip_id> stop_seq_vec;
};
struct StopID_refrSorted {
    std::vector<StopID_refrSorted__stop_seq> vec;
};


template<typename _vecType>
void threadTask_loadFile(
    std::vector<_vecType>& returnVec,
    std::string filename,
    std::function<_vecType(std::string file_line)> modifierFunc,
    size_t thread_ID,
    size_t totalNumThreads,
    bool* success = nullptr
) {
    std::ifstream fileToRead(filename);
    if(!fileToRead.is_open()) {
        if(success) *success = false;
        fileToRead.close();
        return;
    }
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
        
        for(size_t _ign=0; _ign<totalNumThreads-1; _ign++) {
            fileToRead.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            if(fileToRead.rdstate()==std::ios::eofbit) break;
            lineCount++;
        }
    }

    fileToRead.close();
    if(success) *success = true;
}

StopID_refrSorted func_loadFile__stop_times(
    std::string filename,
    std::vector<std::vector<std::string>>& returVecRef,
    std::vector<std::string> tripFilter
) {
    size_t maxThreadCount = 1; //std::thread::hardware_concurrency();
    if(maxThreadCount==0) {
        std::cerr << "std::thread::hardware_concurrency() return 0\n";
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
        exit(1);
    }
    
    returVecRef = std::vector<std::vector<std::string>>(stop_times_max_lineCount, std::vector<std::string>(columnCount, ""));
    std::list<std::list<size_t>> refrFoundIdx(maxThreadCount, std::list<size_t>());
    std::vector<std::thread> threadObjects;
    std::list<bool> result_threadTask(maxThreadCount, false);
    auto itr__result_threadTask = result_threadTask.begin();
    std::advance(itr__result_threadTask, 1);
    auto lambdaFunc = [tripFilter](std::string _line, bool* _tripFound=nullptr) {
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
    for(size_t id_thread=1; id_thread<maxThreadCount; id_thread++) {
        threadObjects.emplace_back([&] {
            threadTask_loadFile<std::vector<std::string>>(
                returVecRef,
                filename,
                lambdaFunc,
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
    size_t NumSuccessLines = 0; //temporary. Prob. won't be usable later on with multithreading enabled.
    std::list<size_t>& ref_refrFoundIdx = *refrFoundIdx.begin();
    for(size_t lineCount=0; std::getline(fileToRead, _line, '\n'); lineCount++) {
        if(_line.size()==0) continue;
        bool _tripIdFromStopTimesFound = false;
        try {
            returVecRef.at(lineCount) = lambdaFunc(_line, &_tripIdFromStopTimesFound);
            //thread issues????
            
            if(_tripIdFromStopTimesFound) {
                ref_refrFoundIdx.push_back(lineCount);
                NumSuccessLines+=1;
            }
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
    // size_t cnt = 0;
    // for(auto itr=returVecRef.begin(); itr!=returVecRef.end(); ++itr) {
    //     if(itr->size()==0) emptyVectorFound = true;
    //     while (itr->size()==0) {
    //         itr = returVecRef.erase(itr);
    //         cnt++;
    //     }
    //     if(emptyVectorFound) {
    //         Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, std::to_string(returVecRef.size())+"   ", false);
    //         emptyVectorFound = false;
    //     }
    // }
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
        size_t _stop_seq = (returVecRef.at(4).size()>0? std::stoi(returVecRef.at(4)) : 0);
        std::string _trip_id = returVecRef.at(0);
        std::string _stop_id = returVecRef.at(3);
        bool found__stop_seq    = false;
        bool found__trip_id     = false;
        for(size_t ii=0; ii<refrRetur.vec.size(); ii++) { //check every stop_seq
            if(refrRetur.vec.at(ii).stop_sequence == _stop_seq) {
                found__stop_seq = true;
                found__trip_id = false;
                for(size_t iii=0; iii<refrRetur.vec.at(ii).stop_seq_vec.size(); iii++) { //check every trip_id
                    if(refrRetur.vec.at(ii).stop_seq_vec.at(iii).trip_id==_trip_id) {
                        found__trip_id = true;
                        /// assuming there are no copies of trip_id's for a given stop_id and stop_seq, a check for dupes isn't done. Will need to be tested when this code runs on main machine later on
                        /// !!! NOTE: If this check below doesnt return positive then dont forget to delete it. The .trip_id_vec also needs to be removed since theoretically there should only exist one stop_id.
                        for(size_t n=0; n<refrRetur.vec.at(ii).stop_seq_vec.at(iii).trip_id_vec.size(); n++) {
                            if(refrRetur.vec.at(ii).stop_seq_vec.at(iii).trip_id_vec.at(n)==_stop_id) {
                                std::cout << "STOP_ID_MATCH FOUND!!!"<<std::endl;
                                system("pause");
                            }
                        }
                        refrRetur.vec.at(ii).stop_seq_vec.at(iii).trip_id_vec.push_back(_stop_id);

                        break;
                    }
                }
                if(!found__trip_id) {
                    refrRetur.vec.at(ii).vec.push_back({_trip_id, {_stop_id}});
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

}


struct STU_refd : public STU {
    std::string trip_id;
    uint32_t STU_idx;
    int64_t filename_epoch;

    STU_refd(STU _stu, std::string _trip_id, uint32_t _stu_idx, int64_t _file_epoch): STU{_stu}, trip_id{_trip_id}, STU_idx{_stu_idx}, filename_epoch{_file_epoch} {}
};

struct parseException_DebugString {
    std::string what;
    std::string where;
};



int64_t parse_epochTime_fromFilename(std::string _toParse);

int main(int argc, char** argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    dim_terminal = Useful::getTerminalSize();
    Useful::clearScreen();
    Useful::PrintOut("Program start.", dim_terminal.x, "left","\n",true,false,false,1,1,&terminalCursorPos);

    if(argc <= 1) {
        std::cerr << "Invalid number of argc. No path to directory given." << std::endl;
        return 1;
    }

    std::string path_dirToSearch;
    path_dirToSearch = argv[1];
    if(!std::filesystem::is_directory(path_dirToSearch)) {
        std::cerr << "ERROR: the program argument for path given is not a valid path." << std::endl;
        return 1;
    }

    int search_depth = 1;
    std::string path_static_historical_data = "C:/Users/berkh/Projects/Github_repo/PublicTransport-Analysis/dataset/static_historical_data/GTFS-SL-2025-01-22";

    std::vector<std::string> route_id__toSearch{"9011001004000000", "9011001004100000"}; //for now, we just check for everything on this route, which may include trips
    // std::vector<std::string> shape_id__toSearch{
    //     "4014010000492969158", // Stockholm City -> Södertälje Centrum
    //     "4014010000492969248", // Uppsala C -> Södertälje Centrum
    //     "4014010000492970270", // Södertälje Centrum -> Märsta
    // }; //way too many shape_id's that point to specific trips even if they're the exact same shape. Need to automate this.
    std::vector<std::string> trip_id__found;
    std::vector<std::vector<std::string>> staticRefData__stop_times;


    Useful::PrintOut("loading file_static_refData__trips vector with trip values which's route_id matched..", dim_terminal.x, "left", "",true,false,false,1,1,&terminalCursorPos);
    std::fstream file_static_refData__trips(path_static_historical_data+"/trips.csv", std::ios::in);
    for(std::string _line; std::getline(file_static_refData__trips, _line, '\n');) {
        bool matched = false;
        for(std::string route : route_id__toSearch) {
            if(route==_line.substr(0, 16)) {
                trip_id__found.push_back(_line.substr(_line.find(',', 17)+1, 17));
                break;
            }
        }
    }
    std::cout << " found " << trip_id__found.size() << " num trips.\n";
    terminalCursorPos.x = 0;
    terminalCursorPos.y++;
    if(trip_id__found.size()==0) {
        std::cerr << "No trips to search for found.\n";
        exit(1);
    }
    
    std::fstream file_saveTripsToFind(std::string("dataFile_tripsToFind.csv"), std::ios::out);
    if(!file_saveTripsToFind.is_open()) {
        std::cerr << "Failed to open file_saveTripsToFind file.\n";
        exit(1);
    }
    file_saveTripsToFind << "trip_id\n";
    for(std::string _tripsToFind : trip_id__found) {
        file_saveTripsToFind << _tripsToFind << "\n";
    }
    file_saveTripsToFind.close();

    // Useful::PrintOut("loading file_static_refData__stop_times with reference data from stop_times.csv..");
    // std::fstream file_static_refData__stop_times(path_static_historical_data+"/stop_times.csv", std::ios::in);
    // if(!file_static_refData__stop_times.is_open()) {
    //     std::cerr << "failed to open file_static_refData__stop_times fstream object.\n";
    //     exit(1);
    // }
    // try {
    //     size_t numLineRead=0;
    //     for(std::string _line; std::getline(file_static_refData__stop_times, _line, '\n');) {
    //         for(std::string _trip_id : trip_id__found) {
    //             if(_line.substr(0, _line.find('\n'))==_trip_id) {
    //                 size_t comma_idx = 0;
    //                 staticRefData__stop_times.push_back(std::vector<std::string>());
    //                 do {
    //                     staticRefData__stop_times.back().push_back(_line.substr(comma_idx+1, _line.find(',', comma_idx+1)-comma_idx-1));
    //                     comma_idx = _line.find(',', comma_idx+1);
    //                 } while (comma_idx!=std::string::npos);
    //                 break;
    //             }
    //         }
    //         numLineRead+=1;
    //         Useful::ANSI_mvprint(105, 3, "currently read: "+Useful::formatNumber(numLineRead,0,1,"right",false,true)+" lines.", false);
    //     }
    // }
    // catch(const std::exception& e) {
    //     std::cerr << e.what() << '\n';
    //     exit(1);
    // }
    // file_static_refData__stop_times.close();
    #if useStopTimesFile
    auto refrTree = func_loadFile__stop_times(path_static_historical_data+"/stop_times.csv", staticRefData__stop_times, trip_id__found);
    #endif


    std::list<std::string> filesToSearch;
    size_t count_searchedDirs = 0;
    func_depthSearch(path_dirToSearch, &filesToSearch, search_depth, &count_searchedDirs);

    std::cout << std::endl;
    Useful::PrintOut(std::string("Num [depth level]        : ")+std::to_string(search_depth),dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    Useful::PrintOut(std::string("Num [searched sub-dir's] : ")+std::to_string(count_searchedDirs),dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    Useful::PrintOut(std::string("Num [found valid file entries]: ")+std::to_string(filesToSearch.size()),dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    if(filesToSearch.size()==0) {
        Useful::PrintOut("closing program..");
        return 0;
    }
    
    
    terminalCursorPos.y+=3;
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, "processing entries:");
    terminalCursorPos.y++;

    std::string progressBar = "";
    size_t numTotalTripsRead = 0;
    size_t entryPathOpenFailures = 0;
    auto entryPathStr_itr = filesToSearch.begin();
    
    std::vector<TrpUpd> storedData;
    std::vector<STU_refd> storedData_tripDelays_idx;

    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y, "", true);


    std::vector<parseException_DebugString> vecExceptions_DebugString;

    terminalCursorPos.y+=2;
    for(size_t i=0; i<filesToSearch.size(); i++) {
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
                            // for(size_t i_refVec_stop_times=0; i_refVec_stop_times<staticRefData__stop_times.size(); i_refVec_stop_times++) {
                            //     Useful::ANSI_mvprint(terminalCursorPos.x+2+_tempString__readingEntry.size()+49, terminalCursorPos.y, Useful::formatNumber(i_refVec_stop_times,10,1,"right",false,true), false);
                            //     auto _vecRefLine = staticRefData__stop_times.at(i_refVec_stop_times);
                            //     if(_vecRefLine.size()>=5 && tempTrp.trip.trip_id==_vecRefLine.at(0) && (_vecRefLine.at(4).size()>0 && std::stoi(_vecRefLine.at(4))==tempSTU.stop_sequence)) {
                            //         tempSTU.stop_id = _vecRefLine.at(3);
                            //         _refrMatchFound = true;
                            //         break;
                            //     }
                            // }
                            for(auto i_stopseq : refrTree.vec) {
                                if(tempSTU.stop_sequence==i_stopseq.stop_sequence) {
                                    for(auto i_tripid : i_stopseq.stop_seq_vec) {
                                        if(tempTrp.trip.trip_id==i_tripid.trip_id) {
                                            tempSTU.stop_id = i_tripid.trip_id_vec.begin(); /// !!! NOTE: NEED TO BE CHANGED TO NOTE IMPLEMENT THE VECTOR.
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

        progressBar = Useful::progressBar(i, filesToSearch.size());
        Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+5, "", false);
        fmt::print(progressBar);
        std::advance(entryPathStr_itr, 1);
        entryFile.close();
        trpUpdate.Clear();
    }
    Useful::PrintOut("finished processing every entry.",dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    std::ofstream file__vecExceptions_DebugString("dataFile__vecExceptions_DebugString.csv");
    if(file__vecExceptions_DebugString.is_open()) {
        for(auto el : vecExceptions_DebugString) {
            file__vecExceptions_DebugString << "\""<< el.what << "\"," << el.where << "\n";
        }
        file__vecExceptions_DebugString.close();
    }

    progressBar = Useful::progressBar(filesToSearch.size(), filesToSearch.size(), true, true);
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+=5, "", false);
    fmt::print(progressBar);
    
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+=5, "");
    Useful::PrintOut("finished processing every entry.",dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    Useful::PrintOut("Num [failures] :"+std::to_string(entryPathOpenFailures),dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    
    std::cout << std::endl;
    std::fstream file_foundDelays(std::string("dataFile_foundDelays_")+std::to_string(storedData_tripDelays_idx.size())+".csv", std::ios::out);
    file_foundDelays << "filename_epoch,trip_id,STU_idx,stop_sequence,stop_id,delay_arrival,delay_departure\n";
    for(size_t i=0; i<storedData_tripDelays_idx.size(); i++) {
        auto refd = storedData_tripDelays_idx.at(i);
        file_foundDelays << refd.filename_epoch << "," << refd.trip_id << "," << refd.STU_idx << "," << refd.stop_sequence << "," << refd.stop_id << "," << refd.arrival.delay << "," << refd.departure.delay << "\n";
    }
    file_foundDelays.close();
    Useful::PrintOut(std::string("Num delays found: ")+std::to_string(storedData_tripDelays_idx.size())+" out of "+std::to_string(numTotalTripsRead)+" total trips.",dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);

    Useful::PrintOut("\n\nFinished program. Closing.",dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}


int64_t parse_epochTime_fromFilename(std::string _toParse) {

    int64_t epoch_time = 0;
    struct tm tm_time{0};
    size_t pos_delim = 0, pos_delim_new = 0;

    // example: "sl-tripupdates-2025-01-22T00-58-54Z.pb"
    // "2025-01-22T00-58-54Z.pb"

    std::string tempStr = "";
    _toParse.erase(0, 15);
    pos_delim_new = _toParse.find('-');
    tm_time.tm_year = std::stoi(_toParse.substr(0, pos_delim_new))-1900;
    pos_delim = pos_delim_new;

    pos_delim_new = _toParse.find('-', pos_delim+1);
    tm_time.tm_mon  = std::stoi(_toParse.substr(pos_delim+1, pos_delim_new)) -1;
    pos_delim = pos_delim_new;

    pos_delim_new = _toParse.find('T', pos_delim+1);
    tm_time.tm_mday = std::stoi(_toParse.substr(pos_delim+1, pos_delim_new));
    pos_delim = pos_delim_new;
    
    pos_delim_new = _toParse.find('-', pos_delim+1);
    tm_time.tm_hour = std::stoi(_toParse.substr(pos_delim+1, pos_delim_new));
    pos_delim = pos_delim_new;

    pos_delim_new = _toParse.find('-', pos_delim+1);
    tm_time.tm_min  = std::stoi(_toParse.substr(pos_delim+1, pos_delim_new));
    pos_delim = pos_delim_new;
    
    pos_delim_new = _toParse.find('Z', pos_delim+1);
    tm_time.tm_sec  = std::stoi(_toParse.substr(pos_delim+1, pos_delim_new));
    
    return (epoch_time = mktime(&tm_time));
}


