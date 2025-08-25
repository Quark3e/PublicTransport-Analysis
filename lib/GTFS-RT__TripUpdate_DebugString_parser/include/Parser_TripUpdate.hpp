#pragma once
#ifndef HPP_LIB_PARSER_TRIPUPDATE
#define HPP_LIB_PARSER_TRIPUPDATE

#include <stdexcept>

#include <string>
#include <vector>


struct STE {
    int32_t delay;  // [1]
    int64_t time;   // [2]
    // int32_t uncertainty;     // [3] // uncertainty is avoided because it appears that data point isnt consistent
};
struct STU {
    uint32_t stop_sequence; // [1]
    STE arrival;    // [2]
    STE departure;  // [3]
    std::string stop_id;    // [4]
    int32_t schedule_relationship;  // [5]
};
struct TrpDsc {
    std::string trip_id;    // [1]
    std::string start_time; // [2]
    std::string start_date; // [3]
    int32_t schedule_relationship;  // [4]
    std::string route_id; // [5]
    uint32_t direction_id; // [6]
};
struct Vhcl {
    std::string id;
};
struct TrpUpd {
    uint64_t            timestamp{0};
    TrpDsc              trip{"", "", 0};
    std::vector<STU>    stop_time_updates;
    Vhcl                vehicle{""};
};

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


inline TrpUpd ParseDebugString(std::string _strToParse) {
    if(_strToParse.size()==0) throw std::runtime_error("Empty string");
    if(_strToParse.substr(0, 11) != "departure {") throw std::runtime_error("String doesn't contain initial signature substr \"departure {\"");

    TrpUpd _result{0, {"", "", "", 0, "", 0}, std::vector<STU>{}, {""}};
    std::string _isol = "";
    size_t _colonPos = 0;
    size_t _refIdx = 0;
    _colonPos = _strToParse.find(':');    //first colon: 'schedule_time'
    if(_refIdx==std::string::npos) throw std::runtime_error("No colons found in string.");
    
    _isol = _strToParse.substr(_colonPos+1, (_refIdx=_strToParse.find('\n', _colonPos))-_colonPos-1); // isolate timestamp substr whilst also updating _refIdx to hold the newline char that follows the colon.
    _result.timestamp = std::stoi(_isol);
    

    if(_strToParse[_refIdx+3]!='1') throw std::runtime_error("String doesn't contain identifier number of 'trip'. Raw string of line:\""+_strToParse.substr(_refIdx+1, _strToParse.find('\n', _refIdx+1)));
    _refIdx = _refIdx + 6;  //update idx to hold index to the newline char after opening curly braces for 'trip'
    
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
    

    _refIdx = _strToParse.find('\n', _refIdx+3); //set _refIdx to the newline char of the closing braces
    
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

    _refIdx = _strToParse.find(':', _refIdx+1);

    if(_strToParse.at(_refIdx-1)=='1') {
        _isol = _strToParse.substr(_refIdx+3, _strToParse.find('\"', _refIdx+3)-_refIdx-3);
        _result.vehicle.id = _isol;
    }


    return _result;
}


#endif //HPP_LIB_PARSER_TRIPUPDATE