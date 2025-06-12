#pragma once

#include <string>
#include <chrono>
#include <thread>
#include <ctime>
#include <sys/time.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <regex>

class LgeTimeLib {
private:
    static const std::string RTC_DEVICE;
    static const std::string DEFAULT_DATETIME;
    static const int NORMAL_SLEEP_INTERVAL = 60;
    static const int FAST_SLEEP_INTERVAL = 5;
    
    std::string old_time;
    int sleep_interval;
    bool running;
    
public:
    LgeTimeLib();
    ~LgeTimeLib();
    
    void stop();
    std::string readRtcTime();
    bool validateDatetime(const std::string& datetime);
    bool setSystemTime(const std::string& datetime);
    void run();
    
    // Getters for testing
    int getSleepInterval() const { return sleep_interval; }
    const std::string& getOldTime() const { return old_time; }
    bool isRunning() const { return running; }
    
    // Setters for testing
    void setSleepInterval(int interval) { sleep_interval = interval; }
    void setOldTime(const std::string& time) { old_time = time; }
    
    // Static methods for testing
    static std::string getRtcDevice() { return RTC_DEVICE; }
    static std::string getDefaultDatetime() { return DEFAULT_DATETIME; }
    static int getNormalSleepInterval() { return NORMAL_SLEEP_INTERVAL; }
    static int getFastSleepInterval() { return FAST_SLEEP_INTERVAL; }
};
