#ifndef GLOBAL_H
#define GLOBAL_H

#include <android/log.h>
#include <media/NdkImage.h>

#define VIS_IMAGE_FORMAT AIMAGE_FORMAT_YUV_420_888
// together with AIMAGE_FORMAT_JPEG, these are the only supported options for my phone apparently!

#define LOG_TAG "ir.mahdiparastesh.mergen"
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ASSERT(cond, fmt, ...)                                \
  if (!(cond)) {                                              \
    __android_log_assert(#cond, LOG_TAG, fmt, ##__VA_ARGS__); \
  }

#endif //GLOBAL_H
