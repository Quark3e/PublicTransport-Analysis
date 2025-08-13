
#include <includes.hpp>


int main(int argc, char** argv) {
    if(argc <= 1) {
        throw std::runtime_error("Invalid number of argc. No path to directory given.");
    }
    std::string dirToSearch(argv[1]);
    if(!std::filesystem::is_directory(dirToSearch)) {
        throw std::runtime_error("ERROR: the program argument for path given is not a valid path.");
    }

    /**
     * 1. Search dir for entries and save every entry's path in a vector *IF* the path ends in a `.pb`
     * 2. 
    */

    std::vector<std::string> filesToSerach;
    for(auto entry : std::filesystem::directory_iterator(dirToSearch)) {
        // std::string entrypath = entry.path().string();
        if(entry.path().extension()==".pb") {
            filesToSerach.push_back(entry.path().string());
        }
    }

    

    return 0;
}