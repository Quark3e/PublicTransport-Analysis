#pragma once
#ifndef HPP_VARIABLES
#define HPP_VARIABLES


#include <includes.hpp>

struct StopID_refrSorted__trip_id {
    std::string trip_id;
    std::string stop_id;
};
struct StopID_refrSorted__stop_seq {
    uint32_t stop_sequence;
    std::vector<StopID_refrSorted__trip_id> stop_seq_vec;
};
struct StopID_refrSorted {
    std::vector<StopID_refrSorted__stop_seq> vec;
};


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



inline Pos2d<size_t>    terminalCursorPos{0, 1};
inline Pos2d<int>       dim_terminal{0, 0};


inline std::string path_static_historical_data = "C:/Users/berkh/Projects/Github_repo/PublicTransport-Analysis/dataset/static_historical_data/GTFS-SL-2025-01-22";


inline std::vector<std::string> route_id__toSearch{"9011001004000000", "9011001004100000"}; //for now, we just check for everything on this route, which may include trips
// std::vector<std::string> shape_id__toSearch{
//     "4014010000492969158", // Stockholm City -> Södertälje Centrum
//     "4014010000492969248", // Uppsala C -> Södertälje Centrum
//     "4014010000492970270", // Södertälje Centrum -> Märsta
// }; //way too many shape_id's that point to specific trips even if they're the exact same shape. Need to automate this.
inline std::vector<std::string> trip_id__found;
inline std::vector<std::vector<std::string>> staticRefData__stop_times;




#endif //HPP_VARIABLES