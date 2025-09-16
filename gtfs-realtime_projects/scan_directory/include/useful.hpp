#pragma once
#ifndef HPP_USEFUL
#define HPP_USEFUL


#include <vector>
#include <string>

#include <cmath>
#include <algorithm>

#include <sstream>
#include <iomanip>
#include <locale>
#include <iostream>

#include <Pos2d.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#else
#include <sys/ioctl.h>
#include <linux/limits.h>
#include <unistd.h>
#endif


// #if __cplusplus < 201703L
//     #include <experimental/filesystem>
//     namespace fs = std::experimental::filesystem;
// #else
//     #include <filesystem>
//     namespace fs = std::filesystem;
// #endif //__cplusplus < 201703L
#include <filesystem>
// namespace fs = std::filesystem;


#ifndef _max
#define _max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef _min
#define _min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


#ifndef RADIANS
// Convert degrees to radians
#define RADIANS(deg) (double(deg)*(double(M_PI)/double(180.0)))
#endif //RADIANS

#ifndef DEGREES
// Convert radians to degrees
#define DEGREES(rad) (double(rad)*double(180.0))/double(M_PI)
#endif //DEGREES


namespace Useful {

    #if _WIN32
        #include <windows.h>

        inline unsigned long long getTotalSystemMemory() {
            MEMORYSTATUSEX status;
            status.dwLength = sizeof(status);
            GlobalMemoryStatusEx(&status);
            return status.ullTotalPhys;
        }
        inline unsigned long long getTotalAvailMemory() {
            MEMORYSTATUSEX status;
            status.dwLength = sizeof(status);
            GlobalMemoryStatusEx(&status);
            return status.ullAvailPhys;
        }
    #else
        #include <unistd.h>
        inline unsigned long long getTotalSystemMemory() {
            long pages = sysconf(_SC_PHYS_PAGES);
            long page_size = sysconf(_SC_PAGE_SIZE);
            return pages * page_size;
        }
    #endif //WIN32


    /**
     * @brief Retrieves the current system time as a formatted string.
     *
     * This function obtains the current time from the system clock and returns it
     * as a human-readable string in the format produced by `ctime`, excluding the trailing newline.
     *
     * @return std::string The current time as a formatted string (e.g., "Wed Jun 12 15:23:45 2024").
     */
    inline std::string getCurrentTime() {

        std::chrono::system_clock::time_point __CURRENT__TIME_POINT   = std::chrono::system_clock::now();
        time_t __CURRENT__TIME_T       = std::chrono::system_clock::to_time_t(__CURRENT__TIME_POINT);
        std::string _tempStr = ctime(&__CURRENT__TIME_T);
        std::string __CURRENT__TIME_STRING  = _tempStr.substr(0, _tempStr.length()-1);
        
        return __CURRENT__TIME_STRING;
    }


    struct HumanReadable {
        std::uintmax_t size{0};

        HumanReadable() {}
        HumanReadable(const HumanReadable &_copy) {
            size = _copy.size;
        }
        HumanReadable(std::uintmax_t _size): size(_size) {}
        ~HumanReadable() {}

        HumanReadable& operator=(const HumanReadable& m) {
            size = m.size;
            return *this;
        }

        std::string stringify() const {
            return this->stringify(this->size);
        }
        std::string stringify(std::uintmax_t _val) const {
            int i{};
            double mantissa = _val;
            std::stringstream ss;
            
            /// Reduce and fit the size value into one of the prefixes.
            for(; mantissa >= 1024.0; mantissa /= 1024.0, i++) {}
            
            ss << std::ceil(mantissa * 10.0) / 10.0 << i["BKMGTPE"];
            if(i) ss << "B";
            return ss.str();
        }

        operator std::string() {
            return stringify();
        }


        friend auto operator<<(std::ostream &os, HumanReadable const &hr) -> std::ostream& {
            os << hr.stringify();
            return os;
        }
    };

    
    /***
     * @brief Get size of terminal window
     * @param width reference to integer variable for width
     * @param height reference to integer variable for height
     * @return `0`-successful; `-1`-error
    */
    inline int getTermSize(int &width, int &height) {
#if _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        int columns, rows;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        width   = csbi.srWindow.Right   - csbi.srWindow.Left    + 1;
        height  = csbi.srWindow.Bottom  - csbi.srWindow.Top     + 1;
#else
        struct winsize winDim;
        if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&winDim)==-1) {
            return -1;
        }
        width   = winDim.ws_col;
        height  = winDim.ws_row;
#endif
        return 0;
    }


    /**
     * @brief Retrieves the current terminal or console window size.
     *
     * This function sets the provided width and height references to the number of columns and rows
     * of the terminal or console window. It supports both Windows and POSIX systems.
     *
     * @param[out] width  Reference to an integer where the terminal width (columns) will be stored.
     * @param[out] height Reference to an integer where the terminal height (rows) will be stored.
     * @return int Returns 0 on success, or -1 on failure (only possible on POSIX systems).
     */
    inline int getTerminalSize(int &width, int &height) {
#if _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        int columns, rows;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        width   = csbi.srWindow.Right   - csbi.srWindow.Left    + 1;
        height  = csbi.srWindow.Bottom  - csbi.srWindow.Top     + 1;
#else
        struct winsize winDim;
        if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&winDim)==-1) {
            return -1;
        }
        width   = winDim.ws_col;
        height  = winDim.ws_row;
#endif
        return 0;
    }

    /**
     * @brief Retrieves the current terminal size.
     *
     * This function calls an overloaded getTerminalSize(int&, int&) to obtain the
     * number of columns and rows of the terminal window. If the call fails,
     * it throws a std::runtime_error with the error code.
     *
     * @return Pos2d<int> An object containing the number of columns (x) and rows (y).
     * @throws std::runtime_error if retrieving the terminal size fails.
     */
    inline Pos2d<int> getTerminalSize() {
        int columns = 0, rows = 0;
        int retCode = 0;
        if((retCode = getTerminalSize(columns, rows))!=0) {
            throw std::runtime_error("ERROR: getTerminalSize(int&, int&) returned: "+std::to_string(retCode));
        }
        return {columns, rows};
    }


    inline std::string getDate(bool addNewline = true) {
        time_t currDate = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::string _out = ctime(&currDate);
        if(addNewline) return _out;
        else {
            return _out.substr(0, _out.length()-1);
        }
    }

    /**
     * @brief Get the absolute path of the current working directory
     * @param inclEndSlash whether to include `/` at the end of return path string
     * @return std::string of path:
    */
    inline std::string getCWD(bool inclEndSlash=true) {
#if _WIN32
        char cwd[MAX_PATH];
        if(_getcwd(cwd, sizeof(cwd)) != NULL)
#else
        char cwd[PATH_MAX];
        if(getcwd(cwd, sizeof(cwd)) != NULL)
#endif
        {
            // std::cout<<cwd<<std::endl;
            std::string returStr = cwd;
            if(inclEndSlash) return returStr+"/";
            else return returStr;
        }
        else {
            std::cout << "getcwd() error." << std::endl;
            return "";
        }
    }


    /**
     * @brief Retrieves the full path of the currently running executable.
     *
     * This function returns the absolute path to the executable file of the current process.
     * On Windows, it uses GetModuleFileNameA to obtain the path.
     * On Unix-like systems, it reads the symbolic link "/proc/self/exe".
     *
     * @return std::string The absolute path to the executable.
     * @throws std::runtime_error If the path cannot be retrieved.
     */
    inline std::string getProgramPath() {
    #if _WIN32
        char path[MAX_PATH];
        HMODULE hModule = GetModuleHandle(NULL);
        if (hModule != NULL) {
            DWORD len = GetModuleFileNameA(hModule, path, MAX_PATH);
            
            if (len != 0) {
                for(DWORD i=0; i<len; i++) if(path[i]=='\\') path[i] = '/';
                
                return std::string(path);
            }
            else {
                DWORD errorCode = GetLastError();
                std::stringstream ss;
                ss << "GetModuleFileNameA failed with error code: " << errorCode;
                throw std::runtime_error(ss.str());
            }
        }

        std::cout << "getProgramPath() error." << std::endl;
        return "";
    #else
        char path[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (len != -1) {
            path[len] = '\0';
            return std::string(path);
        }
        else {
            throw std::runtime_error(std::string("readlink failed with error code: ")+std::to_string(errno));
        }
        std::cout << "getProgramPath() error." << std::endl;
        return "";
    #endif
    }


    /**
     * @brief Checks if the given path is a directory.
     *
     * This function uses the stat system call to determine if the specified path refers to a directory.
     * 
     * @param _path The file system path to check.
     * @return int Returns 0 if the path is a directory, 1 if it is not a directory or does not exist,
     *             and -1 if an error occurred during the stat call (other than ENOENT or ENOTDIR).
     */
    inline int getPathIsDir(std::string _path) {
        struct stat info;

        int stat_ret = stat(_path.c_str(), &info);
        if(stat_ret != 0) {
            if(errno == ENOENT)         { return 1; }
            else if(errno == ENOTDIR)   { return 1; }
            return -1;
        } 

        return (info.st_mode & S_IFDIR)? 0 : 1;
    }


    /**
     * @brief Get the index of a desired value
     * 
     * @tparam varType value type
     * @param toCheck the container to check
     * @param toFind what to find.
     * ```
     * - `0` - biggest value
     * - `1` - smallest value
     * ```
     * @return size_t of the desired element index
     */
    template<typename varType>
    inline size_t findIdx(std::vector<varType> toCheck, int toFind) {
        size_t index = 0;
        for(size_t i=1; i<toCheck.size(); i++) {
            switch (toFind) {
            case 0: //max
                if(toCheck[i]>toCheck[index]) {index = i;}
                break;
            case 1: //min
                if(toCheck[i]<toCheck[index]) {index = i;}
                break;
            default:
                throw std::invalid_argument("findIdx: invalid `toFind` argument.");
                break;
            }
        }
        return index;
    }
    template<typename varType>
    inline size_t findIdx(std::initializer_list<varType> toCheck, int toFind) {
        return findIdx<varType>(toCheck, toFind);
    }


    inline size_t findSubstr(std::string _toFind, std::string _toSearch) {
        size_t size_toFind  = _toFind.size();
        size_t size_toSearch= _toSearch.size();
        if(size_toFind==0) throw std::runtime_error("findSubstr(std::string, std::string) arg for _toFind is empty, which is not allowed.");
        else if(size_toSearch==0) throw std::runtime_error("findSubstr(std::string, std::string) arg for _toSearch is empty, which is not allowed.");
        else if(size_toFind>size_toSearch) return std::string::npos; //throw std::runtime_error("findSubstr(std::string, std::string) string length of _toFind is bigger than _toSearch which is not allowed.");
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
    
    /// @brief replace every `{toReplace}` std::string in `{text}` with `{replaceTo}`
    /// @param text std::string to replace parts of
    /// @param toReplace old std::string that is to be removed/replaced
    /// @param replaceTo new std::string that will be inserted
    /// @return original `{text}` std::string but with `{toReplace}` replaced
    inline std::string replaceSubstr(std::string text, std::string toReplace, std::string replaceTo) {
        size_t count=0;
        size_t pos = text.find(toReplace);
        int replaceLen = toReplace.length();
        
        while(pos!=std::string::npos) {
            text.replace(pos, replaceLen, replaceTo);
            count+=pos;
            pos = text.find(toReplace, pos+1);
        }
        return text;
    }
    inline void replaceSubstr(std::string* text, std::string toReplace, std::string replaceTo) {
        size_t count=0;
        size_t pos = text->find(toReplace);
        int replaceLen = toReplace.length();
        
        while(pos!=std::string::npos) {
            text->replace(pos, replaceLen, replaceTo);
            count+=pos;
            pos = text->find(toReplace, pos+1);
        }
    }

    template<class T>
    inline std::string FormatWithSymbol(T value, std::string formatSymbol=" ") {
        std::stringstream ss;
        ss.imbue(std::locale(""));
        ss << std::fixed << value;
        return replaceSubstr(ss.str(), ",", formatSymbol);
    }

    inline std::string formatDate(std::chrono::system_clock::time_point _dateVar) {
        auto tEnd_time = std::chrono::system_clock::to_time_t(_dateVar);
        char* endTime_text = ctime(&tEnd_time);
        if (endTime_text[strlen(endTime_text)-1] == '\n') endTime_text[strlen(endTime_text)-1] = '\0';
        return std::string(endTime_text);
    }
    
    inline std::string formatDuration(std::chrono::duration<double> _duration) {
        std::stringstream fullString;
        const auto hrs = std::chrono::duration_cast<std::chrono::hours>(_duration);
        const auto mins = std::chrono::duration_cast<std::chrono::minutes>(_duration - hrs);
        const auto secs = std::chrono::duration_cast<std::chrono::seconds>(_duration - hrs - mins);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(_duration - hrs - mins - secs);

        if(hrs.count()>0)   fullString << hrs.count()   << " hours ";
        if(mins.count()>0)  fullString << mins.count()  << " minutes ";
        fullString << secs.count() << "." << ms.count() << " seconds";
        return fullString.str();
    }

    template<class _castType>
    inline std::string formatNumber(
        _castType   value,
        size_t      width       = 0,
        int         precision   = 1,
        std::string align       = "right",
        bool        numberFill  = false,
        bool        formatCommas= false
    ) {
        std::stringstream outStream, _temp;
        std::string _final;
        int fillZeros = 0;
        if(formatCommas) {
            outStream.imbue(std::locale(""));
        }
        if(numberFill && align=="right") {
            _temp << std::fixed;
            _temp << std::setprecision(precision) << value;
            if(static_cast<int>(_temp.str().length()) < width) fillZeros = width - static_cast<int>(_temp.str().length());
        }
        outStream << std::fixed;
        outStream << std::boolalpha;
        if(align=="left") outStream<<std::left;
        else if(align=="right") outStream<<std::right;
        outStream << std::setw(width - fillZeros);
        if(numberFill && align=="right") outStream << std::string(fillZeros, '0');
        // outStream << (align=="left"? std::left : std::right);
        outStream << std::setprecision(precision) << value;

        return outStream.str();
    }

    template<class T>
    inline std::string formatContainer(
        T _container,
        int strWidth,
        int varPrecision,
        std::string align = "right",
        bool numberFill = false,
        char openSymb   = '{',
        char closeSymb  = '}',
        char _sepSymb   = ','
    ) {
        std::string _out(1, openSymb);
        for(auto itr=_container.begin(); itr!=_container.end(); ++itr) {
            _out += formatNumber(*itr, strWidth, varPrecision, align, numberFill);
            if(itr!=--_container.end()) openSymb += _sepSymb;
        }
        return _out + closeSymb;
    }

    template<class T>
    inline std::string formatVector(
        std::vector<T>  _container,
        int             _strWidth   = 0,
        int             _precision  = 1,
        std::string     _align      = "right",
        bool            _numberFill = false,
        char            _openSymb   = '{',
        char            _closeSymb  = '}',
        char            _sepSymb    = ',',
        size_t          _startIdx   = 0,
        size_t          _endIdx     = std::string::npos
    ) {
        std::string _out(1, _openSymb);
        size_t endIdx = (_endIdx==std::string::npos? _container.size() : _endIdx);
        for(size_t i=_startIdx; i<endIdx; i++) {
            _out += formatNumber(_container[i], _strWidth, _precision, _align, _numberFill);
            if(i<endIdx-1) _out += _sepSymb;
        }
        return _out + _closeSymb;
    }

    template<class T>
    inline std::string formatContainer1(
        T _container,
        size_t contSize,
        int strWidth,
        int varPrecision,
        std::string align = "right",
        bool numberFill = false,
        char openSymb   = '{',
        char closeSymb  = '}',
        char _sepSymb   = ','
    ) {
        std::string _out(1, openSymb);
        for(size_t i=0; i<contSize; i++) {
            _out += formatNumber(_container[i], strWidth, varPrecision, align, numberFill);
            if(i<contSize-1) _out += _sepSymb;
        }
        return _out + closeSymb;
    }


    /**
     * @brief Search and find the vector index position of a certain value
     * 
     * @tparam T -data type of elements to look through
     * @param vec vector to search through
     * @param toFind value to find in the vector
     * @return int index of where on `vec` the given `toFind` value exists.
     * @note if the value is not found in the vector then the function will return -1
     */
    template<class T> inline int searchVec(std::vector<T> vec, T toFind) {
        typename std::vector<T>::iterator idx = find(vec.begin(), vec.end(), toFind);
        if(idx!=vec.end()) return idx-vec.begin();
        else return -1;
        // int idx = -1;
        // for(size_t i=0; i<vec.size(); i++) {
        // 	if(vec.at(i)==toFind) {
        //     	idx=i;
        //         break;
        //     }
        // }
        // return idx;
    }

    /// @brief Solve whether a number is positive or negative (same as int(var/abs(var)))
    /// @param var variable to check
    /// @return 1 if var>0; -1 if var<0; 0 if var==0
    inline int PoN(float var) {
        if(var>0) return 1;
        else if(var<0) return -1;
        else {
            return 1;
            std::cout << "PoN(): \"" << var << "\" is not a number\n";
            return 0;
        }
    }

    /**
     * @brief find desired value from vector
     * 
     * @param toCheck container of values to find the desired value of
     * @param toFind what to find:
     * - `0` - biggest value
     * - `1` - smallest value
     * - `2` - index of biggest value
     * - `3` - index of smallest value
     * @return float of the desired value
     */
    inline float findVal(std::vector<float> toCheck, int toFind) {
        int index = 0;
        float val = toCheck[0];
        for(int i=0; i<toCheck.size(); i++) {
            if(toFind==0 || toFind==2) {     if(toCheck[i]>val) { val=toCheck[i]; index=i; } }
            else if(toFind==1 || toFind==3){ if(toCheck[i]<val) { val=toCheck[i]; index=i; } }
        }
        if(toFind==0 || toFind==1) return val;
        else if(toFind==2 || toFind==3) return index;
        else return -1;
    }

    /**
     * @brief Check if two decimal number containers' elements/values are equal to a specific precision/decimal-numbers
     * 
     * @tparam _contType the type of variable/decimal-number containers
     * @param _var0 decimal number container 0
     * @param _var1 decimal number container 1
     * @param _contSize size of the decimal number containers
     * @param _precision the precision to check equality for.
     * @return true if the two containers have the same numbers for given precision.
     * @return false if there are non-same numbers.
     */
    template<class _contType>
    inline bool decimalSame(_contType _var0, _contType _var1, size_t _contSize, size_t _precision=6) {
        bool matched = true;
        for(size_t i=0; i<_contSize; i++) {
            if(roundf(_var0[i]*pow(10, _precision))/pow(10, _precision) != roundf(_var1[i]*pow(10, _precision))/pow(10, _precision)) {
                matched = false;
                break;
            }
        }
        return matched;
    }

    /**
     * @brief 
     * 
     * @tparam idx_varType type of the variable that's searched for
     * @tparam idx_type type of the return value, i.e. the value that is given by indexing `index_varType`
     * @param toCheck `std::vector<idx_varType>` container that is to be searched, i.e. the hay stack
     * @param toFind what to find:
     * - `0` - biggest value
     * - `1` - smallest value
     * - `2` - index of biggest value
     * - `3` - index of smallest value
     * @param idx index of the data type to search: `toCheck[element][idx]`
     * @return idx_varType of the returned type
     */
    template<typename idx_varType, typename idx_type>
    inline idx_type idx_findVal(
        std::vector<idx_varType> toCheck,
        int toFind,
        size_t idx
    ) {
        int index = 0;
        idx_type val = toCheck[0][idx];
        for(int i=0; i<toCheck.size(); i++) {
            if(toFind==0 || toFind==2)      { if(toCheck[i][idx]>val) { val=toCheck[i][idx]; index=i; } }
            else if(toFind==1 || toFind==3) { if(toCheck[i][idx]<val) { val=toCheck[i][idx]; index=i; } }
        }
        if(toFind==0 || toFind==1)      return val;
        else if(toFind==2 || toFind==3) return index;
        else return -1;
    }


    inline std::vector<int> convert_RGB_HSV(
        std::vector<int> _RGB
    ) {
        std::vector<float> RGB_p{
            static_cast<float>(_RGB[0])/255,
            static_cast<float>(_RGB[1])/255,
            static_cast<float>(_RGB[2])/255
        };
        std::vector<int> HSV(3, 0);
        size_t maxIdx = findIdx<float>(RGB_p, 0);
        size_t minIdx = findIdx<float>(RGB_p, 1);

        int delta = _RGB[maxIdx]-_RGB[minIdx];

        HSV[2] = static_cast<int>(100*RGB_p[maxIdx]);
        HSV[1] = static_cast<int>(100*(HSV[2]==0? 0 : delta/RGB_p[maxIdx]));
        switch (maxIdx) {
            case 0:
                HSV[0] = static_cast<int>(60*(delta==0? 0 : ((RGB_p[1]-RGB_p[2])/(delta)+0)));
                break;
            case 1:
                HSV[0] = static_cast<int>(60*(delta==0? 0 : ((RGB_p[2]-RGB_p[0])/(delta)+2)));
                break;
            case 2:
                HSV[0] = static_cast<int>(60*(delta==0? 0 : ((RGB_p[0]-RGB_p[1])/(delta)+4)));
                break;
        }
        if(HSV[0]<0) HSV[0]+=360;

        return HSV;
    }

    /**
     * @brief Convert HSV values to RGB values
     * 
     * @param HSV float values of HSV (Hue, Saturation, Value) (degrees, percentage, percentage) [0-360, 0-100, 0-100]
     * @return std::vector<float> of the HSV values [0-255]
     */
    inline std::vector<float> convert_HSV_RGB(
        std::vector<float> HSV
    ) {
        std::vector<float> _RGB(3, 0);
        std::vector<float> RGB_p(3, 0);
        std::vector<float> HSV_p{
            static_cast<float>(HSV[0]),
            static_cast<float>(HSV[1])/100,
            static_cast<float>(HSV[2])/100
        };

        float C = HSV_p[2] * HSV_p[1];
        float X = C * float(1 - abs(fmod(HSV_p[0]/60, 2) -1));
        float m = HSV_p[2] - C;

        // std::cout<<"{"<<C<<", "<<X<<", "<<m<<"}\n";

        if(HSV_p[0] < 60) {
            RGB_p[0] = C;
            RGB_p[1] = X;
            RGB_p[2] = 0;
        }
        else if(HSV_p[0] < 120) {
            RGB_p[0] = X;
            RGB_p[1] = C;
            RGB_p[2] = 0;
        }
        else if(HSV_p[0] < 180) {
            RGB_p[0] = 0;
            RGB_p[1] = C;
            RGB_p[2] = X;
        }
        else if(HSV_p[0] < 240) {
            RGB_p[0] = 0;
            RGB_p[1] = X;
            RGB_p[2] = C;
        }
        else if(HSV_p[0] < 300) {
            RGB_p[0] = X;
            RGB_p[1] = 0;
            RGB_p[2] = C;
        }
        else {
            RGB_p[0] = C;
            RGB_p[1] = 0;
            RGB_p[2] = X;
        }

        _RGB[0] = (RGB_p[0]+m)*static_cast<float>(255);
        _RGB[1] = (RGB_p[1]+m)*static_cast<float>(255);
        _RGB[2] = (RGB_p[2]+m)*static_cast<float>(255);

        return _RGB;
    }

    
    inline void printDirEntries(std::string _path) {
        std::cout << "======== dir entries ========" << std::endl;
        for(const auto& entry : std::filesystem::directory_iterator(_path)) {
            std::cout << formatNumber(entry.path(), 100, 1, "left") << " | ";
            std::cout << " size:" << formatNumber(HumanReadable{entry.file_size()}, 10, 1) << " ("<<formatNumber(HumanReadable{entry.file_size()}.size,10)<<")";
            std::cout << std::endl;
        }
        std::cout << "=============================" << std::endl;
    }


    /**
     * @brief Finds the index of the shortest or longest string in a vector of strings.
     *
     * This function searches through the provided vector of strings and returns the index
     * of either the shortest or longest string, depending on the value of the `id` parameter.
     *
     * @param stringVec The vector of strings to search.
     * @param id Selection mode: 
     *           - 0 to find the index of the shortest string.
     *           - 1 to find the index of the longest string.
     * @return The index of the selected string in the vector, or -1 if an invalid `id` is provided.
     *
     * @note If `id` is not 0 or 1, an error message is printed and -1 is returned.
     */
    inline int stringOfVector(std::vector<std::string> stringVec, int id) {
        int idx;
        std::vector<std::string>::iterator leWord;

        if(id == 0) {
            leWord = min_element(
                stringVec.begin(), stringVec.end(),
                [](const auto& a, const auto& b) {
                    return a.size() < b.size();
                });
        }
        else if(id == 1) {
            leWord = max_element(
                stringVec.begin(), stringVec.end(),
                [](const auto& a, const auto& b) {
                    return a.size() < b.size();
                });
        }
        else {
            std::cout << "ERROR: stringOfVector: wrong id selected. Exiting.." << std::endl;
            return -1;
        }
        // resultStrRef = *leWord;
        return distance(stringVec.begin(), leWord);
    }
    /**
     * @brief Splits a string into float values based on a delimiter.
     *
     * Parses the input string `line` using the specified `delimiter`, converts each substring
     * to a float, and stores the results in the provided `returnArr` array. The number of expected
     * float values is specified by `numVar`. Optionally, prints debug information if `printVar` is true.
     *
     * @param line The input string to split and parse.
     * @param delimiter The delimiter used to split the string.
     * @param returnArr Array to store the parsed float values.
     * @param numVar Number of expected float values (default is 4).
     * @param printVar If true, prints debug information (default is false).
     */
    inline void splitString(
        std::string line,
        std::string delimiter,
        float returnArr[],
        int numVar=4,
        bool printVar=false
    ) {
        if(printVar) std::cout << "--- \"" << line << "\"\n";
        size_t pos = 0;
        for(int i=0; i<numVar; i++) {
            if(i<(numVar-1)) pos = line.find(delimiter);
            if(printVar) std::cout << "- pos:" << pos << " :" << line.substr(0, pos) << "\n";
            returnArr[i] = stof(line.substr(0, pos));
            line.erase(0, pos + delimiter.length());
        }
        if(printVar) std::cout << "---";
    }

    /**
     * @brief Splits a string into a vector of substrings based on a specified delimiter.
     *
     * This function takes an input string and splits it into substrings wherever the delimiter occurs.
     * Optionally, it can print debug information and throw an error if the delimiter is not found.
     *
     * @param line The input string to be split.
     * @param delimiter The delimiter string used to split the input.
     * @param printVar If true, prints debug information about the splitting process. Default is false.
     * @param causeError If true, throws a runtime error if the delimiter is not found in the input string. Default is false.
     * @return std::vector<std::string> A vector containing the split substrings.
     * @throws std::runtime_error If causeError is true and the delimiter is not found in the input string.
     */
    inline std::vector<std::string> splitString(
        std::string line,
        std::string delimiter,
        bool printVar   = false,
        bool causeError = false
    ) {
        if(printVar) std::cout << "--- \"" << line << "\"\n";
        /// `std::vector<std::string>` of the strings containing the results
        std::vector<std::string> resultStrings;
        size_t idx0 = 0;
        size_t idx1 = line.find(delimiter, idx0);

        if(idx1==std::string::npos) {
            resultStrings.push_back(line);
            if(causeError) throw std::runtime_error("std::vector<std::string> splitString(std::string, std::string, bool, bool) delimiter doesn't exist in `line`");
            if(printVar) std::cout << "error: splitString: no delimiter \"" << delimiter << "\" found. Returning empty vector\n";
            return resultStrings;
        }

        while(true) {
            resultStrings.push_back(line.substr(idx0, idx1-idx0));
            if(idx1==std::string::npos) break;
            for(;line.substr(idx1+delimiter.length(), delimiter.length())==delimiter; idx1+=delimiter.length());
            idx0 = idx1+delimiter.length();
            if(idx0==line.length()) break;
            idx1 = line.find(delimiter, idx0);
        }

        if(printVar) std::cout << "---";
        return resultStrings;
    }
    /**
     * @brief Splits a string into substrings based on a specified delimiter.
     *
     * This function clears the provided vector and fills it with substrings
     * obtained by splitting the input string using the given delimiter.
     * Optionally, it can print the variable contents if requested.
     *
     * @param line The input string to be split.
     * @param delimiter The delimiter string used to split the input.
     * @param returnVec Reference to a vector where the resulting substrings will be stored.
     * @param printVar If true, prints the variable contents (default: false).
     */
    inline void splitString(
        std::string line,
        std::string delimiter,
        std::vector<std::string> &returnVec,
        bool printVar=false
    ) {
        returnVec.clear();
        returnVec = splitString(line,delimiter,printVar);
    }
    /**
     * @brief Splits a string into substrings based on a specified delimiter and stores the result in a provided vector pointer.
     *
     * This function clears the contents of the vector pointed to by `returnPtr`, then splits the input `line` using the given
     * `delimiter`, and stores the resulting substrings in the vector. Optionally, it can print the variable if `printVar` is set to true.
     *
     * @param line The input string to be split.
     * @param delimiter The delimiter string used to split the input.
     * @param returnPtr Pointer to a vector where the resulting substrings will be stored.
     * @param printVar If true, prints the variable (default is false).
     */
    inline void splitString(
        std::string line,
        std::string delimiter,
        std::vector<std::string> *returnPtr,
        bool printVar=false
    ) {
        returnPtr->clear();
        (*returnPtr) = splitString(line,delimiter,printVar);
    }

    
    inline void clearScreen(int pos_X=0, int pos_Y=0) {
        std::string ansiCode = "\x1B[";

        std::cout<<ansiCode<<"2J";
        std::cout<<ansiCode<<pos_Y<<";"<<pos_X<<"H";
        std::cout.flush();
    }

    /**
     * @brief Prints text to the console at a specified position using ANSI escape codes.
     *
     * This function moves the cursor to the given (posX, posY) position and prints the provided text.
     * Supports both absolute and relative positioning, optional screen clearing, and handles multi-line text.
     *
     * @param posX        The X (column) position to print the text. Interpreted as absolute or relative based on x_method.
     * @param posY        The Y (row) position to print the text. Interpreted as absolute or relative based on y_method.
     * @param printText   The text to print. Supports multi-line text separated by '\n'.
     * @param flushEnd    If true, flushes the output stream after printing. Default is true.
     * @param x_method    Positioning method for X: "abs" for absolute, "rel" for relative to previous position. Default is "abs".
     * @param y_method    Positioning method for Y: "abs" for absolute, "rel" for relative to previous position. Default is "abs".
     * @param initClearScr If true, clears the screen before printing. Default is false.
     */
    inline void ANSI_mvprint(
        int posX,
        int posY,
        std::string printText,
        bool flushEnd = true,
        std::string x_method = "abs",
        std::string y_method = "abs",
        bool initClearScr = false
    ) {
        static int prevPos[2] = {0, 0};

        if(x_method=="rel") posX = prevPos[0]+posX;
        if(y_method=="rel") posY = prevPos[1]+posY;

        std::string ansiCode = "\x1B[";

        if(initClearScr) {
            std::cout<<ansiCode<<"2J";
            std::cout.flush();
        }

        size_t tPos=0, ePos=0;
        int rowCount= 0;
        int rowLen  = 0;
        std::string _substring = "";
        while(printText.find('\n', tPos)!=std::string::npos) {
            ePos = printText.find('\n', tPos);
            _substring = printText.substr(tPos, ePos-tPos);
            std::cout<<ansiCode<<posY+rowCount<<";"<<posX<<"H"<<_substring;
            tPos = ePos+1;
            rowCount++;
        }
        _substring = printText.substr(tPos, printText.length());
        std::cout<<ansiCode<<posY+rowCount<<";"<<posX<<"H"<<_substring;
        rowCount++;

        // std::cout<<ansiCode<<posY<<";"<<posX<<"H"<<printText;
        if(flushEnd) std::cout.flush();
        prevPos[0] = posX + _substring.length();
        prevPos[1] = posY + rowCount;
    }

    /**
     *
     * @brief Prints a vector of strings to the terminal at a specified position using ANSI escape codes.
     * This function prints each string in `textVec` at consecutive lines starting from the position (`posX`, `posY`)
     * in the terminal. It uses ANSI escape codes to position the cursor and optionally clear the screen.
     * The function also handles terminal resizing by clearing the screen if the terminal dimensions change.
     *
     * @param textVec Vector of strings to print.
     * @param posX Horizontal position (column) to start printing.
     * @param posY Vertical position (row) to start printing.
     * @param flushEnd If true, flushes the output stream after printing. Default is true.
     * @param clearScr If true, clears the screen before printing. Default is false.
     * @param end String to print at the end (e.g., newline). Default is "\n".
     * @param initClear If true, clears the screen on the first call. Default is true.
     */
    inline void baseAnsiPrint(
        std::vector<std::string> textVec,
        int posX,
        int posY,
        bool flushEnd = true,
        bool clearScr = false,
        std::string end = "\n",
        bool initClear = true
    ) {
        int terminalDim[2];
        static int oldTermDim[2];
        static bool funcInit = false;
        std::string ansiCode = "\x1B[";

        getTermSize(terminalDim[0], terminalDim[1]);

        if(!funcInit) {
            oldTermDim[0] = terminalDim[0];
            oldTermDim[1] = terminalDim[1];
            if(initClear) {
                std::cout << ansiCode+"2J";
                funcInit = true;
            }
        }
        if(terminalDim[0]!=oldTermDim[0] || terminalDim[1]!=oldTermDim[1]) {
            std::cout << ansiCode+"2J";
            oldTermDim[0] = terminalDim[0];
            oldTermDim[1] = terminalDim[1];
        }

        if(clearScr) std::cout << ansiCode+"2J";
        for(int i=0; i<static_cast<int>(textVec.size()); i++) {
            std::cout << ansiCode+std::to_string(posY+i)+";"+std::to_string(posX)+"H"+textVec[i];
        }
        std::cout << end;
        if(flushEnd) std::cout.flush();
    }


    /**
     * @brief Prints text to the terminal using ANSI escape codes for positioning and formatting.
     *
     * This function allows printing a string to a specified position in the terminal,
     * optionally clearing the screen before printing, and flushing the output at the end.
     * The text can be split into lines using a specified delimiter.
     *
     * @param text The text to print.
     * @param posX The column position to start printing (default is 0).
     * @param posY The row position to start printing (default is 0).
     * @param flushEnd If true, flushes the output stream after printing (default is true).
     * @param clearScr If true, clears the terminal screen before printing (default is false).
     * @param textDelim The delimiter used to split the text into lines (default is "\n").
     * @param end The string to print at the end (default is "\n").
     */
    inline void ansiPrint(
        std::string text,
        int posX = 0,
        int posY = 0,
        bool flushEnd = true,
        bool clearScr = false,
        std::string textDelim = "\n",
        std::string end = "\n"
    ) {
        int printPos[2] = {0, 0};
        int maxRowLen = 0;

        /// @brief vector to hold each line of `text`
        std::vector<std::string> lines = splitString(text,textDelim,false);
        
        baseAnsiPrint(lines,posX,posY,flushEnd,clearScr,end);
    }

    /**
     * @brief Prints text to the terminal at a scaled position using ANSI escape codes.
     *
     * This function splits the input text into lines, calculates the position to print
     * based on the terminal size and scaling factors, and then prints the text at the
     * calculated position. It supports optional screen clearing, custom line delimiters,
     * and flushing behavior.
     *
     * @param text The text to print.
     * @param scalX Horizontal scaling factor (0.0 to 1.0) for print position relative to terminal width.
     * @param scalY Vertical scaling factor (0.0 to 1.0) for print position relative to terminal height.
     * @param flushEnd If true, flushes the output stream after printing.
     * @param clearScr If true, clears the screen before printing.
     * @param textDelim Delimiter used to split the text into lines (default: "\n").
     * @param end String appended at the end of the printed text (default: "\n").
     * @param initClear If true, performs initial screen clear before printing.
     */
    inline void ansiPrint(
        std::string text,
        float scalX = 0,
        float scalY = 0,
        bool flushEnd = true,
        bool clearScr = false,
        std::string textDelim = "\n",
        std::string end = "\n",
        bool initClear = true
    ) {
        
        int terminalDim[2] = {0, 0};
        int printPos[2] = {0, 0};
        int textDim[2] = {0, 0};

        /// @brief vector to hold each line of `text`
        std::vector<std::string> lines = splitString(text,textDelim,false);
        getTermSize(terminalDim[0], terminalDim[1]);
        textDim[0] = lines[stringOfVector(lines,1)].length();
        textDim[1] = static_cast<int>(lines.size());
        printPos[0] = static_cast<int>(round(static_cast<float>(terminalDim[0])*scalX));
        printPos[1] = static_cast<int>(round(static_cast<float>(terminalDim[1])*scalY));
        if(printPos[0]+textDim[0]>terminalDim[0]) printPos[0]+=(terminalDim[0]-(printPos[0]+textDim[0]));
        if(printPos[1]+textDim[1]>terminalDim[1]) printPos[1]+=(terminalDim[1]-(printPos[1]+textDim[1]));
        
        baseAnsiPrint(lines,printPos[0],printPos[1],flushEnd,clearScr,end,initClear);
    }

    /**
     * @brief ANSI print with side alignment
     * @param text text/paragraph to print on terminal
     * @param xAlign x-axis side alignment:
     * options: {`"left"`, `"right"`}
     * @param yAlign y-axis side alignment:
     * options: {`"top"`, `"bottom"`}
     * @param flushEnd whether to flush `std::cout` after printing
     * @param clearScr whether to clear screen before printing to terminal
     * @param textDelim delimiter std::string to indicate where to split `text` into separate lines/rows
     * @param end std::string to print at the end
    */
    inline void ansiPrint(
        std::string text,
        std::string xAlign,
        std::string yAlign,
        bool flushEnd = true,
        bool clearScr = false,
        std::string textDelim = "\n",
        std::string end = "\n"
    ) {
        float axisScal[2] = {0, 0};
        if(xAlign=="left") axisScal[0] = 0;
        else if(xAlign=="right") axisScal[0] = 1;
        else {
            std::cout << "ERROR: ansiPrint(): wrong xAlign parameter input. Exiting.."<<std::endl;
            return;
        }
        if(yAlign=="top") axisScal[1] = 0;
        else if(yAlign=="bottom") axisScal[1] = 1;
        else {
            std::cout << "ERROR: ansiPrint(): wrong yAlign parameter input. Exiting.."<<std::endl;
            return;
        }
        ansiPrint(text,axisScal[0],axisScal[1],flushEnd,clearScr,textDelim, end);
    }


    /**
     * @brief Prints a formatted value to the standard output with customizable options.
     *
     * This function formats the given value according to the specified width, precision,
     * alignment, and other options, then prints it to the console. It can handle cases
     * where the formatted string exceeds the specified width by either truncating,
     * splitting, or leaving it as is, based on the method_widthOver parameter.
     *
     * @tparam _castType The type of the value to print.
     * @param value The value to be printed.
     * @param width The minimum width of the output (default: std::string::npos for no limit).
     * @param align Alignment of the output ("right" or "left", default: "right").
     * @param end The string to append at the end of each line (default: "\n").
     * @param flushEnd If true, flushes the output stream after printing (default: true).
     * @param flushBeginning If true, flushes the output stream before printing (default: false).
     * @param numberFill If true, fills the output with zeros (default: false).
     * @param precision The number of decimal places for floating-point values (default: 1).
     * @param method_widthOver Determines behavior when output exceeds width:
     *        0 - Do nothing,
     *        1 - Truncate,
     *        2 - Split into multiple lines (default: 1).
     * @return int Returns 0 on success, 1 on invalid method_widthOver value.
     */
    template<typename _castType>
    inline int PrintOut(
        _castType   value,
        size_t      width           = std::string::npos,
        std::string align           = "left",
        std::string end             = "\n",
        bool        flushEnd        = true,
        bool        flushBeginning  = false,
        bool        numberFill      = false,
        int         precision       = 1,
        size_t      method_widthOver= 1,
        Pos2d<size_t>* cursorPos= nullptr
    ) {
        std::string initString = "[" + getCurrentTime()+"]: ";
        std::vector<std::string> formattedString;
        std::string rawString = formatNumber(value, (width==std::string::npos? 0 : width-initString.size()), precision, align, numberFill);

        if((rawString.size()+initString.size()) > width) {
            switch (method_widthOver) {
                case 0: //Do nothing
                    formattedString.push_back(rawString);
                    break;
                case 1: //Cut and delete the extra
                    formattedString.push_back(rawString.substr(0, width));
                    break;
                case 2: //Cut and move the extra to new elements
                    do {
                        formattedString.push_back(rawString.substr(0, width));
                        rawString.erase(0, width);
                    } while (rawString.size()>width);
                    break;
                default:
                    return 1;
                    break;
            }
        }
        else {
            formattedString.push_back(rawString);
        }
        
        if(cursorPos) Useful::ANSI_mvprint(cursorPos->x, cursorPos->y, "", false);

        if(flushBeginning) std::cout.flush();
        for(size_t i=0; i<formattedString.size(); i++) {
            switch (i) {
                case 0:
                    std::cout << initString;
                    break;
                default:
                    std::cout << std::string(initString.size(), ' ');
                    break;
            }
            std::cout << formattedString.at(i);
            std::cout << end;
        }


        if(cursorPos && formattedString.size()>0) {
            if(end=="\n") {
                cursorPos->operator[](1) += formattedString.size();
                cursorPos->operator[](0) = 0;
            }
            else {
                (*cursorPos).x += initString.size();//????
                for(std::string str : formattedString) cursorPos->operator[](0) += str.size() + end.size();
            }
        }
        if(flushEnd) std::cout.flush();
        return 0;
    }


    /**
     * @brief Converts a tm date structure to a formatted string.
     *
     * The resulting string is in the format "DD-MM-YYYY", where:
     * - DD is the day of the month (2 digits, zero-padded if necessary)
     * - MM is the month (2 digits, zero-padded if necessary; note: tm_mon is zero-based)
     * - YYYY is the year (4 digits, zero-padded if necessary; note: tm_year is years since 1900)
     *
     * @param _date The tm structure representing the date.
     * @return std::string The formatted date string.
     */
    inline std::string dateToStr(tm _date) {
        return (
            formatNumber(_date.tm_mday, 2, 0, "right", true) + "-" +
            formatNumber(_date.tm_mon , 2, 0, "right", true) + "-" +
            formatNumber(_date.tm_year, 4, 0, "right", true)
        );
    } 


    inline std::string basicProgressBar(
        double  _progress,
        double  _totalProgress,
        size_t  _funcStatus, // 0-init, 1-running
        size_t  _barWidth_numChars = 100,
        double* _speed = nullptr,
        std::chrono::duration<double>*  _ETA_seconds = nullptr,
        std::chrono::duration<double>   _update_interval = std::chrono::duration<double>(0.1),
        size_t  _symbol_ID_progr = 3,
        std::string _symbol_empty = " "
    ) {
        if(_totalProgress==0) throw std::runtime_error("ERROR: basicProgressBar() value for _totalProgress cannot be 0.");
        if(_barWidth_numChars<1) throw std::runtime_error("ERROR: basicProgressBar() value for _barWidth_numChars cannot be <1.");
        if(_progress==0) _funcStatus = 0;

        const std::vector<std::string> symb_progr{"■", "⬛", "▉", "▉", "█", "O"};
        const std::vector<char> intr{'|', '/', '-', '\\'};
        
        double scalar = _progress/_totalProgress;
        
        static size_t cnt_intr = 0;
        static double prev_progress = 0;
        static std::string returBar = "";
        static std::chrono::system_clock::time_point absTime_start  = std::chrono::system_clock::now();
        static std::chrono::steady_clock::time_point relTime_prev   = std::chrono::steady_clock::now();
        
        if(cnt_intr>=intr.size()) cnt_intr = 0;
        std::chrono::steady_clock::time_point relTime_curr = std::chrono::steady_clock::now();

        if(_funcStatus==0) {
            absTime_start   = std::chrono::system_clock::now();
            relTime_prev    = std::chrono::steady_clock::now();
        }

        auto interval_dur = std::chrono::duration<double>(relTime_curr-relTime_prev);
        // Useful::ANSI_mvprint(100, 0, Useful::formatDuration(interval_dur));
        if(_funcStatus!=0 && interval_dur<_update_interval) return returBar;
        
        returBar = "|";
        for(size_t i=0; i<std::floor(scalar*_barWidth_numChars); i++) returBar += symb_progr.at(_symbol_ID_progr);
        returBar+= intr.at(cnt_intr);
        for(size_t i=0; i<std::ceil(((_totalProgress-_progress)/_totalProgress)*(_barWidth_numChars-1)); i++) returBar += _symbol_empty;
        returBar+= "|";

        double delta_progress = _progress-prev_progress;

        if(_speed) (*_speed) = delta_progress/interval_dur.count();
        if(_ETA_seconds) (*_ETA_seconds) = std::chrono::duration<double>((_totalProgress-_progress) / (delta_progress/interval_dur.count()));

        cnt_intr++;
        relTime_prev = relTime_curr;
        prev_progress = _progress;
        return returBar;
    }
    
    /// @brief create a progress bar on terminal
    /// @param progress current progress made
    /// @param total_val total progress which reesembles 100&
    /// @param printBar whether to print the progress bar
    /// @param progFin whether the progress is finished (used to get total duration)
    /// @param interval the time interval between each progressBar update
    /// @param symbolIndex what symbol to use {"■", "⬛", "▉", "▉", "█"}
    /// @return std::string of the progressbar
    inline std::string progressBar(
        float progress,
        float total_val,
        bool printBar   = true,
        bool progFin    = false,
        float interval  = 0.1,
        int symbolIndex = 3
    ) {
        static int symbIdx = 2;
        symbIdx = symbolIndex;
        static std::vector<std::string> symb{"■", "⬛", "▉", "▉", "█", "O"};
        static char intr[] = {'|', '/', '-', '\\'};
        static int prev_intrCount = 0;
        static bool func_initd = false; // whether the function progressBar has initialised.f
        static bool func_ended = true;
        static int total_progLen;
        static double speed, percent = 0;

        static float prev_progress = 0;
        static auto prevTime = std::chrono::steady_clock::now();
        static auto start_time = std::chrono::system_clock::now();
        static auto abs_startTime = std::chrono::system_clock::now();
        static float progressDiff = 0;

        static float filterVal = 0.1;

        static int total_val_sanityCheck = 0;

        std::stringstream fullString;
        static std::string init_string = "";
        static std::string prev_fullString = "";

        if(!func_initd) total_val_sanityCheck = int(total_val);

        percent = progress/total_val*100;

        auto currTime = std::chrono::steady_clock::now();
        auto currTime_abs = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currTime-prevTime);
        if(!progFin && float(elapsed.count()/float(1'000'000)) < interval) return prev_fullString;

        speed = filterVal*(progress-prev_progress)/(elapsed.count()/double(1'000'000)) + (1.0-filterVal)*speed;
        progressDiff = float(0.5)*(progress-prev_progress)+float(0.5)*progressDiff;
        prevTime = currTime;
        prev_progress = progress;

        if(!func_initd) {
            abs_startTime = std::chrono::system_clock::now();
            time_t tStart_time = std::chrono::system_clock::to_time_t(abs_startTime);
            char* startTime_text = ctime(&tStart_time);
            if (startTime_text[strlen(startTime_text)-1] == '\n') startTime_text[strlen(startTime_text)-1] = '\0';
            fullString << "Start time: [" << startTime_text << "]" << "\n";
            init_string = fullString.str();
            total_progLen = FormatWithSymbol(int(total_val), "'").length();
            func_initd = true;
        }
        else {
            fullString << init_string;
        }
        if(int(total_val) != total_val_sanityCheck) {
            init_string= "";
            func_initd = false;
        }


        std::string prog_formatted = FormatWithSymbol(int(progress), "'");
        std::string emptSpac(total_progLen-prog_formatted.length(), ' ');
        std::string totalStr = " progress: "+emptSpac+prog_formatted+" / "+Useful::formatNumber(FormatWithSymbol(int(total_val), "'"),total_progLen,0)+" : ";

        std::string percent_formatted = formatNumber(percent,2);
        std::string emptSpac2(6-percent_formatted.length(), ' ');
        totalStr += emptSpac2+percent_formatted+"% ";

        std::string progBars="";
        for(int i=0; i<=int(floor(percent)); i++) {
            progBars+=symb[symbIdx];
        }

        std::string progressStr;
        if(int(floor(percent))<100) progressStr = progBars+intr[prev_intrCount];
        else progressStr = progBars;
        prev_intrCount+=1;
        if(prev_intrCount>=4) prev_intrCount=0;

        std::string emptSpac3(100-int(floor(percent)), ' ');
        totalStr += "|"+progressStr+emptSpac3+"|: ";

        std::string speed_formatted = formatNumber(speed, 5, 3);
        std::string emptSpac4(6-speed_formatted.length(), ' ');
        totalStr += emptSpac4+speed_formatted+"pt/s ";

        // double ETA_seconds = double(total_val-progress)/speed;
        double ETA_seconds = double(total_val-progress)/(double(progress)/std::chrono::duration<double>(currTime_abs-abs_startTime).count());
        totalStr += " ETA: ";
		totalStr += Useful::formatDuration(std::chrono::duration<double>(ETA_seconds));

        fullString << totalStr.c_str();
        // if(printBar) printf(" %s\r", totalStr.c_str());

        if(progFin) {
            auto tEnd_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char* endTime_text = ctime(&tEnd_time);

            if (endTime_text[strlen(endTime_text)-1] == '\n') endTime_text[strlen(endTime_text)-1] = '\0';
            fullString << "\nEnd time  : [" << endTime_text << "]\n";

            std::chrono::duration<double> elapsedTime = std::chrono::system_clock::now()-abs_startTime;
            fullString << "total elapsed time: " + Useful::formatDuration(elapsedTime);
        }
        
        return fullString.str();
    }

}

#endif //HPP_USEFUL