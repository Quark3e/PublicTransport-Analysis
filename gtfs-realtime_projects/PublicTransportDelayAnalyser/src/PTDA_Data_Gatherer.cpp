
#include <PTDA_Data_Gatherer.hpp>


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
        bit7z_ref.setProgressCallback([this](uint64_t var) {callback_progress(var)});
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

        DG_refObj.progressInfo.message << std::this_thread::get_id() << " | [bit7z] Decompression progress: " << Useful::formatNumber(progressInfo.progress.percent,5,1) << "% ";
        DG_refObj.progressInfo.message << "(" << Useful::formatNumber(DG_refObj.progressInfo.progress.now,strLen_total,0,"right",false,true) << "/" << (progressInfo.progress.total,strLen_total,0,"right",false,true) <<" bytes)\n";
    
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
    std::unique_lock<std::mutex> u_lck_accss__url_components(DG_ref.mtx_access__url_components, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__TaskRequest(DG_ref.mtx_access__TaskRequest, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__data_shared(DG_ref.mtx_access__data_shared, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__callback_errors(DG_ref.mtx_access__callback_errors, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__callback_updates(DG_ref.mtx_access__callback_updates, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__progressInfo(DG_ref.mtx_access__progressInfo, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__path_dirTempCompressed(DG_ref.mtx_access__path_dirTempCompressed, std::defer_lock);
    std::unique_lock<std::mutex> u_lck_accss__path_dirDelayFileOut(DG_ref.mtx_access__path_dirDelayFileOut, std::defer_lock);

    std::thread::id threadID = std::this_thread::get_id();


    DG_ref.threadRunning = true;
    DG_ref.status = running;
    while(DG_ref.threadRunning) {
        if(!DG_ref.newTaskRequested.load()) continue;

        for(size_t i=0; i<DG_ref.currentTaskRequest.dates.size(); i++) {
            
            tm temp_tm = DG_ref.currentTaskRequest.dates.at(i);
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

            /// ---------- Uncompressing downloaded raw data files ----------

            try {

                bit7z::Bit7zLibrary lib{"C:\\Program Files\\7-Zip\\7z.dll"};
                bit7z::BitFileExtractor extractor{lib, bit7z::BitFormat::SevenZip};

                bit7z_callbackClass callbackClass(DG_ref);

                // extractor.setTotalCallback(callbackClass.callback_total);
                // extractor.setProgressCallback(callbackClass.callback_progress);
                // extractor.setTotalCallback(bit7z_callbackClass::callback_total(&DG_ref));
                // extractor.setProgressCallback(bit7z_callbackClass::callback_progress(&DG_ref));
                // extractor.setTotalCallback([DG_ref](uint64_t var) {DG_ref.callback_total(var);});
                // extractor.setProgressCallback([DG_ref](uint64_t var) {DG_ref.callback_progress(var)});
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
            }
            
            /// ---------- "Processing"/parsing the raw data files ----------


        }

    }
    

}
void DGNC::threadFunc_Task__GET_Request() {
    
}
void DGNC::threadFunc_Task__ExtractFiles() {

}
void DGNC::threadFunc_Task__ParseDelays() {

}


DGNC::DataGatherer::DataGatherer(std::string _api_key, bool _initialise=true, std::string _path_dirDelayFileOut="", std::string _path_tempDirCompressed=""): 
    path_dirDelayFileOut(_path_dirDelayFileOut), path_dirTempCompressed(_path_tempDirCompressed)
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
    if(this->status==TaskRequest_Status::running) throw std::runtime_error("DGNC::DataGatherer::setNewTask(DGNC::TaskRequest): a task is currently running.");
    if(_newRequest.dates.size()==0) throw std::runtime_error("DGNC::DataGatherer::setNewTask(DGNC::TaskRequest): _newRequest.dates.size() cannot be 0");
    if(_newRequest.stopIDs.size()==0) throw std::runtime_error("DGNC::DataGatherer::setNewTask(DGNC::TaskRequest): _newRequest.stopIDs.size() cannot be 0");

    std::unique_lock<std::mutex> u_lck_setTaskReq(this->mtx_access__TaskRequest);
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
std::vector<STU_refd> DGNC::DataGatherer::getData() {
    std::unique_lock<std::mutex> u_lck(this->mtx_access__data_shared);

    return this->data_shared;
}
PTDA_Coms DGNC::DataGatherer::getProgressInfo() {
    std::unique_lock<std::mutex> u_lck(this->mtx_access__progressInfo);
    return progressInfo;
}
DGNC::TaskRequest_Status DGNC::DataGatherer::getStatus() {
    return this->status.load();
}

void DGNC::DataGatherer::swapSharedData(std::vector<STU_refd>& _refToSwap) {
    std::unique_lock<std::mutex> u_lck(this->mtx_access__data_shared);
    _refToSwap.swap(this->data_shared);
    return;
}

