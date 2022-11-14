#ifndef NATIVE_AUDIO_DEBUG_UTILS_H
#define NATIVE_AUDIO_DEBUG_UTILS_H

#include <cstdio>
#include <mutex>
#include <string>

/*
 * debug_write_file()
 *  Write given data to a file as binary file. File name is
 *     "/sdcard/data/audio_%d", file_index++
 *  requirement: must have /sdcard/data already created on android device
 */
class Lock {
public:
    explicit Lock(std::recursive_mutex *mtx) {
        mutex_ = mtx;
        mutex_->lock();
    }

    ~Lock() { mutex_->unlock(); }

private:
    std::recursive_mutex *mutex_;
};

class AndroidLog {
public:
    AndroidLog();

    AndroidLog(std::string &fileName);

    ~AndroidLog();

    void log(void *buf, uint32_t size);

    void log(const char *fmt, ...);

    void logTime();

    void flush();

    static volatile uint32_t fileIdx_;

private:
    uint64_t getCurrentTicks();

    FILE *fp_;

    FILE *openFile();

    uint64_t prevTick_;  // Tick in milisecond
    std::recursive_mutex mutex_;
    std::string fileName_;
};

void debug_write_file(void *buf, uint32_t size);

#endif
