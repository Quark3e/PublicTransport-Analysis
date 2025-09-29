
#include <PTDA_Data_Gatherer.hpp>
#include <gtfs-realtime.pb.h>

/**
 * 
 * 
 */
void load_stopInfo_map(
    std::string dirPath_staticHistorical,
    std::unordered_map<std::string, stopInfo>& ref_stopInfoMapToFill,   // <stop_id, {}>
    std::unordered_map<std::string, std::unordered_map<uint32_t, std::string>>& ref_reverseLookup_stopInfo, // <trip_id, <stop_sequence, stop_id>>
    size_t numMultithreadCount = 1
) {
    if(numMultithreadCount==0) throw std::runtime_error("ERROR: numMultithreadCount==0.");

    if(!(dirPath_staticHistorical.back()=='/' || dirPath_staticHistorical.back()=='\\')) dirPath_staticHistorical+='/';
    std::ifstream fileRead_stops(dirPath_staticHistorical+"stops.csv");
    std::ifstream fileRead_stop_times(dirPath_staticHistorical+"stop_times.csv");
    if(!fileRead_stops.is_open()) throw std::runtime_error(std::string("Failed to open stops.csv from path \"")+dirPath_staticHistorical+"stops.csv\".");
    if(!fileRead_stop_times.is_open()) throw std::runtime_error(std::string("Failed to open stop_times.csv from path \"")+dirPath_staticHistorical+"stop_times.csv\".");

    ref_stopInfoMapToFill.clear();
    fileRead_stops.ignore(1);
    for(std::string _line; std::getline(fileRead_stops, _line); ) {
        size_t pos_temp = 0;
        size_t pos_delim = _line.find(',');
        std::string str__stop_id = _line.substr(pos_delim);
        if(str__stop_id.size()<16) continue;

        std::string str__stop_name = _line.substr(pos_delim++, (pos_temp = _line.find(',', pos_delim))-pos_delim); pos_delim = pos_temp;
        Pos2d<float> cont__map_coord;
        cont__map_coord.x = std::stof(_line.substr(pos_delim++, (pos_temp = _line.find(',', pos_delim))-pos_delim)); pos_delim = pos_temp;
        cont__map_coord.y = std::stof(_line.substr(pos_delim++, (pos_temp = _line.find(',', pos_delim))-pos_delim)); pos_delim = pos_temp;
        
        ref_stopInfoMapToFill[str__stop_id] = {str__stop_id, str__stop_name, cont__map_coord};
    }
    fileRead_stops.close();

    fileRead_stop_times.ignore(1);
    for(std::string _line; std::getline(fileRead_stop_times, _line); ) {
        size_t pos_temp = 0;
        size_t pos_delim = _line.find(',');
        std::string str__trip_id = _line.substr(pos_delim);
        if(str__trip_id.size()<16) continue;

        std::string str__arrival_time   = _line.substr(pos_delim++, (pos_temp = _line.find(',', pos_delim))-pos_delim); pos_delim = pos_temp;
        std::string str__departure_time = _line.substr(pos_delim++, (pos_temp = _line.find(',', pos_delim))-pos_delim); pos_delim = pos_temp;
        std::string str__stop_id        = _line.substr(pos_delim++, (pos_temp = _line.find(',', pos_delim))-pos_delim); pos_delim = pos_temp;
        std::string str__stop_sequence  = _line.substr(pos_delim++, (pos_temp = _line.find(',', pos_delim))-pos_delim); pos_delim = pos_temp;
        std::string str__stop_headsign  = _line.substr(pos_delim++, (pos_temp = _line.find(',', pos_delim))-pos_delim); pos_delim = pos_temp;
        
        ref_stopInfoMapToFill[str__stop_id].tripRelatives.push_back({
            str__trip_id,
            Useful::time_string_to_tm(str__arrival_time),
            Useful::time_string_to_tm(str__departure_time),
            std::stoul(str__stop_sequence),
            str__stop_headsign
        });
        ref_reverseLookup_stopInfo[str__trip_id][std::stoul(str__stop_sequence)] = str__stop_id;
    }
    fileRead_stop_times.close();

}

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

TrpUpd ParseDebugString(std::string _strToParse/*, ThrdPrf::ThreadPerf* _ptrThreadPerf=nullptr*/) {
    if(_strToParse.size()==0) throw std::runtime_error("Empty string");
    if(_strToParse.substr(0, 11) != "departure {") throw std::runtime_error("String doesn't contain initial signature substr \"departure {\"");

    // if(_ptrThreadPerf) _ptrThreadPerf->set_T_start("Variable Init");
    TrpUpd _result{0, {"", "", "", 0, "", 0}, std::vector<STU>{}, {""}};
    std::string _isol = "";
    size_t _colonPos = 0;
    size_t _refIdx = 0;
    _colonPos = _strToParse.find(':');    //first colon: 'schedule_time'
    if(_refIdx==std::string::npos) throw std::runtime_error("No colons found in string.");
    
    _isol = _strToParse.substr(_colonPos+1, (_refIdx=_strToParse.find('\n', _colonPos))-_colonPos-1); // isolate timestamp substr whilst also updating _refIdx to hold the newline char that follows the colon.
    _result.timestamp = std::stoi(_isol);
    // if(_ptrThreadPerf) _ptrThreadPerf->set_T_end("Variable Init").multiplier+=1;

    if(_strToParse[_refIdx+3]!='1') throw std::runtime_error("String doesn't contain identifier number of 'trip'. Raw string of line:\""+_strToParse.substr(_refIdx+1, _strToParse.find('\n', _refIdx+1)));
    _refIdx = _refIdx + 6;  //update idx to hold index to the newline char after opening curly braces for 'trip'
    
    // if(_ptrThreadPerf) _ptrThreadPerf->set_T_start("Parse TripDescriptor");
    /// Parse TripDescriptor values.
    do {
        _colonPos = _strToParse.find(':', _refIdx);
        int _colonsID = std::stoi(_strToParse.substr(_refIdx+1, _colonPos-_refIdx-1));
        switch (_colonsID) {
        case 1: // trip_id [string]
            _isol = _strToParse.substr(_colonPos+3, _strToParse.find('\"', _colonPos+3)-_colonPos-3);
            _result.trip.trip_id = _isol;
            break;
        case 2: // start_time [string]
            _isol = _strToParse.substr(_colonPos+3, _strToParse.find('\"', _colonPos+3)-_colonPos-3);
            _result.trip.start_time = _isol;
            break;
        case 3: // start_date [string]
            _isol = _strToParse.substr(_colonPos+3, _strToParse.find('\"', _colonPos+3)-_colonPos-3);
            _result.trip.start_date = _isol;
            break;
        case 4: // schedule_relationship [int32]
            _isol = _strToParse.substr(_colonPos+2, _strToParse.find('\n', _colonPos));
            _result.trip.schedule_relationship = static_cast<int32_t>(std::stoi(_isol));
            break;
        case 5: // route_id [string]
            _isol = _strToParse.substr(_colonPos+3, _strToParse.find('\"', _colonPos+3)-_colonPos-3);
            _result.trip.route_id = _isol;
            break;
        case 6: // direction_id [uint32]
            _isol = _strToParse.substr(_colonPos+2, _strToParse.find('\n', _colonPos));
            _result.trip.direction_id = static_cast<uint32_t>(std::stoul(_isol));
            break;
        default:
            break;
        }
        _refIdx = _strToParse.find('\n', _colonPos); //set _refIdx to index to the newline of same line as found colon
    } while (_strToParse.at(_refIdx+3)!='}'); //while the char at next line's 3rd index isn't the closing braces for TripDescriptor

    // if(_ptrThreadPerf) _ptrThreadPerf->set_T_end("Parse TripDescriptor").multiplier+=1;

    _refIdx = _strToParse.find('\n', _refIdx+3); //set _refIdx to the newline char of the closing braces
    
    // if(_ptrThreadPerf) _ptrThreadPerf->set_T_start("Parse StopTimeUpdates");
    /// Parse StopTimeUpdate's
    while(_strToParse.at(_refIdx+3)=='2') {
        _strToParse.erase(0, _refIdx+3+3); //erase everything up 'til the newline character following the stop_time_update's opening braces
        _refIdx = 0;
        /// parse an instance of stop_time_update


        _result.stop_time_updates.push_back({
            0,
            {0, 0},
            {0, 0},
            "",
            0
        });
        auto& stu_ref = _result.stop_time_updates.back();

        do {
            int _id = std::stoi(_strToParse.substr(_refIdx+1, 6));
            
            STE* stu_ste__tempPtr = nullptr;
            switch (_id) {
            case 1: // stop_sequence [uint32]
                _colonPos = _strToParse.find(':', _refIdx);
                _isol = _strToParse.substr(_colonPos+2, _strToParse.find('\"', _colonPos+2)-_colonPos-2);

                stu_ref.stop_sequence = static_cast<uint32_t>(std::stoul(_isol));
                _refIdx = _strToParse.find('\n', _colonPos);
                break;
            case 2: // arrival [StopTimeEvent]
                stu_ste__tempPtr = &stu_ref.arrival;
            case 3: // departure [StopTimeEvent]
                if(!stu_ste__tempPtr) stu_ste__tempPtr = &stu_ref.departure;
                
                _refIdx = _strToParse.find('\n', _refIdx+1); //set _refIdx to index to newline after opening curly braces of STE.
                if(_strToParse.at(_refIdx+5)=='}') throw std::runtime_error(std::string("stop_time_event at [")+std::to_string(_result.stop_time_updates.size())+"] has its closing braces immedately after opening.");
                
                /// parse STE values
                do {
                    size_t __colonPos = _strToParse.find(':', _refIdx);
                    int __id = std::stoi(_strToParse.substr(_refIdx+1, __colonPos-_refIdx-1));
                    switch (__id) {
                    case 1: // delay [int32]
                        _isol = _strToParse.substr(__colonPos+2, _strToParse.find('\n', __colonPos+1)-__colonPos-1);
                        (*stu_ste__tempPtr).delay = static_cast<int32_t>(std::stoull(_isol));
                        // std::cout << "\"" << _isol << "\" :: "<<(*stu_ste__tempPtr).delay << std::endl;
                        break;
                    case 2: // time [int64]
                        (*stu_ste__tempPtr).time = static_cast<int64_t>(std::stoll(_strToParse.substr(__colonPos+1, _strToParse.find('\n', __colonPos+1)-__colonPos-1)));
                        break;
                    default:
                        break;
                    }
                    _refIdx = _strToParse.find('\n', __colonPos);
                } while (_strToParse.at(_refIdx+5)!='}');

                _refIdx = _strToParse.find('\n', _refIdx+1); //set _refIdx to index to newline after closing curly braces of STE.
                break;
            case 4: { // stop_id [string]
                if(_strToParse.substr(_refIdx+1, _strToParse.find('\n', _refIdx+1)-_refIdx-1).find('{')!=std::string::npos) { //method to deal with stop_id weird data fuckery. NOTE: Temporary solution.
                    /// encountered weird invalid data. Have to skip over this stop_id and pass an empty value.
                    /// had to resort to a more detailed search because apparently the members of this incorrect type can hold its own members... fuck
                    _refIdx +=Useful::findSubstr("\n    }", _strToParse.substr(_refIdx))+6;
                    break;
                }
                size_t _quoteMarkPos = _strToParse.find('\"', _refIdx);
                _isol = _strToParse.substr(_quoteMarkPos+1, _strToParse.find('\"', _quoteMarkPos+1)-_quoteMarkPos-1);
                stu_ref.stop_id = _isol;
                _refIdx = _strToParse.find('\n', _quoteMarkPos); // locate and set _refIdx to following newline
                
                break;
            }
            case 5: // schedule_relationship [int32]
                _colonPos = _strToParse.find(':', _refIdx);
                stu_ref.schedule_relationship = static_cast<int32_t>(std::stoi(_strToParse.substr(_colonPos+2, _strToParse.find('\n', _colonPos)-_colonPos-2)));
                _refIdx = _strToParse.find('\n', _colonPos); // locate and set _refIdx to following newline
                break;
            default:
                break;
            }

        } while (_strToParse.at(_refIdx+3)!='}'); //while the char at next line's 3rd index isn't the closing braces for TripDescriptor
        
        /// end parsing of an instance of stop_time_update

        _refIdx = _strToParse.find('\n', _refIdx+1); //set _refIdx to index to the newline char for the closing braces for the stop_time_update '  }\n'
        if(_refIdx==std::string::npos) return _result;
    }

    // if(_ptrThreadPerf) _ptrThreadPerf->set_T_end("Parse StopTimeUpdates").multiplier+=1;

    _refIdx = _strToParse.find(':', _refIdx+1);

    if(_strToParse.at(_refIdx-1)=='1') {
        _isol = _strToParse.substr(_refIdx+3, _strToParse.find('\"', _refIdx+3)-_refIdx-3);
        _result.vehicle.id = _isol;
    }


    return _result;
}

struct threadJob_sectionIdx {
    size_t start;
    size_t end;
    size_t gap;
    size_t total;
};
void threadFunc_subThreadFunc_TripUpdateParser(
    size_t                      thread_idx,
    std::list<std::string>&     all_entry_filenames,
    threadJob_sectionIdx        idx_section,
    const std::vector<std::string>& stopIDsToFind,
    const std::unordered_map<std::string, stopInfo>&    stopInfo_map,
    const std::unordered_map<std::string, std::unordered_map<uint32_t, std::string>>&   stopInfo_reverseLookup,
    std::atomic<bool>&          ref_running,
    std::mutex&                 mtx_share_progress,
    PTDA_Coms&                  thread_coms,
    std::list<exc_DebugStruct>* retur_caughtExcepts,
    std::list<TrpUpd>*          retur_storedData_tripUpds,
    std::list<STU_refd>*        retur_storedData_tripDelays
) {
    std::unique_lock<std::mutex> u_lck_progress(mtx_share_progress, std::defer_lock);
    std::list<STU_refd> localStoredData_tripDelays;
    std::list<TrpUpd>   localStoredData_tripUpds;
    std::list<exc_DebugStruct>  localStoredData_caughtExcepts;

    ref_running = true;

    size_t strLen_TotalNumSTU = thread_coms.progress.total;
    size_t strLen_TotalNumFiles = idx_section.gap;
    size_t exceptCatch_idx = 0;
    size_t numRead_STU = 0;

    std::thread::id threadID = std::this_thread::get_id();
    size_t strLen_initSubstr = 0;

    try {
        auto itr_entryFileName = all_entry_filenames.begin();
        std::advance(itr_entryFileName, idx_section.start);

        for(size_t i=0; i<idx_section.gap; i++) { // go over ever file entry for given idx_section
            std::ifstream entryFile(*itr_entryFileName, std::ios::in | std::ios::binary);
            std::string entry_filename = itr_entryFileName->substr(Useful::findSubstr("sl-tripupdates-", *itr_entryFileName));
            int64_t filename_epoch = 0;
            
            if(!ref_running.load()) break;

            u_lck_progress.lock();
            thread_coms.message.clear();
            thread_coms.message << threadID << " | reading entry #" << (i>9? "" : " ") << i << " \"" << entry_filename << "\"(";
            thread_coms.message << Useful::formatNumber(i, strLen_TotalNumFiles,0,"right",false,true)<<"/";
            thread_coms.message << Useful::formatNumber(idx_section.gap, strLen_TotalNumFiles,0,"right",false,true)<<") | ";
            strLen_initSubstr = thread_coms.message.str().size();
            u_lck_progress.unlock();

            if(!entryFile.is_open()) {
                localStoredData_caughtExcepts.push_back(
                    {"entryFile.isOpen()==false", std::string("\"file:'")+entry_filename+"'\",\"i_upd:-1\",\"exceptCatch_processID:"+std::to_string(-1)+"\""}
                );
                u_lck_progress.lock();
                thread_coms.message << "ERROR: entryFile \""<<entry_filename<<"\" could not be opened.";
                u_lck_progress.unlock();
                std::advance(itr_entryFileName, 1);
                continue;
            }

            u_lck_progress.lock();
            thread_coms.message << "ParseFromIstream |";
            u_lck_progress.unlock();
            transit_realtime::TripUpdate trpUpdate;
            trpUpdate.ParseFromIstream(&entryFile);

            filename_epoch = parse_epochTime_fromFilename(entry_filename);

            size_t i_upd = 0;
            exceptCatch_idx = 0;
            
            u_lck_progress.lock();
            thread_coms.message << " STU for-loop |";
            size_t strLen_initSubstr2 = thread_coms.message.str().size();
            u_lck_progress.unlock();
            size_t strLen_NumSTU = trpUpdate.stop_time_update_size();

            if(!ref_running.load()) break;
            try {
                for(; i_upd<trpUpdate.stop_time_update_size(); i_upd++) { //iterate over trips from TripUpdate
                    u_lck_progress.lock();
                    thread_coms.message.str(thread_coms.message.str().substr(0, strLen_initSubstr2));
                    thread_coms.message << " i_upd:"<<Useful::formatNumber(i_upd,strLen_NumSTU,0,"right",false,true)<<" |";
                    u_lck_progress.unlock();

                    std::string __tempToParse = trpUpdate.stop_time_update(i_upd).DebugString();
                    TrpUpd parsedTrpUpd = ParseDebugString(__tempToParse);
                    localStoredData_tripUpds.push_back(parsedTrpUpd);

                    u_lck_progress.lock();
                    thread_coms.message << " STU.size():" << parsedTrpUpd.stop_time_updates.size() << " |";
                    u_lck_progress.unlock();
                    exceptCatch_idx = 1;
                    for(size_t i_stu=0; i_stu<parsedTrpUpd.stop_time_updates.size(); i_stu++) { //iterate over parsed STU
                        STU& refSTU = parsedTrpUpd.stop_time_updates.at(i_stu);
                        if(parsedTrpUpd.trip.trip_id.size()==0 || refSTU.stop_sequence==0 || (refSTU.arrival.delay==0 && refSTU.departure.delay==0)) continue;
                        
                        exceptCatch_idx = 11;
                        std::string _stopID = refSTU.stop_id;
                        if(_stopID=="") _stopID = stopInfo_reverseLookup.at(parsedTrpUpd.trip.trip_id).at(parsedTrpUpd.stop_time_updates.at(i_stu).stop_sequence);
                        
                        exceptCatch_idx = 12;
                        localStoredData_tripDelays.push_back(STU_refd{refSTU, parsedTrpUpd.trip.trip_id, static_cast<uint32_t>(i_stu), filename_epoch});
                    }

                }
                numRead_STU+=trpUpdate.stop_time_update_size();
                u_lck_progress.lock();
                thread_coms.progress.update(numRead_STU);
                u_lck_progress.unlock();
            }
            catch(const std::exception& e) {
                localStoredData_caughtExcepts.push_back({e.what(),std::string("\"file:'")+entry_filename+"'\",\"i_upd:"+std::to_string(i_upd)+"\",\"exceptCatch_idx:"+std::to_string(exceptCatch_idx)+"\""});
                u_lck_progress.lock();
                thread_coms.message << " EXCEPTION:" << "e.what()[layer:1]:\"" << e.what() << "\": exceptCatch_idx:"<<exceptCatch_idx<<" |";
                u_lck_progress.unlock();
            }
            
            std::advance(itr_entryFileName, 1);
            entryFile.close();
            trpUpdate.Clear();
        }
    }
    catch(const std::exception& e) {
        u_lck_progress.lock();
        thread_coms.message << " e.what()[layer:0]:\"" << e.what() << "\": exceptCatch_idx:"<<exceptCatch_idx<<" |";
        u_lck_progress.unlock();
    }
    
    retur_caughtExcepts->swap(localStoredData_caughtExcepts);
    retur_storedData_tripDelays->swap(localStoredData_tripDelays);
    retur_storedData_tripUpds->swap(localStoredData_tripUpds);

    ref_running = false;
}


class bit7z_callbackClass {
private:
    uint64_t    total;
    uint64_t    progress;
    double      speed;
    std::chrono::steady_clock::time_point time_0;
    std::chrono::duration<double> ETA_sec;

    DGNC::DataGatherer& DG_refObj;
public:
    bit7z_callbackClass(DGNC::DataGatherer& _DG_refObj):
        DG_refObj(_DG_refObj), total(0), progress(0), speed(0), time_0(std::chrono::steady_clock::now()), ETA_sec(std::chrono::duration<double>(0))
    {

    }
    bit7z_callbackClass(const bit7z_callbackClass& _other):
        DG_refObj(_other.DG_refObj), total(_other.total), progress(_other.progress), speed(_other.speed), time_0(_other.time_0), ETA_sec(_other.ETA_sec)
    {

    }
    bit7z_callbackClass(bit7z_callbackClass&& _other):
        DG_refObj(_other.DG_refObj), total(_other.total), progress(_other.progress), speed(_other.speed), time_0(_other.time_0), ETA_sec(_other.ETA_sec)
    {

    }
    ~bit7z_callbackClass() {}
    bit7z_callbackClass& operator=(const bit7z_callbackClass& _other) {
        this->total     = _other.total;
        this->progress  = _other.progress;
        this->speed     = _other.speed;
        this->time_0    = _other.time_0;
        this->ETA_sec   = _other.ETA_sec;

    }
    bit7z_callbackClass& operator=(bit7z_callbackClass&& _other) {
        this->total     = _other.total;
        this->progress  = _other.progress;
        this->speed     = _other.speed;
        this->time_0    = _other.time_0;
        this->ETA_sec   = _other.ETA_sec;

    }

    void setExtractorCallbacks(bit7z::BitFileExtractor& bit7z_ref) {
        
        bit7z_ref.setTotalCallback([this](uint64_t var) {callback_total(var);});
        bit7z_ref.setProgressCallback([this](uint64_t var) {callback_progress(var); return true;});
    }

    void callback_total(uint64_t _total_size) {
        this->total = _total_size;
    }
    bool callback_progress(uint64_t _processed_size) {
        auto time_1 = std::chrono::steady_clock::now();
        auto delta = _processed_size - this->progress;
        std::chrono::duration<double> interval = time_1 - this->time_0;
        if(interval.count()==0) return true;

        this->speed = delta/interval.count();
        this->ETA_sec = std::chrono::duration<double>((this->total - _processed_size) / this->speed);

        std::unique_lock<std::mutex> u_lck(DG_refObj.mtx_access__progressInfo, std::defer_lock);
        
        size_t strLen_total = Useful::formatNumber(this->total, 0, 0, "right", false, true).size();
        u_lck.lock();
        std::string tempStr = DG_refObj.progressInfo.message.str();
        size_t dumpIdx = std::string::npos;
        for(size_t i=tempStr.size()-2; i>=0; i--) {
            if(tempStr.at(i)=='\n') {
                dumpIdx = i;
                break;
            }
        }

        DG_refObj.progressInfo.progress.update(_processed_size, this->total);
        // progressInfo.message.clear();
        DG_refObj.progressInfo.message.str(tempStr.substr(0, dumpIdx));

        DG_refObj.progressInfo.message << std::this_thread::get_id() << " | [bit7z] Decompression progress: " << Useful::formatNumber(DG_refObj.progressInfo.progress.percent,5,1) << "% ";
        DG_refObj.progressInfo.message << "(" << Useful::formatNumber(DG_refObj.progressInfo.progress.now, strLen_total,0,"right",false,true) << "/" << (DG_refObj.progressInfo.progress.total, strLen_total,0,"right",false,true) <<" bytes)\n";
    
        u_lck.unlock();

        this->time_0 = time_1;
        this->progress = _processed_size;
        return true;
    }
};


int DGNC::DataGatherer::threadFunc__callbackDownloadProgress(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    if(dltotal > 0) {
        size_t strLen_total = Useful::formatNumber(dltotal, 0, 0, "right", false, true).size();
        
        std::string tempStr = progressInfo.message.str();
        size_t dumpIdx = std::string::npos;
        for(size_t i=tempStr.size()-2; i>=0; i--) {
            if(tempStr.at(i)=='\n') {
                dumpIdx = i;
                break;
            }
        }

        std::unique_lock<std::mutex> u_lck_progress(mtx_access__progressInfo);
        progressInfo.progress.update(dlnow, dltotal);
        // progressInfo.message.clear();
        progressInfo.message.str(tempStr.substr(0, dumpIdx));

        progressInfo.message << std::this_thread::get_id() << " | [curl] Download progress: " << Useful::formatNumber(progressInfo.progress.percent,5,1) << "% ";
        progressInfo.message << "(" << Useful::formatNumber(progressInfo.progress.now,strLen_total,0,"right",false,true) << "/" << (progressInfo.progress.total,strLen_total,0,"right",false,true) <<" bytes)\n";
    }

    return 0;
}

void DGNC::threadFunc(DGNC::DataGatherer& DG_ref) {
    std::unique_lock<std::mutex> u_lck_accss__callback_errors(DG_ref.mtx_access__callback_errors, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__callback_updates(DG_ref.mtx_access__callback_updates, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__callback_finished(DG_ref.mtx_access__callback_finished, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__path_dirDelayFileOut(DG_ref.mtx_access__path_dirDelayFileOut, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__path_dirTempCompressed(DG_ref.mtx_access__path_dirTempCompressed, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__progressInfo(DG_ref.mtx_access__progressInfo, std::defer_lock);
    // std::unique_lock<std::mutex> u_lck_accss__TaskRequest(DG_ref.mtx_access__TaskRequest, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__url_components(DG_ref.mtx_access__url_components, std::defer_lock);

    std::thread::id threadID = std::this_thread::get_id();
    

    DG_ref.threadRunning = true;
    DG_ref.status = noTask;
    while(DG_ref.threadRunning) {
        if(!DG_ref.newTaskRequested.load()) continue;
        DG_ref.status = running;

        DGNC::TaskRequest& RequestedTask = DG_ref.currentTaskRequest;
        for(size_t i=0; i<RequestedTask.dates.size(); i++) {
            
            tm temp_tm = RequestedTask.dates.at(i);
            std::string url = DG_ref.url_base;

            std::stringstream ss;
            ss << std::put_time(&temp_tm, "%y-%m-&d");
            u_lck_accss__url_components.lock();
            DG_ref.url_components["date"] = ss.str();
            std::string zipFilename     = DG_ref.url_components["operator"]+"-"+DG_ref.url_components["feed"]+"-"+DG_ref.url_components["date"]+".7z";
            u_lck_accss__path_dirTempCompressed.lock();
            std::string zipFilename_fullPath= DG_ref.path_dirTempCompressed+(DG_ref.path_dirTempCompressed.back()=='/'? "" : "/")+zipFilename;
            u_lck_accss__path_dirTempCompressed.unlock();

            u_lck_accss__progressInfo.lock();
            DG_ref.progressInfo.message.clear();
            DG_ref.progressInfo.message << threadID << " | Running Task for date " << DG_ref.url_components["date"] << "; filename: \""<<zipFilename<<"\"\n";
            u_lck_accss__progressInfo.unlock();

            /// ---------- Downloading compressed raw data files ----------

            for(auto itr_pair : DG_ref.url_components) {
                try {
                    size_t findPos = 0;
                    url.replace((findPos=Useful::findSubstr(std::string("{")+itr_pair.first+"}", url)), itr_pair.first.size()+2, itr_pair.second);
                }
                catch(const std::exception& e) {
                    u_lck_accss__progressInfo.lock();
                        DG_ref.progressInfo.message << threadID << " | EXCEPTION[\"" << e.what() << "\" from itr_pair for loop at {" << itr_pair.first << " : " << itr_pair.second << "}]\n";
                        u_lck_accss__callback_updates.lock();
                            if(DG_ref.callback_updates) DG_ref.callback_updates(DG_ref.progressInfo);
                        u_lck_accss__callback_updates.unlock();
                        u_lck_accss__callback_errors.lock();
                            if(DG_ref.callback_errors) DG_ref.callback_errors(DG_ref.progressInfo);
                        u_lck_accss__callback_errors.unlock();
                    u_lck_accss__progressInfo.unlock();
                }
            }
            u_lck_accss__url_components.unlock();

            try {
                CGd::HttpClient client;
                client.setTimeout(60);
                client.setSslVerify(true);
                client.progressCallback_func = DG_ref.threadFunc__callbackDownloadProgress;
                
                u_lck_accss__path_dirTempCompressed.lock();
                u_lck_accss__progressInfo.lock();
                DG_ref.progressInfo.message << threadID << " | GET request of url: \"" << url << "\"\n";
                u_lck_accss__progressInfo.unlock();
                if(client.downloadFile(url, zipFilename_fullPath)) {
                    if(std::filesystem::exists(DG_ref.path_dirTempCompressed)) {
                        DG_ref.progressInfo.message << CGd::verboseStream.rdbuf();
                        auto fileSize = std::filesystem::file_size(DG_ref.path_dirTempCompressed);
                        u_lck_accss__progressInfo.lock();
                        DG_ref.progressInfo.message << threadID << " | " << "Download completed successfully.\n";
                        DG_ref.progressInfo.message << threadID << " | " << "Size: " << Useful::HumanReadable{fileSize} << "\n";

                        u_lck_accss__progressInfo.unlock();
                    }
                }
                else {
                    u_lck_accss__progressInfo.lock();
                    DG_ref.progressInfo.message << threadID << " | " << "download failed. getLastError(): " << client.getLastError() << "\n";
                    u_lck_accss__callback_errors.lock();
                    if(DG_ref.callback_errors) DG_ref.callback_errors(DG_ref.progressInfo);
                    u_lck_accss__callback_errors.unlock();
                    u_lck_accss__progressInfo.unlock();

                    continue;
                }
                u_lck_accss__path_dirTempCompressed.unlock();
            }
            catch(const std::exception& e) {
                u_lck_accss__progressInfo.lock();
                DG_ref.progressInfo.message << threadID << " | EXCEPTION: " << e.what() << "\n";
                u_lck_accss__callback_errors.lock();
                if(DG_ref.callback_errors) DG_ref.callback_errors(DG_ref.progressInfo);
                u_lck_accss__callback_errors.unlock();
                u_lck_accss__progressInfo.unlock();

                continue;
            }

            if(!DG_ref.threadRunning.load()) break;
            /// ---------- Uncompressing downloaded raw data files ----------

            try {

                bit7z::Bit7zLibrary lib{"C:\\Program Files\\7-Zip\\7z.dll"};
                bit7z::BitFileExtractor extractor{lib, bit7z::BitFormat::SevenZip};

                bit7z_callbackClass callbackClass(DG_ref);

                callbackClass.setExtractorCallbacks(extractor);

                u_lck_accss__path_dirTempCompressed.lock();
                extractor.extract(zipFilename_fullPath, DG_ref.path_dirTempCompressed);
                u_lck_accss__path_dirTempCompressed.unlock();
            }
            catch(const bit7z::BitException& ex) {
                u_lck_accss__progressInfo.lock();
                DG_ref.progressInfo.message << threadID << " | BitException: " << ex.what() << "\n";
                u_lck_accss__callback_errors.lock();
                if(DG_ref.callback_errors) DG_ref.callback_errors(DG_ref.progressInfo);
                u_lck_accss__callback_errors.unlock();
                u_lck_accss__progressInfo.unlock();
                continue;
            }

            if(!DG_ref.threadRunning.load()) break;
            /// ---------- "Processing"/parsing the raw data files ----------

            std::list<std::string> dataFilesToProcess;
            size_t count_searchedDirs = 0;
            u_lck_accss__path_dirTempCompressed.lock();
            func_depthSearch(DG_ref.path_dirTempCompressed, &dataFilesToProcess, 10, &count_searchedDirs);
            u_lck_accss__path_dirTempCompressed.unlock();
            if(dataFilesToProcess.size()==0) {
                u_lck_accss__progressInfo.lock();
                DG_ref.progressInfo.message << threadID << " | ERROR: func_depthSearch() result: dataFilesToProcess.size()==0 \n";
                u_lck_accss__callback_errors.lock();
                if(DG_ref.callback_errors) DG_ref.callback_errors(DG_ref.progressInfo);
                u_lck_accss__callback_errors.unlock();
                u_lck_accss__progressInfo.unlock();
                continue;
            }   


            if(!DG_ref.threadRunning.load()) break;
            // ----- load every data file into TripUpdate and get the total STU_size for progress measurement 

            transit_realtime::TripUpdate tempTripUpdHolder;
            size_t num_failedIstreams = 0;
            size_t TotalNum_STU = 0;
            auto itr_entry = dataFilesToProcess.begin();
            for(size_t i=0; i<dataFilesToProcess.size(); i++) { // ----- get the total num of StopTimeUpdates
                std::ifstream entryFile(*itr_entry, std::ios::in | std::ios::binary);
                if(!entryFile.is_open()) {
                    num_failedIstreams++;
                    std::advance(itr_entry, 1);
                    continue;
                }
                tempTripUpdHolder.ParseFromIstream(&entryFile);
                TotalNum_STU += tempTripUpdHolder.stop_time_update_size();

                tempTripUpdHolder.Clear();
                entryFile.close();
                std::advance(itr_entry, 1);
            }

            auto itr_entriesToProcess = dataFilesToProcess.begin();
            std::vector<threadJob_sectionIdx> idx_sections;
            for(size_t _thr=0; _thr<RequestedTask.numParallelThreads; _thr++) {
                size_t _gap = std::ceil(float(dataFilesToProcess.size())/float(RequestedTask.numParallelThreads));
                idx_sections.push_back({
                    _thr*_gap,
                    _thr*_gap+_gap,
                    _gap,
                    dataFilesToProcess.size()
                });
            }
            
            std::list<std::list<exc_DebugStruct>>   refReturned_caughtExcepts(RequestedTask.numParallelThreads, std::list<exc_DebugStruct>());
            std::list<std::mutex>                   refReturned_mtxShareProgr(RequestedTask.numParallelThreads, std::mutex());
            std::list<std::atomic<bool>>            refReturned_running(RequestedTask.numParallelThreads,       std::atomic<bool>{true});
            std::list<PTDA_Coms>                    refReturned_thread_coms(RequestedTask.numParallelThreads,   PTDA_Coms());
            std::list<std::list<STU_refd>>          refReturned_tripDelays(RequestedTask.numParallelThreads,    std::list<STU_refd>());
            std::list<std::list<TrpUpd>>            refReturned_tripUpdates(RequestedTask.numParallelThreads,   std::list<TrpUpd>());
            auto itr_refRet__caughtExcepts  = refReturned_caughtExcepts.begin();
            auto itr_refRet__mtxShareProgr  = refReturned_mtxShareProgr.begin();
            auto itr_refRet__running        = refReturned_running.begin();
            auto itr_refRet__thread_coms    = refReturned_thread_coms.begin();
            auto itr_refRet__tripDelays     = refReturned_tripDelays.begin();
            auto itr_refRet__tripUpdates    = refReturned_tripUpdates.begin();

            u_lck_accss__progressInfo.lock();
            DG_ref.progressInfo.message << threadID << " | initialising sub-threads:\n";
            u_lck_accss__progressInfo.unlock();
            std::list<std::thread> threadObjects;
            for(size_t idx_thread=0; idx_thread<RequestedTask.numParallelThreads; idx_thread++) {
                itr_refRet__thread_coms->progress.total = TotalNum_STU;

                threadObjects.emplace_back(
                    [&, idx_thread, itr_refRet__caughtExcepts, itr_refRet__mtxShareProgr, itr_refRet__thread_coms, itr_refRet__tripDelays, itr_refRet__tripUpdates] {
                    threadFunc_subThreadFunc_TripUpdateParser(
                        idx_thread, dataFilesToProcess, idx_sections.at(idx_thread),
                        std::ref(RequestedTask.stopIDs),
                        std::ref(DG_ref.stopInfoMap_ref),
                        std::ref(DG_ref.stopInfoReverseLookup),
                        std::ref(*itr_refRet__running),
                        std::ref(*itr_refRet__mtxShareProgr),
                        std::ref(*itr_refRet__thread_coms),
                        &(*itr_refRet__caughtExcepts),
                        &(*itr_refRet__tripUpdates),
                        &(*itr_refRet__tripDelays)
                    );
                });

                if(idx_thread+1<RequestedTask.numParallelThreads) {
                    std::advance(itr_refRet__caughtExcepts, 1);
                    std::advance(itr_refRet__mtxShareProgr, 1);
                    std::advance(itr_refRet__running,       1);
                    std::advance(itr_refRet__thread_coms,   1);
                    std::advance(itr_refRet__tripDelays,    1);
                    std::advance(itr_refRet__tripUpdates,   1);
                }
            }
            if(threadObjects.size()==0) {
                u_lck_accss__progressInfo.lock();
                DG_ref.progressInfo.message << threadID << " | ERROR: threadObjects failed to get created. threadObjects.size()==0.\n";
                u_lck_accss__callback_errors.lock();
                if(DG_ref.callback_errors) DG_ref.callback_errors(DG_ref.progressInfo);
                u_lck_accss__callback_errors.unlock(); 
                u_lck_accss__progressInfo.unlock();
                continue;
            }
            
            std::vector<std::unique_lock<std::mutex>> subThread_sharedProgresses(RequestedTask.numParallelThreads);
            std::map<size_t, PTDA_Coms> subThread_progressCopy;
            for(size_t ii=0; ii<RequestedTask.numParallelThreads; ii++) {
                subThread_sharedProgresses.at(ii) = std::unique_lock<std::mutex>(*itr_refRet__mtxShareProgr, std::defer_lock);
                if(ii+1<RequestedTask.numParallelThreads) std::advance(itr_refRet__mtxShareProgr, 1);
                subThread_progressCopy[ii] = {};
            }
            
            
            auto itr_threadObj          = threadObjects.begin();
            // itr_refRet__caughtExcepts  = refReturned_caughtExcepts.begin();
            itr_refRet__mtxShareProgr   = refReturned_mtxShareProgr.begin();
            itr_refRet__running         = refReturned_running.begin();
            itr_refRet__thread_coms     = refReturned_thread_coms.begin();
            // itr_refRet__tripDelays     = refReturned_tripDelays.begin();
            // itr_refRet__tripUpdates    = refReturned_tripUpdates.begin();
            size_t Read_TotalNum_STU = 0;
            std::string stringSection_subThreadProgress = "";
            std::chrono::duration<double> ETA_maxDuration(0);

            u_lck_accss__progressInfo.lock();
            DG_ref.progressInfo.progress.total  = TotalNum_STU;
            DG_ref.progressInfo.progress.now    = 0;
            DG_ref.progressInfo.message << threadID << " | subThread progress: ";
            size_t strLen_upUntilProgressInfo = DG_ref.progressInfo.message.str().size();
            u_lck_accss__progressInfo.unlock();
            std::string str_TotalNumSTU = Useful::formatNumber(TotalNum_STU,0,0,"right",false,true);

            size_t id_thread = RequestedTask.numParallelThreads;
            bool hasActiveThreads = true;
            while(true) {
                id_thread++;
                if(id_thread+1<RequestedTask.numParallelThreads) {
                    std::advance(itr_threadObj, 1);
                    // std::advance(itr_refRet__caughtExcepts, 1);
                    std::advance(itr_refRet__mtxShareProgr, 1);
                    std::advance(itr_refRet__running,       1);
                    std::advance(itr_refRet__thread_coms,   1);
                    // std::advance(itr_refRet__tripDelays,    1);
                    // std::advance(itr_refRet__tripUpdates,   1);
                }
                else {
                    itr_threadObj   = threadObjects.begin();
                    // itr_refRet__caughtExcepts  = refReturned_caughtExcepts.begin();
                    itr_refRet__mtxShareProgr  = refReturned_mtxShareProgr.begin();
                    itr_refRet__running        = refReturned_running.begin();
                    itr_refRet__thread_coms    = refReturned_thread_coms.begin();
                    // itr_refRet__tripDelays     = refReturned_tripDelays.begin();
                    // itr_refRet__tripUpdates    = refReturned_tripUpdates.begin();
                }

                if(id_thread>=RequestedTask.numParallelThreads) {
                    if(!DG_ref.threadRunning.load()) {
                        (*itr_refRet__running) = false;
                        continue;
                    }
                    u_lck_accss__progressInfo.lock();
                    DG_ref.progressInfo.progress.update(Read_TotalNum_STU);
                    DG_ref.progressInfo.message.str(DG_ref.progressInfo.message.str().substr(0, strLen_upUntilProgressInfo));
                    DG_ref.progressInfo.message << "STU:("<<Useful::formatNumber(Read_TotalNum_STU,str_TotalNumSTU.size(),0,"right",false,true)<<"/"<<str_TotalNumSTU<<") ";
                    DG_ref.progressInfo.message << "ETA: " << Useful::formatDuration(ETA_maxDuration) << "\n";
                    DG_ref.progressInfo.message << stringSection_subThreadProgress;
                    u_lck_accss__progressInfo.unlock();

                    if(!hasActiveThreads) break;

                    Read_TotalNum_STU = 0;
                    stringSection_subThreadProgress = "";
                    ETA_maxDuration = std::chrono::duration<double>(0);
                    id_thread = 0;
                    hasActiveThreads = false;
                }

                stringSection_subThreadProgress += "status: ";
                if(!itr_refRet__running->load()) {
                    stringSection_subThreadProgress += "closed\n";
                    continue;
                }
                else if(!(itr_threadObj->joinable())) {
                    stringSection_subThreadProgress += "terminated!\n";
                    continue;
                }
                else stringSection_subThreadProgress += "open  | ";
                hasActiveThreads = true;

                subThread_sharedProgresses.at(id_thread).lock();
                subThread_progressCopy[id_thread] = *itr_refRet__thread_coms;
                subThread_sharedProgresses.at(id_thread).unlock();
                
                stringSection_subThreadProgress += "progr:" + Useful::formatNumber(subThread_progressCopy[id_thread].progress.percent,5,2,"right",false,true)+"% ";
                stringSection_subThreadProgress += "speed:" + Useful::formatNumber(subThread_progressCopy[id_thread].progress.speed,  7,2,"right",false,true)+" ";

                stringSection_subThreadProgress += subThread_progressCopy[id_thread].message.str() + "\n";
                
                Read_TotalNum_STU += subThread_progressCopy[id_thread].progress.now;
                if(subThread_progressCopy[id_thread].progress.ETA_sec>ETA_maxDuration) ETA_maxDuration = subThread_progressCopy[id_thread].progress.ETA_sec;

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if(!DG_ref.threadRunning.load()) break;

            for(auto& threadRef : threadObjects) {
                if(threadRef.joinable()) threadRef.join();
            }
            
            size_t TotalNumDelays = 0;
            for(auto thread_delayList : refReturned_tripDelays) TotalNumDelays += thread_delayList.size();
            if(TotalNumDelays==0) {
                u_lck_accss__progressInfo.lock();
                DG_ref.progressInfo.message << threadID << " | ERROR: TotalNumDelays=="<<TotalNumDelays<<"\n";
                u_lck_accss__callback_errors.lock();
                DG_ref.callback_errors(DG_ref.progressInfo);
                u_lck_accss__callback_errors.unlock();
                u_lck_accss__progressInfo.unlock();
            }

            u_lck_accss__progressInfo.lock();
            DG_ref.progressInfo.message << threadID << " | finished parsing data. Sub-threads joined. Total num delays:"<<TotalNumDelays<<".\n";
            u_lck_accss__progressInfo.unlock();
            
            DG_ref.data_delays.clear();
            DG_ref.data_delays[DG_ref.url_components["date"]] = std::vector<STU_refd>(TotalNumDelays);
            
            bool memLow_leave = false;
            size_t delay_idx = 0;
            for(auto thread_tripDelay : refReturned_tripDelays) {
                for(auto tripDelay : thread_tripDelay) {
                    DG_ref.data_delays[DG_ref.url_components["date"]].at(delay_idx) = tripDelay;
                    delay_idx++;

                    if(Useful::getTotalAvailMemory() < 10*std::pow(10,6)) { // if available memory is less than 10MB
                        memLow_leave = true;
                        for(size_t _=0; _<3; _++) {
                            if(Useful::getTotalAvailMemory() > 10*std::pow(10,6)) {
                                memLow_leave = false;
                                break;
                            }
                            
                        }
                        if(memLow_leave) {
                            DG_ref.data_delays.erase(DG_ref.url_components["date"]);
                            break;
                        }
                    }
                
                }
                if(memLow_leave) break;
            }
            if(memLow_leave) {
                u_lck_accss__progressInfo.lock();
                DG_ref.progressInfo.message << threadID << " | ERROR: NO MEMORY(memory left < 10MB). Skipping current date.\n";
                u_lck_accss__callback_errors.lock();
                DG_ref.callback_errors(DG_ref.progressInfo);
                u_lck_accss__callback_errors.unlock();
                u_lck_accss__progressInfo.unlock();
                continue;
            }

        }

        DG_ref.newTaskRequested = false;
        DG_ref.status = finished;
        
        u_lck_accss__callback_finished.lock();
        if(DG_ref.isDefined__callback_finished) DG_ref.callback_finished(DG_ref.data_delays);
        u_lck_accss__callback_finished.unlock();
    }
    
    DG_ref.threadRunning = false;

}
void DGNC::threadFunc_Task__GET_Request() {
    
}
void DGNC::threadFunc_Task__ExtractFiles() {

}
void DGNC::threadFunc_Task__ParseDelays() {

}


DGNC::DataGatherer::DataGatherer(
    std::string _api_key,
    std::unordered_map<std::string, stopInfo>& _refStopInfoMap,
    std::unordered_map<std::string, std::unordered_map<uint32_t, std::string>>& _refStopInfoReverseLookupmap,
    bool _initialise,
    std::string _path_dirDelayFileOut,
    std::string _path_tempDirCompressed
): 
    stopInfoMap_ref(_refStopInfoMap), stopInfoReverseLookup(_refStopInfoReverseLookupmap), path_dirDelayFileOut(_path_dirDelayFileOut), path_dirTempCompressed(_path_tempDirCompressed)
{
    this->url_components["api_key"] = _api_key;
    if(_initialise) this->init();
}
DGNC::DataGatherer::~DataGatherer() {
    this->threadRunning = false;
    std::unique_lock<std::mutex> u_lck(this->mtx_access__progressInfo, std::defer_lock);
    u_lck.lock();
    progressInfo.message << std::this_thread::get_id() << " | ";
    progressInfo.message << "DGNC::DataGatherer::~DataGatherer(): threadRunning variable set to false: ";
    if(thread_dataGatherer.joinable()) {
        progressInfo.message << "waiting for thread to close...\n";
        u_lck.unlock();

        thread_dataGatherer.join();
    }
    else {
        progressInfo.message << "thread was not joinable.\n";
        u_lck.unlock();
    }

}

int DGNC::DataGatherer::init() {
    if(this->classInit) throw std::runtime_error("DGNC::DataGatherer::init() : class has already been initialised.");

    this->progressInfo.message << std::this_thread::get_id() << " | ";
    this->progressInfo.message << "init called.\n";

    this->thread_dataGatherer = std::thread(DGNC::threadFunc, *this);
    this->classInit = true;
}

int DGNC::DataGatherer::setApiKey(std::string _key) {
    std::unique_lock<std::mutex> u_lck(this->mtx_access__url_components);
    this->url_components["api_key"] = _key;
    
    return 0;
}

int DGNC::DataGatherer::setNewTask(DGNC::TaskRequest _newRequest) {
    if(this->status.load()==TaskRequest_Status::running) throw std::runtime_error("DGNC::DataGatherer::setNewTask(DGNC::TaskRequest): a task is currently running.");
    if(_newRequest.dates.size()==0) throw std::runtime_error("DGNC::DataGatherer::setNewTask(DGNC::TaskRequest): _newRequest.dates.size() cannot be 0");
    if(_newRequest.stopIDs.size()==0) throw std::runtime_error("DGNC::DataGatherer::setNewTask(DGNC::TaskRequest): _newRequest.stopIDs.size() cannot be 0");
    if(_newRequest.numParallelThreads==0) throw std::runtime_error("DGNC::DataGatherer::setNewTask(DGNC::TaskRequest): _newRequest.numParallelThreads cannot be 0");

    // std::unique_lock<std::mutex> u_lck_setTaskReq(this->mtx_access__TaskRequest);
    currentTaskRequest = _newRequest;
    this->newTaskRequested = true;
    
    return 0;
}

int DGNC::DataGatherer::setCallback_errors(funcType_updateCallback _callbackFunc) {
    std::unique_lock<std::mutex> lck(this->mtx_access__callback_errors);
    callback_errors = _callbackFunc;

    return 0;
}
int DGNC::DataGatherer::setCallback_updates(funcType_updateCallback _callbackFunc) {
    std::unique_lock<std::mutex> lck(this->mtx_access__callback_updates);
    callback_updates = _callbackFunc;

    return 0;
}

void DGNC::DataGatherer::setPath_dirDelayFileOut(std::string _newPath) {
    std::unique_lock<std::mutex> u_lck(mtx_access__path_dirDelayFileOut);
    this->path_dirDelayFileOut = _newPath;
}
void DGNC::DataGatherer::setPath_dirTempCompressed(std::string _newPath) {
    std::unique_lock<std::mutex> u_lck(mtx_access__path_dirTempCompressed);
    this->path_dirTempCompressed = _newPath;
}

DGNC::TaskRequest DGNC::DataGatherer::getCurrentRequest() {
    return this->currentTaskRequest;
}
PTDA_Coms DGNC::DataGatherer::getProgressInfo() {
    std::unique_lock<std::mutex> u_lck(this->mtx_access__progressInfo);
    return progressInfo;
}
DGNC::TaskRequest_Status DGNC::DataGatherer::getStatus() {
    return this->status.load();
}

std::map<std::string, std::vector<STU_refd>>& DGNC::DataGatherer::extractData(std::map<std::string, std::vector<STU_refd>>& _toSwapTo) {
    if(this->status.load()==TaskRequest_Status::running) throw std::runtime_error("DGNC::DataGatherer::extractData(std::map<std::string, std::vector<STU_refd>>&): a task is currently running.");

    _toSwapTo.swap(this->data_delays);
    return _toSwapTo;
}
