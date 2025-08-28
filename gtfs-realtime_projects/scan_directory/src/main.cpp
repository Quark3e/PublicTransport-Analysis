
#include <includes.hpp>
#include <main_subprocess.hpp>


int main(int argc, char** argv) {
    program_running = true;
	
	
	for(size_t i=2; i<argc; i++) {
		std::string arg_str(argv[i]);
		
		
		size_t idx_setThreadLim = 0;
		if((idx_setThreadLim=Useful::findSubstr("-threadLim:", arg_str))!=std::string::npos) {
			setThreadLim = std::stoul(arg_str.substr(idx_setThreadLim+11));
		}
	}
	
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    dim_terminal = Useful::getTerminalSize();
    Useful::clearScreen();
    Useful::PrintOut("Program start.", dim_terminal.x, "left","\n",true,false,false,1,1,&terminalCursorPos);

    if(argc <= 1) {
        std::cerr << "Invalid number of argc. No path to directory given." << std::endl;
        program_running = false;
        return 1;
    }

    std::string path_dirToSearch;
    path_dirToSearch = argv[1];
    if(!std::filesystem::is_directory(path_dirToSearch)) {
        std::cerr << "ERROR: the program argument for path given is not a valid path." << std::endl;
        program_running = false;
        return 1;
    }

    int search_depth = 1;


    Useful::PrintOut("loading file_static_refData__trips vector with trip values which's route_id matched..", std::string::npos, "left", "",true,false,false,1,1,&terminalCursorPos);
    std::fstream file_static_refData__trips(path_static_historical_data+"/trips.csv", std::ios::in);
    for(std::string _line; std::getline(file_static_refData__trips, _line, '\n');) {
        bool matched = false;
        for(std::string route : route_id__toSearch) {
            if(route==_line.substr(0, 16)) {
                trip_id__found.push_back(_line.substr(_line.find(',', 17)+1, 17));
                break;
            }
        }
    }
    std::cout << " found " << trip_id__found.size() << " num trips.\n";
    terminalCursorPos.x = 0;
    terminalCursorPos.y++;
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


    #if useStopTimesFile
    
    auto refrTree = subProcess_loadFile__stop_times(path_static_historical_data+"/stop_times.csv", staticRefData__stop_times, trip_id__found);
    #endif //add stop_times arg to refrTree


    std::list<std::string> filesToSearch;
    size_t count_searchedDirs = 0;
    func_depthSearch(path_dirToSearch, &filesToSearch, search_depth, &count_searchedDirs);

    
    Useful::PrintOut(std::string("Num [depth level]        : ")+std::to_string(search_depth),dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    Useful::PrintOut(std::string("Num [searched sub-dir's] : ")+std::to_string(count_searchedDirs),dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    Useful::PrintOut(std::string("Num [found valid file entries]: ")+std::to_string(filesToSearch.size()),dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    if(filesToSearch.size()==0) {
        Useful::PrintOut("found no entries. closing program..");
        program_running = false;
        return 0;
    }
    

    ///-------------------------------------------------------------
    
    terminalCursorPos.y+=3;

    
    size_t numTotalTripsRead = 0;
    size_t entryPathOpenFailures = 0;
    std::vector<STU_refd> storedData_tripDelays_idx;

    std::vector<parseException_DebugString> vecExceptions_DebugString = subProcess_processEntries(filesToSearch, refrTree, numTotalTripsRead, entryPathOpenFailures, storedData_tripDelays_idx);

    
    Useful::PrintOut("finished processing every entry.",dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);

    std::ofstream file__vecExceptions_DebugString("dataFile__vecExceptions_DebugString.csv");
    if(file__vecExceptions_DebugString.is_open()) {
        for(auto el : vecExceptions_DebugString) {
            file__vecExceptions_DebugString << "\""<< el.what << "\"," << el.where << "\n";
        }
        file__vecExceptions_DebugString.close();
    }

    std::string progressBar = Useful::progressBar(filesToSearch.size(), filesToSearch.size(), true, true);
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+=5, "", false);
    fmt::print(progressBar);
    
    Useful::ANSI_mvprint(terminalCursorPos.x, terminalCursorPos.y+=5, "");
    Useful::PrintOut("finished processing every entry.",dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    Useful::PrintOut("Num [entryPathOpenFailures]       : "+std::to_string(entryPathOpenFailures),dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    Useful::PrintOut("Num [vecExceptions_DebugString]   : "+std::to_string(vecExceptions_DebugString.size()),dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    
    std::fstream file_foundDelays(std::string("dataFile_foundDelays_")+std::to_string(storedData_tripDelays_idx.size())+".csv", std::ios::out);
    file_foundDelays << "filename_epoch,trip_id,STU_idx,stop_sequence,stop_id,delay_arrival,delay_departure\n";
    for(size_t i=0; i<storedData_tripDelays_idx.size(); i++) {
        auto refd = storedData_tripDelays_idx.at(i);
        file_foundDelays << refd.filename_epoch << "," << refd.trip_id << "," << refd.STU_idx << "," << refd.stop_sequence << "," << refd.stop_id << "," << refd.arrival.delay << "," << refd.departure.delay << "\n";
    }
    file_foundDelays.close();
    Useful::PrintOut(std::string("Num delays found: ")+std::to_string(storedData_tripDelays_idx.size())+" out of "+std::to_string(numTotalTripsRead)+" total trips.",dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);

    Useful::PrintOut("\n\nFinished program. Closing.",dim_terminal.x+1, "left","\n",true,false,false,1,1,&terminalCursorPos);
    
    google::protobuf::ShutdownProtobufLibrary();
    std::cout << std::endl;

    program_running = false;

    return 0;
}


