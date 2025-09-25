#pragma once
#ifndef HPP__PTDA_Variables
#define HPP__PTDA_Variables

#include <cstdint>
#include <string>
#include <vector>

#include <Pos2d.hpp>

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


struct STU_refd : public STU {
    std::string trip_id;
    uint32_t STU_idx;
    int64_t filename_epoch;

    STU_refd(STU _stu, std::string _trip_id, uint32_t _stu_idx, int64_t _file_epoch): STU{_stu}, trip_id{_trip_id}, STU_idx{_stu_idx}, filename_epoch{_file_epoch} {}
};


struct stopInfo_tripRelatives {
    std::string trip_id;
    tm arrival_time;
    tm departure_time;
    uint32_t stop_sequence;
    std::string stop_headsign;
};

struct stopInfo {
    std::string stop_id;
    std::string stop_name;
    Pos2d<float> map_coord;
    std::vector<stopInfo_tripRelatives> tripRelatives;
};



#endif //HPP__PTDA_Variables