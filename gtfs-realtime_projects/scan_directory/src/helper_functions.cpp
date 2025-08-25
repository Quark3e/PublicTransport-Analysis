

#include <helper_functions.hpp>

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

