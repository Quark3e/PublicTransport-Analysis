
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

struct stopTimeEvent {
    int32_t delay;
    int64_t time;
    int32_t uncertainty;
};

int main(int argc, char** argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    
    int search_depth = 1;
    std::string dirToSearch;
    
    Pos2d<int> dim_terminal = Useful::getTerminalSize();

    Useful::clearScreen();
    Useful::PrintOut("Program start.", dim_terminal.x);

    if(argc <= 1) {
        std::cerr << "Invalid number of argc. No path to directory given." << std::endl;
        return 1;
    }

    dirToSearch = argv[1];
    if(!std::filesystem::is_directory(dirToSearch)) {
        std::cerr << "ERROR: the program argument for path given is not a valid path." << std::endl;
        return 1;
    }


    /**
     * 1. Search dir for entries and save every entry's path in a vector *IF* the path ends in a `.pb`
     * 2. 
     */

    std::list<std::string> filesToSearch;
    size_t count_searchedDirs = 0;
    func_depthSearch(dirToSearch, &filesToSearch, search_depth, &count_searchedDirs);

    std::cout << std::endl;
    Useful::PrintOut(std::string("Num [depth level]        : ")+std::to_string(search_depth));
    Useful::PrintOut(std::string("Num [searched sub-dir's] : ")+std::to_string(count_searchedDirs));
    Useful::PrintOut(std::string("Num [found valid entries]: ")+std::to_string(filesToSearch.size()));
    if(filesToSearch.size()==0) {
        Useful::PrintOut("closing program..");
        return 0;
    }
    
    std::cout << std::endl;
    
    Useful::ANSI_mvprint(0, 7, "processing entires:");

    std::string progressBar = "";
    int entryPathOpenFailures = 0;
    auto entry_itr = filesToSearch.begin();
    
    std::vector<stopTimeEvent> storedData;
    
    std::string::npos

    Useful::ANSI_mvprint(0, 8, "", true);
    for(size_t i=0; i<filesToSearch.size(); i++) {
        std::fstream entryFile(*entry_itr, std::ios::in |std::ios::binary);
        if(!entryFile) {
            entryPathOpenFailures++;
        }
        transit_realtime::TripUpdate trpUpdate;
        trpUpdate.ParseFromIstream(&entryFile);
        //---------- process start ----------

        Useful::ANSI_mvprint(0, 9, std::string("S.T.U. size:")+Useful::formatNumber(trpUpdate.stop_time_update_size()));
        for(size_t i_upd=0; i_upd<trpUpdate.stop_time_update_size(); i_upd++) {
            auto _departure = trpUpdate.stop_time_update(i_upd).departure();
            if(_departure.delay() > 0) {
                storedData.push_back(stopTimeEvent{
                    _departure.delay(),
                    _departure.time(),
                    _departure.uncertainty()
                });
            }
            // Useful::ANSI_mvprint(0, 8, Useful::formatNumber(i_upd));
        }

        //---------- process end   ----------
        progressBar = Useful::progressBar(i, filesToSearch.size());
        Useful::ANSI_mvprint(0, 10, "", false);
        fmt::print(progressBar);
        std::advance(entry_itr, 1);
        entryFile.close();
        
    }
    progressBar = Useful::progressBar(filesToSearch.size(), filesToSearch.size(), true, true);
    fmt::print(progressBar);
    
    Useful::ANSI_mvprint(0, 11, "");
    Useful::PrintOut("finished processing every entry.");
    Useful::PrintOut("Num [failures] :"+std::to_string(entryPathOpenFailures));
    
    std::cout << std::endl;
    Useful::PrintOut(std::string("Num departure delays found: ")+std::to_string(storedData.size()));
    for(size_t i=0; i<storedData.size(); i++) {
        std::cout << storedData.at(i).delay << " | " << storedData.at(i).time << " | " << storedData.at(i).uncertainty << std::endl;
    }
    
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}