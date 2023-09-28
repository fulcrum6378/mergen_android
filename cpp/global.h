#ifndef GLOBAL_H
#define GLOBAL_H

#include <android/log.h>
#include <string>

#define LOG_TAG "ASAJJ"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ASSERT(cond, fmt, ...)                                \
  if (!(cond)) {                                              \
    __android_log_assert(#cond, LOG_TAG, fmt, ##__VA_ARGS__); \
  }

static const std::string cacheDirPath("/data/data/ir.mahdiparastesh.mergen/cache/");

#endif //GLOBAL_H
