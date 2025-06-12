#include "lge-time-lib.h"
#include <iostream>
#include <fstream>

const std::string LgeTimeLib::RTC_DEVICE = "/dev/rtc0";
const std::string LgeTimeLib::DEFAULT_DATETIME = "2024-01-01 00:00:00";

LgeTimeLib::LgeTimeLib() : sleep_interval(NORMAL_SLEEP_INTERVAL), running(true) {
    openlog("lge-time-lib", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "LGE Time Library initialized");
}

LgeTimeLib::~LgeTimeLib() {
    syslog(LOG_INFO, "LGE Time Library destroyed");
    closelog();
}

void LgeTimeLib::stop() {
    running = false;
}

std::string LgeTimeLib::readRtcTime() {
    std::ifstream rtc_file(RTC_DEVICE);
    if (!rtc_file.is_open()) {
        syslog(LOG_ERR, "Failed to open RTC device: %s", RTC_DEVICE.c_str());
        return DEFAULT_DATETIME;
    }
    
    std::string datetime;
    if (std::getline(rtc_file, datetime)) {
        rtc_file.close();
        return datetime;
    } else {
        syslog(LOG_ERR, "Failed to read from RTC device");
        rtc_file.close();
        return DEFAULT_DATETIME;
    }
}

bool LgeTimeLib::validateDatetime(const std::string& datetime) {
    std::regex datetime_pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    return std::regex_match(datetime, datetime_pattern);
}

bool LgeTimeLib::setSystemTime(const std::string& datetime) {
    if (!validateDatetime(datetime)) {
        syslog(LOG_ERR, "Invalid datetime format: %s", datetime.c_str());
        return false;
    }
    
    struct tm tm_time = {};
    if (strptime(datetime.c_str(), "%Y-%m-%d %H:%M:%S", &tm_time) == nullptr) {
        syslog(LOG_ERR, "Failed to parse datetime: %s", datetime.c_str());
        return false;
    }
    
    tm_time.tm_isdst = 0;
    time_t new_time = timegm(&tm_time);
    
    struct timeval tv;
    tv.tv_sec = new_time;
    tv.tv_usec = 0;
    
    if (settimeofday(&tv, nullptr) != 0) {
        syslog(LOG_ERR, "Failed to set system time: %s", datetime.c_str());
        return false;
    }
    
    syslog(LOG_INFO, "System time updated to: %s UTC", datetime.c_str());
    return true;
}

void LgeTimeLib::run() {
    syslog(LOG_INFO, "Starting main daemon loop");
    
    while (running) {
        std::string current_time = readRtcTime();
        
        if (current_time != old_time) {
            if (setSystemTime(current_time)) {
                old_time = current_time;
                sleep_interval = FAST_SLEEP_INTERVAL;
                syslog(LOG_INFO, "Time changed, using fast sleep interval");
            } else {
                syslog(LOG_ERR, "Failed to update system time");
            }
        } else {
            sleep_interval = NORMAL_SLEEP_INTERVAL;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(sleep_interval));
    }
}
