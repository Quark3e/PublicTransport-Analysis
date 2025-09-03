#pragma once
#ifndef HPP_HELPER_FUNCTIONS
#define HPP_HELPER_FUNCTIONS

#include <includes.hpp>


/**
 * @brief Recursively searches directories up to a specified depth and stores the results.
 *
 * Traverses the directory tree starting from the given path, searching up to _maxDepth levels deep.
 * Stores the paths of found directories or files in the provided list. Optionally tracks the number
 * of directories searched.
 *
 * @param _dirPath The starting directory path for the search.
 * @param _storeResult Pointer to a list where the found directory or file paths will be stored.
 * @param _maxDepth The maximum depth to search within the directory tree.
 * @param _numDirSearched Optional pointer to a variable that will be incremented with the number of directories searched.
 */
void func_depthSearch(std::string _dirPath, std::list<std::string> *_storeResult, int _maxDepth, size_t *_numDirSearched=nullptr);

int64_t parse_epochTime_fromFilename(std::string _toParse);

template <typename T>
struct atomwrapper
{
  std::atomic<T> _a;

  atomwrapper()
    :_a()
  {}

  atomwrapper(const std::atomic<T> &a)
    :_a(a.load())
  {}

  atomwrapper(const atomwrapper &other)
    :_a(other._a.load())
  {}

  atomwrapper &operator=(const atomwrapper &other)
  {
    _a.store(other._a.load());
  }
};


#include <initializer_list>

namespace ThrdPrf {

    struct __element__threadPerf {
        std::string label;
        double multiplier;

        __element__threadPerf(std::string _label, double _multiplier=1): label(_label), multiplier(_multiplier) {
            refrTimePoint = std::chrono::steady_clock::now();
        }

        std::chrono::steady_clock::time_point refrTimePoint;
        std::vector<std::chrono::duration<double>> durations;
        
    };

    class ThreadPerf {
        private:

        std::vector<__element__threadPerf> perf_objects;
        size_t numStored_durations;

        public:

        ThreadPerf() {}
        ThreadPerf(size_t _numStored_durations): numStored_durations(_numStored_durations) {}
        ThreadPerf(size_t _numStored_durations, std::initializer_list<__element__threadPerf> _perf_elements): numStored_durations(_numStored_durations), perf_objects(_perf_elements) {
            for(size_t i=0; i<perf_objects.size(); i++) {
                for(size_t j=i+1; j<perf_objects.size(); i++) {
                    if(perf_objects.at(i).label==perf_objects.at(j).label) 
                        throw std::runtime_error("ThreadPerf(size_t, std::initializer_list<__element__threadPerf>): ERROR: label duplicate found for labels at initializer indices ["+std::to_string(i)+"] and ["+std::to_string(j)+"]");
                }
            }
        }
        ThreadPerf(size_t _numStored_durations, std::initializer_list<std::string> _perf_element_labels): numStored_durations(_numStored_durations) {
            auto itr_i = _perf_element_labels.begin();
            for(size_t i=0; i<_perf_element_labels.size()-1; i++) {
                auto itr_j = _perf_element_labels.begin();
                std::advance(itr_j, i+1);
                for(size_t j=i+1; j<_perf_element_labels.size(); j++) {
                    if((*itr_i).compare(*itr_j)==0) {
                        throw std::runtime_error("ThreadPerf(size_t, std::initializer_list<std::string>): ERROR: label duplicate found for labels at initializer indices ["+std::to_string(i)+"] and ["+std::to_string(j)+"]");
                    }
                    if(j+1<_perf_element_labels.size()) std::advance(itr_j, 1);
                }
                std::advance(itr_i, 1);
            }

            for(auto el_label : _perf_element_labels) {
                perf_objects.push_back(el_label);
            }
        }

        ThreadPerf(const ThreadPerf& _copy) {
            perf_objects = _copy.perf_objects;
            numStored_durations = _copy.numStored_durations;
        }
        ~ThreadPerf() {

        }

        __element__threadPerf &operator[](size_t _i) {
            return perf_objects.at(_i);
        }
        __element__threadPerf operator[](size_t _i) const {
            return perf_objects.at(_i);
        }


        int set_T_start(std::string _label) {
            bool objFound = false;
            for(size_t i=0; i<perf_objects.size(); i++) {
                if(perf_objects.at(i).label==_label) {
                    perf_objects.at(i).refrTimePoint = std::chrono::steady_clock::now();
                    objFound = true;
                    break;
                }
            }
            if(!objFound) {
                perf_objects.push_back({_label});
                return 1;
            }

            return 0;
        }
        __element__threadPerf &set_T_end(std::string _label) {
            auto currTime = std::chrono::steady_clock::now();
            for(size_t i=0; i<perf_objects.size(); i++) {
                if(perf_objects.at(i).label==_label) {
                    if(numStored_durations==0 || perf_objects.at(i).durations.size()<numStored_durations) {
                        perf_objects.at(i).durations.push_back(currTime - perf_objects.at(i).refrTimePoint);
                    }
                    else {
                        std::rotate(perf_objects.at(i).durations.begin(), perf_objects.at(i).durations.begin()+1, perf_objects.at(i).durations.end());
                        perf_objects.at(i).durations.back() = currTime - perf_objects.at(i).refrTimePoint;
                    }
                    return perf_objects.at(i);
                }
            }
            throw std::runtime_error("ThreadPerf::set_T_end(std::string): _label arg \""+_label+"\" not found.");
        }

        std::chrono::duration<double> get_duration(std::string _label) {
            bool objFound = false;
            for(size_t i=0; i<perf_objects.size(); i++) {
                if(perf_objects.at(i).label==_label) {
                    if(perf_objects.at(i).durations.size()==0) throw std::runtime_error("ThreadPerf::getDuration(std::string): _label arg \""+_label+"\" has no recorded durations.");
                    return perf_objects.at(i).durations.back();
                }
            }
            throw std::runtime_error("ThreadPerf::getDuration(std::string): _label arg \""+_label+"\" not found.");
        }
        std::chrono::duration<double> get_duration(size_t _i) {
            if(perf_objects.at(_i).durations.size()==0) throw std::runtime_error("ThreadPerf::getDuration(size_t): index arg ["+std::to_string(_i)+"] has no recorded durations.");
            return perf_objects.at(_i).durations.back();
        }
        std::vector<std::chrono::duration<double>> get_allDurations(std::string _label) {
            bool objFound = false;
            for(size_t i=0; i<perf_objects.size(); i++) {
                if(perf_objects.at(i).label==_label) {
                    if(perf_objects.at(i).durations.size()==0) throw std::runtime_error("ThreadPerf::getAllDurations(std::string): _label arg \""+_label+"\" has no recorded durations.");
                    return perf_objects.at(i).durations;
                }
            }
            throw std::runtime_error("ThreadPerf::getAllDurations(std::string): _label arg \""+_label+"\" not found.");
        }
        std::vector<std::chrono::duration<double>> get_allDurations(size_t _i) {
            if(perf_objects.at(_i).durations.size()==0) throw std::runtime_error("ThreadPerf::getAllDurations(size_t): index arg ["+std::to_string(_i)+"] has no recorded durations.");
            return perf_objects.at(_i).durations;
        }
        std::chrono::duration<double> get_avgDuration(std::string _label) {
            bool objFound = false;
            for(size_t i=0; i<perf_objects.size(); i++) {
                if(perf_objects.at(i).label==_label) {
                    if(perf_objects.at(i).durations.size()==0) throw std::runtime_error("ThreadPerf::getAvgDuration(std::string): _label arg \""+_label+"\" has no recorded durations.");
                    std::chrono::duration<double> totalDur(0);
                    for(auto dur : perf_objects.at(i).durations) totalDur += dur;
                    return (totalDur / perf_objects.at(i).durations.size());
                }
            }
            throw std::runtime_error("ThreadPerf::getAvgDuration(std::string): _label arg \""+_label+"\" not found.");
        }
        std::chrono::duration<double> get_avgDuration(size_t _i) {
            if(perf_objects.at(_i).durations.size()==0) throw std::runtime_error("ThreadPerf::getAvgDuration(size_t): index arg ["+std::to_string(_i)+"] has no recorded durations.");
            std::chrono::duration<double> totalDur(0);
            for(auto dur : perf_objects.at(_i).durations) totalDur += dur;
            return (totalDur / perf_objects.at(_i).durations.size());
        }

        size_t size() const {
            return perf_objects.size();
        }

        std::string get_label(size_t _i) const {
            return perf_objects.at(_i).label;
        }

        size_t get_durationsSizeLim() const {
            return numStored_durations;
        }
        size_t set_durationsSizeLim(size_t _newDurationsSize) {
            for(auto& threadObj_el : perf_objects) {
                if(threadObj_el.durations.size() > _newDurationsSize) {
                    auto itr__el_dur = threadObj_el.durations.begin();
                    std::advance(itr__el_dur, threadObj_el.durations.size()-_newDurationsSize);
                    threadObj_el.durations.erase(threadObj_el.durations.begin(), itr__el_dur);
                }
            }

            return numStored_durations;
        }

    };



}


#endif //HPP_HELPER_FUNCTIONS