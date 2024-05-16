#ifndef MERGEN_GLOBAL_H
#define MERGEN_GLOBAL_H

#include <android/log.h>
#include <string>

/** Logging */
#define LOG_TAG "ZOEY"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ASSERT(cond, fmt, ...)                                \
  if (!(cond)) {                                              \
    __android_log_assert(#cond, LOG_TAG, fmt, ##__VA_ARGS__); \
  }

/** Internal Storage */
static const std::string
        filesDir("/data/data/ir.mahdiparastesh.mergen/files/"),
        cacheDir("/data/data/ir.mahdiparastesh.mergen/cache/");

// JNI references can't be put here statically.

#endif //MERGEN_GLOBAL_H
