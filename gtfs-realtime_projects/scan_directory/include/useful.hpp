#pragma once
#ifndef HPP_USEFUL
#define HPP_USEFUL


#include <vector>
#include <string>

#include <cmath>
#include <algorithm>

#include <sstream>
#include <iomanip>
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
namespace fs = std::filesystem;


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


    template<class _castType>
    inline std::string formatNumber(
        _castType   value,
        size_t      width       = 0,
        int         precision   = 1,
        std::string align       = "right",
        bool        numberFill  = false
    ) {
        std::stringstream outStream, _temp;
        std::string _final;
        int fillZeros = 0;
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
        for(const auto& entry : fs::directory_iterator(_path)) {
            std::cout << formatNumber(entry.path(), 100, 1, "left") << " | ";
            std::cout << " size:" << formatNumber(HumanReadable{entry.file_size()}, 10, 1) << " ("<<formatNumber(HumanReadable{entry.file_size()}.size,10)<<")";
            std::cout << std::endl;
        }
        std::cout << "=============================" << std::endl;
    }


}

#endif //HPP_USEFUL