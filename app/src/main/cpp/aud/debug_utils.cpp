#include <cinttypes>
#include <cstdio>
#include <sys/stat.h>

#include "../native_debug.h"
#include "debug_utils.h"

static const char *FILE_PREFIX = "/data/data/ir.mahdiparastesh.mergen/files/";

volatile uint32_t AndroidLog::fileIdx_ = 0;

AndroidLog::AndroidLog() : fp_(nullptr), prevTick_(static_cast<uint64_t>(0)) {
    fileName_ = FILE_PREFIX;
    openFile();
}

AndroidLog::AndroidLog(std::string &file_name)
        : fp_(nullptr), prevTick_(static_cast<uint64_t>(0)) {
    fileName_ = std::string(FILE_PREFIX) + std::string("_") + file_name;
    openFile();
}

AndroidLog::~AndroidLog() { flush(); }

void AndroidLog::flush() {
    if (fp_) {
        fflush(fp_);
        fclose(fp_);
        fp_ = nullptr;
    }
    prevTick_ = static_cast<uint64_t>(0);
}

void AndroidLog::log(void *buf, uint32_t size) {
    Lock fileLock(&mutex_);
    if (!buf || !size) return;
    if (fp_ || openFile()) fwrite(buf, size, 1, fp_);
}

void AndroidLog::log(const char *fmt, ...) {
    Lock fileLock(&mutex_);
    if (!fmt) return;
    if (fp_ || openFile()) {
        va_list vp;
        va_start(vp, fmt);
        vfprintf(fp_, fmt, vp);
        va_end(vp);
    }
}

FILE *AndroidLog::openFile() {
    Lock fileLock(&mutex_);
    if (fp_) return fp_;

    char fileName[64];
    sprintf(fileName, "%s_%d", fileName_.c_str(), AndroidLog::fileIdx_++);
    fp_ = fopen(fileName, "wb");
    if (fp_ == nullptr) LOGE("====failed to open file %s", fileName);
    return fp_;
}

void AndroidLog::logTime() {
    if (prevTick_ == static_cast<uint64_t>(0)) {
        /*
         * init counter, bypass the first one
         */
        prevTick_ = getCurrentTicks();
        return;
    }
    uint64_t curTick = getCurrentTicks();
    uint64_t delta = curTick - prevTick_;
    log("%" PRIu64 "    %" PRIu64 "\n", curTick, delta);
    prevTick_ = curTick;
}

uint64_t AndroidLog::getCurrentTicks() {
    struct timeval Time;
    gettimeofday(&Time, nullptr);
    return (static_cast<uint64_t>(1000000) * Time.tv_sec + Time.tv_usec);
}
