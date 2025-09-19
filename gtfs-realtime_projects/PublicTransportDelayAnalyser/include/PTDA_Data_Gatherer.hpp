#pragma once
#ifndef HPP__PTDA_Data_Gatherer
#define HPP__PTDA_Data_Gatherer

#include <thread>
#include <mutex>
#include <atomic>

#include <map>
#include <vector>
#include <string>

namespace DGNC {


    /**
     * A struct for a Task request that's sent to the subthread running the data gatherer class.
     * It contains details on what date's to collect raw data of and what stop_id's to find delays.
     * 
     * 
     */
    struct Task_request {
        /**
         * std::map<[date:"year-month-day"], std::vector<std::string>{[StopID]}>
         * std::map of the stopID's to collect for the given dates.
         */
        std::map<std::string, std::vector<std::string>> dateToStopIDs;

        std::atomic<bool> finished{false};


    };

};

class DataGatherer {
private:

    std::thread thread_dataGatherer;
public:

    DataGatherer();
    ~DataGatherer();

    

};


#endif //HPP__PTDA_Data_Gatherer