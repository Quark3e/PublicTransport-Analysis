
#include <includes.hpp>

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


struct STU_refd : public STU {
    std::string trip_id;
    uint32_t STU_idx;
    int64_t filename_epoch;

    STU_refd(STU _stu, std::string _trip_id, uint32_t _stu_idx, int64_t _file_epoch): STU{_stu}, trip_id{_trip_id}, STU_idx{_stu_idx}, filename_epoch{_file_epoch} {}
};


int64_t parse_epochTime_fromFilename(std::string _toParse);

int main(int argc, char** argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    Pos2d<int> dim_terminal = Useful::getTerminalSize();
    Useful::clearScreen();
    Useful::PrintOut("Program start.", dim_terminal.x);

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

    Useful::PrintOut("loading static_refData__trips vector with trip values which's route_id matched..", std::string::npos, "left", "");
    std::fstream static_refData__trips(path_static_historical_data+"/trips.csv", std::ios::in);
    for(std::string _line; std::getline(static_refData__trips, _line, '\n');) {
        bool matched = false;
        for(std::string route : route_id__toSearch) {
            if(route==_line.substr(0, 16)) {
                trip_id__found.push_back(_line.substr(_line.find(',', 17)+1, 17));
                break;
            }
        }
    }
    std::cout << " found " << trip_id__found.size() << " num trips.\n";
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

    std::list<std::string> filesToSearch;
    size_t count_searchedDirs = 0;
    func_depthSearch(path_dirToSearch, &filesToSearch, search_depth, &count_searchedDirs);

    std::cout << std::endl;
    Useful::PrintOut(std::string("Num [depth level]        : ")+std::to_string(search_depth));
    Useful::PrintOut(std::string("Num [searched sub-dir's] : ")+std::to_string(count_searchedDirs));
    Useful::PrintOut(std::string("Num [found valid file entries]: ")+std::to_string(filesToSearch.size()));
    if(filesToSearch.size()==0) {
        Useful::PrintOut("closing program..");
        return 0;
    }
    
    
    std::cout << std::endl;
    
    Useful::ANSI_mvprint(0, 7, "processing entries:");

    std::string progressBar = "";
    size_t numTotalTripsRead = 0;
    size_t entryPathOpenFailures = 0;
    auto entryPathStr_itr = filesToSearch.begin();
    
    std::vector<TrpUpd> storedData;
    std::vector<STU_refd> storedData_tripDelays_idx;

    Useful::ANSI_mvprint(0, 8, "", true);
    try {
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
                std::cout << "Failed to parse filename: "<< e.what() << '\n';
            }
            
            Useful::ANSI_mvprint(0, 9, std::string("reading entry #")+Useful::formatNumber(i, 3)+": \""+entry_filename+"\"; S.T.U. size:"+Useful::formatNumber(trpUpdate.stop_time_update_size())+"  ");
            for(size_t i_upd=0; i_upd<trpUpdate.stop_time_update_size(); i_upd++) {
                storedData.push_back(ParseDebugString(trpUpdate.stop_time_update(i_upd).DebugString()));
                auto tempTrp = storedData.back();
                bool tripToSearch = false;
                numTotalTripsRead+=1;
                for(std::string itr_trip : trip_id__found) {
                    if(tempTrp.trip.trip_id==itr_trip) {
                        tripToSearch = true;
                        break;
                    }
                }
                // if(!tripToSearch) continue;
                for(size_t i_stu=0; i_stu<tempTrp.stop_time_updates.size(); i_stu++) {
                    auto tempSTU = tempTrp.stop_time_updates.at(i_stu);
                    if(!(tempSTU.arrival.delay==0 && tempSTU.departure.delay==0)) {
                        storedData_tripDelays_idx.push_back(STU_refd{tempSTU, tempTrp.trip.trip_id, static_cast<uint32_t>(i_stu), filename_epoch});
                    }
                }
                // Useful::ANSI_mvprint(0, 8, Useful::formatNumber(i_upd));
            }

            //---------- process end   ----------
            progressBar = Useful::progressBar(i, filesToSearch.size());
            Useful::ANSI_mvprint(0, 10, "", false);
            fmt::print(progressBar);
            std::advance(entryPathStr_itr, 1);
            entryFile.close();
            
        }
    }
    catch(const std::exception& e) {
        std::cerr << "\n\n\n" << e.what() << '\n';
        return 1;
    }
    
    progressBar = Useful::progressBar(filesToSearch.size(), filesToSearch.size(), true, true);
    Useful::ANSI_mvprint(0, 10, "", false);
    fmt::print(progressBar);
    
    Useful::ANSI_mvprint(0, 15, "");
    Useful::PrintOut("finished processing every entry.");
    Useful::PrintOut("Num [failures] :"+std::to_string(entryPathOpenFailures));
    
    std::cout << std::endl;
    std::fstream file_foundDelays(std::string("dataFile_foundDelays_")+std::to_string(storedData_tripDelays_idx.size())+".csv", std::ios::out);
    file_foundDelays << "filename_epoch,trip_id,STU_idx,stop_sequence,stop_id,delay_arrival,delay_departure\n";
    for(size_t i=0; i<storedData_tripDelays_idx.size(); i++) {
        auto refd = storedData_tripDelays_idx.at(i);
        file_foundDelays << refd.filename_epoch << "," << refd.trip_id << "," << refd.STU_idx << "," << refd.stop_sequence << "," << refd.stop_id << "," << refd.arrival.delay << "," << refd.departure.delay << "\n";
    }
    file_foundDelays.close();
    Useful::PrintOut(std::string("Num delays found: ")+std::to_string(storedData_tripDelays_idx.size())+" out of "+std::to_string(numTotalTripsRead)+" total trips.");
    // system("pause");
    // for(size_t i=0; i<storedData_tripDelays_idx.size(); i++) {
    //     auto refd = storedData_tripDelays_idx.at(i);
    //     std::cout << " - filename_epoch: "<< refd.filename_epoch <<" | [trip_id: \"" << refd.trip_id << "\" | STU_idx: " << Useful::formatNumber(refd.STU_idx, 3) << " | stop_sequence: " << Useful::formatNumber(refd.stop_sequence, 3) << " | stop_id: " << refd.stop_id << "] : delays: ";
    //     if(refd.arrival.delay!=0)   std::cout << " arr.:" << Useful::formatNumber(refd.arrival.delay, 6) << "   ";
    //     if(refd.departure.delay!=0) std::cout << " dep.:" << Useful::formatNumber(refd.departure.delay, 6);
    //     std::cout << std::endl;
    // }
    
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
