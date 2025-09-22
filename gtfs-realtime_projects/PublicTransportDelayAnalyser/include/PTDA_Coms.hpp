#pragma once
#ifndef HPP__PTDA_Coms
#define HPP__PTDA_Coms

#include <sstream>
#include <chrono>


struct PTDA_ComsProgress {
    std::chrono::system_clock::time_point absTime_start;

    std::chrono::steady_clock::time_point time_0;

    double now;
    double total;
    double speed;
    double percent;
    std::chrono::duration<double> ETA_sec;

    PTDA_ComsProgress(
        double _now=0, double _total=0, double _speed=0, double _percent=0,
        std::chrono::duration<double> _ETA_sec=std::chrono::duration<double>(0), 
        std::chrono::system_clock::time_point _absTime_start=std::chrono::system_clock::now(), 
        std::chrono::steady_clock::time_point _time_0=std::chrono::steady_clock::now()
    ):
        now(_now), total(_total), speed(_speed), percent(_percent), ETA_sec(_ETA_sec), absTime_start(_absTime_start), time_0(_time_0)
    {

    }
    PTDA_ComsProgress(const PTDA_ComsProgress& _other): now(_other.now), total(_other.total), speed(_other.speed), percent(_other.percent), ETA_sec(_other.ETA_sec), absTime_start(_other.absTime_start), time_0(_other.time_0) {
    }
    PTDA_ComsProgress(PTDA_ComsProgress&& _other): now(_other.now), total(_other.total), speed(_other.speed), percent(_other.percent), ETA_sec(_other.ETA_sec), absTime_start(_other.absTime_start), time_0(_other.time_0) {
    }
    ~PTDA_ComsProgress() {}

    PTDA_ComsProgress& operator=(const PTDA_ComsProgress& _other) {
        now = _other.now;
        total = _other.total;
        speed = _other.speed;
        percent = _other.percent;
        ETA_sec = _other.ETA_sec;
        absTime_start = _other.absTime_start;
        time_0 = _other.time_0;

        return *this;
    }
    PTDA_ComsProgress& operator=(PTDA_ComsProgress&& _other) {
        now = _other.now;
        total = _other.total;
        speed = _other.speed;
        percent = _other.percent;
        ETA_sec = _other.ETA_sec;
        absTime_start = _other.absTime_start;
        time_0 = _other.time_0;
        
        return *this;
    }

    PTDA_ComsProgress& update(double _now) {
        auto time_1 = std::chrono::steady_clock::now();
        auto interval = time_1 - this->time_0;
        double delta_progress = _now - this->now;
        this->speed = delta_progress / interval.count();
        this->percent = (_now/this->total)*100.0;

        this->ETA_sec = std::chrono::duration<double>((total-_now)/(this->speed));

        this->now = _now;
        this->time_0 = time_1;
        return *this;
    }
    PTDA_ComsProgress& update(double _now, double _total) {
        this->total = _total;
        this->update(_now);
        return *this;
    }

};

struct PTDA_Coms {
    std::stringstream message;

    PTDA_ComsProgress progress;

    PTDA_Coms() {
        message.clear();
    }
    PTDA_Coms(const PTDA_Coms& _other) {
        message << _other.message.rdbuf();
        progress = _other.progress;
    }
    PTDA_Coms(PTDA_Coms&& _other) {
        message.swap(_other.message);
        progress = _other.progress;
    }
    ~PTDA_Coms() {}

    PTDA_Coms& operator=(const PTDA_Coms& _other) {
        message << _other.message.rdbuf();
        progress = _other.progress;
    }
    PTDA_Coms& operator=(PTDA_Coms&& _other) {
        message.swap(_other.message);
        progress = _other.progress;
    }
};


#endif //HPP__PTDA_Coms