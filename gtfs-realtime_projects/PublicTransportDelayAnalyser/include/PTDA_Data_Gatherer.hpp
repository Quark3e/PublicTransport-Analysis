#pragma once
#ifndef HPP__PTDA_Data_Gatherer
#define HPP__PTDA_Data_Gatherer

#include <thread>
#include <mutex>
#include <atomic>

#include <map>
#include <vector>
#include <string>

#include <filesystem>

#include <Pos2d.hpp>
#include <bit7z/bitfileextractor.hpp>


#include <CURL_GET_downloader.hpp>
#include <PTDA_Variables.hpp>
#include <PTDA_Coms.hpp>
#include <PTDA_Useful.hpp>

using funcType_updateCallback = void(*)(PTDA_Coms&);
// typedef void(*funcType_updateCallback)(PTDA_Coms*);

class bit7z_callbackClass;

namespace DGNC {
    enum TaskRequest_Status {
        error   = -1,
        running =  0,
        finished=  1,
        noTask  =  2
    };

    /**
     * A struct for a Task request that's sent to the subthread running the data gatherer class.
     * 
     */
    struct TaskRequest {
        std::vector<tm> dates;
        std::vector<std::string> stopIDs;
    };

    class DataGatherer {
    private:
        std::string url_base{"https://api.koda.trafiklab.se/KoDa/api/v2/gtfs-rt/{operator}/{feed}?date={date}&key={api_key}"};
        std::map<std::string, std::string> url_components{
            {"operator",    "sl"},
            {"feed",        "TripUpdates"},
            {"date",        ""},
            {"api_key",     ""}
        };
        std::mutex mtx_access__url_components;

        friend class bit7z_callbackClass;

        std::thread thread_dataGatherer;
        friend void threadFunc(DGNC::DataGatherer& DG_ref);
        static int threadFunc__callbackDownloadProgress(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
        friend void threadFunc_Task__GET_Request();
        friend void threadFunc_Task__ExtractFiles();
        friend void threadFunc_Task__ParseDelays();

        TaskRequest currentTaskRequest;
        std::mutex mtx_access__TaskRequest;
        
        std::vector<STU_refd> data_private;
        std::vector<STU_refd> data_shared;
        std::mutex mtx_access__data_shared;
        
        funcType_updateCallback callback_errors = nullptr;
        funcType_updateCallback callback_updates = nullptr;
        std::mutex mtx_access__callback_errors;
        std::mutex mtx_access__callback_updates;
        
        static inline PTDA_Coms progressInfo;
        static inline std::mutex mtx_access__progressInfo;

        std::string path_dirTempCompressed = "";
        std::string path_dirDelayFileOut = "";
        std::mutex mtx_access__path_dirTempCompressed;
        std::mutex mtx_access__path_dirDelayFileOut;

        std::atomic<TaskRequest_Status> status{noTask};
        std::atomic<bool> newTaskRequested{false};

        std::atomic<bool> threadRunning{false};
        static inline bool classInit = false;
    public:
        DataGatherer(std::string _api_key, bool _initialise=true, std::string _path_dirDelayFileOut="", std::string _path_tempDirCompressed="");
        ~DataGatherer();
    
        int init();

        int setApiKey(std::string _key);

        int setNewTask(TaskRequest _newRequest);
        
        int setCallback_errors(funcType_updateCallback _callbackFunc);
        int setCallback_updates(funcType_updateCallback _callbackFunc);

        void setPath_dirDelayFileOut(std::string _newPath);
        void setPath_dirTempCompressed(std::string _newPath);

        TaskRequest             getCurrentRequest();
        std::vector<STU_refd>   getData();
        PTDA_Coms               getProgressInfo();
        TaskRequest_Status      getStatus();

        void swapSharedData(std::vector<STU_refd>& _refToSwap);
    };
};



#endif //HPP__PTDA_Data_Gatherer