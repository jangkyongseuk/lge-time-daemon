#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <sstream>
#include "../lge-time-lib.h"

class LgeTimeLibTest : public ::testing::Test {
protected:
    void SetUp() override {
        lib = std::make_unique<LgeTimeLib>();
    }
    
    void TearDown() override {
        lib.reset();
    }
    
    std::unique_ptr<LgeTimeLib> lib;
};

TEST_F(LgeTimeLibTest, ValidateDatetime_ValidFormat_ReturnsTrue) {
    EXPECT_TRUE(lib->validateDatetime("2024-01-01 12:30:45"));
    EXPECT_TRUE(lib->validateDatetime("2023-12-31 23:59:59"));
    EXPECT_TRUE(lib->validateDatetime("2024-02-29 00:00:00"));
}

TEST_F(LgeTimeLibTest, ValidateDatetime_InvalidFormat_ReturnsFalse) {
    EXPECT_FALSE(lib->validateDatetime("2024/01/01 12:30:45"));
    EXPECT_FALSE(lib->validateDatetime("2024-1-1 12:30:45"));
    EXPECT_FALSE(lib->validateDatetime("2024-01-01 12:30"));
    EXPECT_FALSE(lib->validateDatetime("invalid-datetime"));
    EXPECT_FALSE(lib->validateDatetime(""));
    EXPECT_FALSE(lib->validateDatetime("2024-01-01 25:00:00"));
    EXPECT_FALSE(lib->validateDatetime("2024-13-01 12:00:00"));
}

TEST_F(LgeTimeLibTest, ReadRtcTime_DeviceNotFound_ReturnsDefault) {
    std::string result = lib->readRtcTime();
    EXPECT_EQ(result, LgeTimeLib::getDefaultDatetime());
}

TEST_F(LgeTimeLibTest, StaticConstants_HaveCorrectValues) {
    EXPECT_EQ(LgeTimeLib::getRtcDevice(), "/dev/rtc0");
    EXPECT_EQ(LgeTimeLib::getDefaultDatetime(), "2024-01-01 00:00:00");
    EXPECT_EQ(LgeTimeLib::getNormalSleepInterval(), 60);
    EXPECT_EQ(LgeTimeLib::getFastSleepInterval(), 5);
}

TEST_F(LgeTimeLibTest, InitialState_IsCorrect) {
    EXPECT_TRUE(lib->isRunning());
    EXPECT_EQ(lib->getSleepInterval(), LgeTimeLib::getNormalSleepInterval());
    EXPECT_TRUE(lib->getOldTime().empty());
}

TEST_F(LgeTimeLibTest, Stop_SetsRunningToFalse) {
    EXPECT_TRUE(lib->isRunning());
    lib->stop();
    EXPECT_FALSE(lib->isRunning());
}

TEST_F(LgeTimeLibTest, SleepInterval_CanBeModified) {
    int newInterval = 30;
    lib->setSleepInterval(newInterval);
    EXPECT_EQ(lib->getSleepInterval(), newInterval);
}

TEST_F(LgeTimeLibTest, OldTime_CanBeSetAndRetrieved) {
    std::string testTime = "2024-06-12 10:30:00";
    lib->setOldTime(testTime);
    EXPECT_EQ(lib->getOldTime(), testTime);
}

TEST_F(LgeTimeLibTest, MainLoopLogic_TimeChanged_UpdatesSleepInterval) {
    std::string newTime = "2024-06-12 10:30:00";
    std::string oldTime = "2024-06-12 10:29:00";
    
    lib->setOldTime(oldTime);
    
    if (newTime != lib->getOldTime()) {
        lib->setSleepInterval(LgeTimeLib::getFastSleepInterval());
        lib->setOldTime(newTime);
    }
    
    EXPECT_EQ(lib->getSleepInterval(), LgeTimeLib::getFastSleepInterval());
    EXPECT_EQ(lib->getOldTime(), newTime);
}

TEST_F(LgeTimeLibTest, MainLoopLogic_TimeUnchanged_KeepsNormalInterval) {
    std::string sameTime = "2024-06-12 10:30:00";
    
    lib->setOldTime(sameTime);
    lib->setSleepInterval(LgeTimeLib::getFastSleepInterval());
    
    if (sameTime == lib->getOldTime()) {
        lib->setSleepInterval(LgeTimeLib::getNormalSleepInterval());
    }
    
    EXPECT_EQ(lib->getSleepInterval(), LgeTimeLib::getNormalSleepInterval());
}

TEST_F(LgeTimeLibTest, DatetimeValidation_Performance) {
    std::string validTime = "2024-06-12 10:30:00";
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        lib->validateDatetime(validTime);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 1000);
}

TEST_F(LgeTimeLibTest, EdgeCases_LeapYear) {
    EXPECT_TRUE(lib->validateDatetime("2024-02-29 12:00:00"));
    EXPECT_TRUE(lib->validateDatetime("2023-02-28 12:00:00"));
}

TEST_F(LgeTimeLibTest, EdgeCases_BoundaryTimes) {
    EXPECT_TRUE(lib->validateDatetime("2024-01-01 00:00:00"));
    EXPECT_TRUE(lib->validateDatetime("2024-12-31 23:59:59"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
