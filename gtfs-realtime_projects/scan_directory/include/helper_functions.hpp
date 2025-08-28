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

#endif //HPP_HELPER_FUNCTIONS