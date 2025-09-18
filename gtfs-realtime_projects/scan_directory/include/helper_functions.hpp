#pragma once
#ifndef HPP_HELPER_FUNCTIONS
#define HPP_HELPER_FUNCTIONS

#include <includes.hpp>


/**
 * @brief Recursively searches directories up to a specified depth and stores the results.
 *
 * Traverses the directory tree starting from the given path, searching up to _maxDepth levels deep.
 * Stores the paths of found directories or files in the provided list. Optionally tracks the number
 * of directories searched.
 *
 * @param _dirPath The starting directory path for the search.
 * @param _storeResult Pointer to a list where the found directory or file paths will be stored.
 * @param _maxDepth The maximum depth to search within the directory tree.
 * @param _numDirSearched Optional pointer to a variable that will be incremented with the number of directories searched.
 */
void func_depthSearch(std::string _dirPath, std::list<std::string> *_storeResult, int _maxDepth, size_t *_numDirSearched=nullptr);

int64_t parse_epochTime_fromFilename(std::string _toParse);

std::string parse_date_fromFilename(std::string _toParse, struct tm* tmPtr = nullptr);

template <typename T>
struct atomwrapper
{
  std::atomic<T> _a;

  atomwrapper()
    :_a()
  {}

  atomwrapper(const std::atomic<T> &a)
    :_a(a.load())
  {}

  atomwrapper(const atomwrapper &other)
    :_a(other._a.load())
  {}

  atomwrapper &operator=(const atomwrapper &other)
  {
    _a.store(other._a.load());
  }
};


#include <initializer_list>

namespace ThrdPrf {

    struct __element__threadPerf {
        std::string label;
        double multiplier;

        __element__threadPerf(std::string _label, double _multiplier=1): label(_label), multiplier(_multiplier) {
            refrTimePoint = std::chrono::steady_clock::now();
        }

        std::chrono::steady_clock::time_point refrTimePoint;
        std::vector<std::chrono::duration<double>> durations;
        
    };

    class ThreadPerf {
        private:

        std::vector<__element__threadPerf> perf_objects;
        size_t numStored_durations;

        public:

        ThreadPerf() {}
        ThreadPerf(size_t _numStored_durations): numStored_durations(_numStored_durations) {}
        ThreadPerf(size_t _numStored_durations, std::initializer_list<__element__threadPerf> _perf_elements): numStored_durations(_numStored_durations), perf_objects(_perf_elements) {
            for(size_t i=0; i<perf_objects.size(); i++) {
                for(size_t j=i+1; j<perf_objects.size(); i++) {
                    if(perf_objects.at(i).label==perf_objects.at(j).label) 
                        throw std::runtime_error("ThreadPerf(size_t, std::initializer_list<__element__threadPerf>): ERROR: label duplicate found for labels at initializer indices ["+std::to_string(i)+"] and ["+std::to_string(j)+"]");
                }
            }
        }
        ThreadPerf(size_t _numStored_durations, std::initializer_list<std::string> _perf_element_labels): numStored_durations(_numStored_durations) {
            auto itr_i = _perf_element_labels.begin();
            for(size_t i=0; i<_perf_element_labels.size()-1; i++) {
                auto itr_j = _perf_element_labels.begin();
                std::advance(itr_j, i+1);
                for(size_t j=i+1; j<_perf_element_labels.size(); j++) {
                    if((*itr_i).compare(*itr_j)==0) {
                        throw std::runtime_error("ThreadPerf(size_t, std::initializer_list<std::string>): ERROR: label duplicate found for labels at initializer indices ["+std::to_string(i)+"] and ["+std::to_string(j)+"]");
                    }
                    if(j+1<_perf_element_labels.size()) std::advance(itr_j, 1);
                }
                std::advance(itr_i, 1);
            }

            for(auto el_label : _perf_element_labels) {
                perf_objects.push_back(el_label);
            }
        }

        ThreadPerf(const ThreadPerf& _copy) {
            perf_objects = _copy.perf_objects;
            numStored_durations = _copy.numStored_durations;
        }
        ~ThreadPerf() {

        }

        __element__threadPerf &operator[](size_t _i) {
            return perf_objects.at(_i);
        }
        __element__threadPerf operator[](size_t _i) const {
            return perf_objects.at(_i);
        }


        int set_T_start(std::string _label) {
            bool objFound = false;
            for(size_t i=0; i<perf_objects.size(); i++) {
                if(perf_objects.at(i).label==_label) {
                    perf_objects.at(i).refrTimePoint = std::chrono::steady_clock::now();
                    objFound = true;
                    break;
                }
            }
            if(!objFound) {
                perf_objects.push_back({_label});
                return 1;
            }

            return 0;
        }
        __element__threadPerf &set_T_end(std::string _label) {
            auto currTime = std::chrono::steady_clock::now();
            for(size_t i=0; i<perf_objects.size(); i++) {
                if(perf_objects.at(i).label==_label) {
                    if(numStored_durations==0 || perf_objects.at(i).durations.size()<numStored_durations) {
                        perf_objects.at(i).durations.push_back(currTime - perf_objects.at(i).refrTimePoint);
                    }
                    else {
                        std::rotate(perf_objects.at(i).durations.begin(), perf_objects.at(i).durations.begin()+1, perf_objects.at(i).durations.end());
                        perf_objects.at(i).durations.back() = currTime - perf_objects.at(i).refrTimePoint;
                    }
                    return perf_objects.at(i);
                }
            }
            throw std::runtime_error("ThreadPerf::set_T_end(std::string): _label arg \""+_label+"\" not found.");
        }

        std::chrono::duration<double> get_duration(std::string _label) {
            bool objFound = false;
            for(size_t i=0; i<perf_objects.size(); i++) {
                if(perf_objects.at(i).label==_label) {
                    if(perf_objects.at(i).durations.size()==0) throw std::runtime_error("ThreadPerf::getDuration(std::string): _label arg \""+_label+"\" has no recorded durations.");
                    return perf_objects.at(i).durations.back();
                }
            }
            throw std::runtime_error("ThreadPerf::getDuration(std::string): _label arg \""+_label+"\" not found.");
        }
        std::chrono::duration<double> get_duration(size_t _i) {
            if(perf_objects.at(_i).durations.size()==0) throw std::runtime_error("ThreadPerf::getDuration(size_t): index arg ["+std::to_string(_i)+"] has no recorded durations.");
            return perf_objects.at(_i).durations.back();
        }
        std::vector<std::chrono::duration<double>> get_allDurations(std::string _label) {
            bool objFound = false;
            for(size_t i=0; i<perf_objects.size(); i++) {
                if(perf_objects.at(i).label==_label) {
                    if(perf_objects.at(i).durations.size()==0) throw std::runtime_error("ThreadPerf::getAllDurations(std::string): _label arg \""+_label+"\" has no recorded durations.");
                    return perf_objects.at(i).durations;
                }
            }
            throw std::runtime_error("ThreadPerf::getAllDurations(std::string): _label arg \""+_label+"\" not found.");
        }
        std::vector<std::chrono::duration<double>> get_allDurations(size_t _i) {
            if(perf_objects.at(_i).durations.size()==0) throw std::runtime_error("ThreadPerf::getAllDurations(size_t): index arg ["+std::to_string(_i)+"] has no recorded durations.");
            return perf_objects.at(_i).durations;
        }
        std::chrono::duration<double> get_avgDuration(std::string _label) {
            bool objFound = false;
            for(size_t i=0; i<perf_objects.size(); i++) {
                if(perf_objects.at(i).label==_label) {
                    if(perf_objects.at(i).durations.size()==0) throw std::runtime_error("ThreadPerf::getAvgDuration(std::string): _label arg \""+_label+"\" has no recorded durations.");
                    std::chrono::duration<double> totalDur(0);
                    for(auto dur : perf_objects.at(i).durations) totalDur += dur;
                    return (totalDur / perf_objects.at(i).durations.size());
                }
            }
            throw std::runtime_error("ThreadPerf::getAvgDuration(std::string): _label arg \""+_label+"\" not found.");
        }
        std::chrono::duration<double> get_avgDuration(size_t _i) {
            if(perf_objects.at(_i).durations.size()==0) throw std::runtime_error("ThreadPerf::getAvgDuration(size_t): index arg ["+std::to_string(_i)+"] has no recorded durations.");
            std::chrono::duration<double> totalDur(0);
            for(auto dur : perf_objects.at(_i).durations) totalDur += dur;
            return (totalDur / perf_objects.at(_i).durations.size());
        }

        size_t size() const {
            return perf_objects.size();
        }

        std::string get_label(size_t _i) const {
            return perf_objects.at(_i).label;
        }

        size_t get_durationsSizeLim() const {
            return numStored_durations;
        }
        size_t set_durationsSizeLim(size_t _newDurationsSize) {
            for(auto& threadObj_el : perf_objects) {
                if(threadObj_el.durations.size() > _newDurationsSize) {
                    auto itr__el_dur = threadObj_el.durations.begin();
                    std::advance(itr__el_dur, threadObj_el.durations.size()-_newDurationsSize);
                    threadObj_el.durations.erase(threadObj_el.durations.begin(), itr__el_dur);
                }
            }

            return numStored_durations;
        }

    };

}


#ifndef HPP_LIB_PARSER_TRIPUPDATE
#define HPP_LIB_PARSER_TRIPUPDATE


inline size_t _findSubstr(std::string _toFind, std::string _toSearch) {
    size_t size_toFind  = _toFind.size();
    size_t size_toSearch= _toSearch.size();
    if(size_toFind==0) throw std::runtime_error("findSubstr(std::string, std::string) arg for _toFind is empty, which is not allowed.");
    else if(size_toSearch==0) throw std::runtime_error("findSubstr(std::string, std::string) arg for _toSearch is empty, which is not allowed.");
    else if(size_toFind>size_toSearch) throw std::runtime_error("findSubstr(std::string, std::string) string length of _toFind is bigger than _toSearch which is not allowed.");
    else if(size_toFind==size_toSearch) return (_toFind==_toSearch? 0 : std::string::npos);

    size_t pos = 0;
    bool matchFound = true;
    for(size_t i=0; i<size_toSearch-size_toFind; i++) {
        if(_toSearch.at(i)==_toFind.at(0)) {
            matchFound = true;
            for(size_t ii=0; ii<size_toFind; ii++) {
                if(_toFind.at(ii)!=_toSearch.at(i+ii)) {
                    matchFound = false;
                    break;
                }
            }
            if(matchFound) return i;
        }
    }
    return std::string::npos;
}


inline TrpUpd ParseDebugString(std::string _strToParse, ThrdPrf::ThreadPerf* _ptrThreadPerf=nullptr) {
    if(_strToParse.size()==0) throw std::runtime_error("Empty string");
    if(_strToParse.substr(0, 11) != "departure {") throw std::runtime_error("String doesn't contain initial signature substr \"departure {\"");

    if(_ptrThreadPerf) _ptrThreadPerf->set_T_start("Variable Init");
    TrpUpd _result{0, {"", "", "", 0, "", 0}, std::vector<STU>{}, {""}};
    std::string _isol = "";
    size_t _colonPos = 0;
    size_t _refIdx = 0;
    _colonPos = _strToParse.find(':');    //first colon: 'schedule_time'
    if(_refIdx==std::string::npos) throw std::runtime_error("No colons found in string.");
    
    _isol = _strToParse.substr(_colonPos+1, (_refIdx=_strToParse.find('\n', _colonPos))-_colonPos-1); // isolate timestamp substr whilst also updating _refIdx to hold the newline char that follows the colon.
    _result.timestamp = std::stoi(_isol);
    if(_ptrThreadPerf) _ptrThreadPerf->set_T_end("Variable Init").multiplier+=1;

    if(_strToParse[_refIdx+3]!='1') throw std::runtime_error("String doesn't contain identifier number of 'trip'. Raw string of line:\""+_strToParse.substr(_refIdx+1, _strToParse.find('\n', _refIdx+1)));
    _refIdx = _refIdx + 6;  //update idx to hold index to the newline char after opening curly braces for 'trip'
    
    if(_ptrThreadPerf) _ptrThreadPerf->set_T_start("Parse TripDescriptor");
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

    if(_ptrThreadPerf) _ptrThreadPerf->set_T_end("Parse TripDescriptor").multiplier+=1;

    _refIdx = _strToParse.find('\n', _refIdx+3); //set _refIdx to the newline char of the closing braces
    
    if(_ptrThreadPerf) _ptrThreadPerf->set_T_start("Parse StopTimeUpdates");
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
                    _refIdx +=_findSubstr("\n    }", _strToParse.substr(_refIdx))+6;
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

    if(_ptrThreadPerf) _ptrThreadPerf->set_T_end("Parse StopTimeUpdates").multiplier+=1;

    _refIdx = _strToParse.find(':', _refIdx+1);

    if(_strToParse.at(_refIdx-1)=='1') {
        _isol = _strToParse.substr(_refIdx+3, _strToParse.find('\"', _refIdx+3)-_refIdx-3);
        _result.vehicle.id = _isol;
    }


    return _result;
}


#endif //HPP_LIB_PARSER_TRIPUPDATE


#endif //HPP_HELPER_FUNCTIONS