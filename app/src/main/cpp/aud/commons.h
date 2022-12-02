#ifndef AUD_COMMONS_H
#define AUD_COMMONS_H

#include <cassert>
#include <memory>
#include <SLES/OpenSLES_Android.h>

#include "../global.h" // do not include "recorder.h" here!

// Audio Sample Controls (frame = sample, bytes < sample < buffer)
#define SAMPLE_RATE 48000000 // millihertz
#define FRAMES_PER_BUF 192
#define AUDIO_SAMPLE_CHANNELS 1
#define BITS_PER_SAMPLE SL_PCMSAMPLEFORMAT_FIXED_16 // name BIT_DEPTH or BITS_PER_FRAME

// Sample Buffer Controls
#define RECORD_DEVICE_KICKSTART_BUF_COUNT 2
#define PLAY_KICKSTART_BUFFER_COUNT 3
#define DEVICE_SHADOW_BUFFER_QUEUE_LEN 4
#define BUF_COUNT 16

struct SampleFormat {
    uint32_t sampleRate_;
    uint32_t framesPerBuf_;
    uint16_t pcmFormat_;  // 8 bit, 16 bit, 24 bit ...
    uint32_t representation_;  // android extensions
};

void ConvertToSLSampleFormat(SLAndroidDataFormat_PCM_EX *pFormat, SampleFormat *format);

#define SLASSERT(x)                   \
  do {                                \
    assert(SL_RESULT_SUCCESS == (x)); \
    (void)(x);                        \
  } while (0)


#define CACHE_ALIGN 64

template<typename T>
class ProducerConsumerQueue { // borrowed from Ian NiLewis
public:
    explicit ProducerConsumerQueue(uint32_t size)
            : ProducerConsumerQueue(size, new T[size]) {}

    explicit ProducerConsumerQueue(uint32_t size, T *buffer)
            : size_(size), buffer_(buffer) {
        // This is necessary because we depend on twos-complement wraparound
        // to take care of overflow conditions.
        assert(size < std::numeric_limits<int>::max());
    }

    bool push(const T &item) {
        return push([&](T *ptr) -> bool {
            *ptr = item;
            return true;
        });
    }

    // writer() can return false, which indicates that the caller
    // of push() changed its mind while writing (e.g. ran out of bytes)
    template<typename F>
    bool push(const F &writer) {
        bool result = false;
        int readptr = read_.load(std::memory_order_acquire);
        int writeptr = write_.load(std::memory_order_relaxed);

        // note that while readptr and writeptr will eventually
        // wrap around, taking their difference is still valid as
        // long as size_ < MAXINT.
        int space = size_ - (int) (writeptr - readptr);
        if (space >= 1) {
            result = true;

            // writer
            if (writer(buffer_.get() + (writeptr % size_))) {
                ++writeptr;
                write_.store(writeptr, std::memory_order_release);
            }
        }
        return result;
    }

    // front out the queue, but not pop-out
    bool front(T *out_item) {
        return front([&](T *ptr) -> bool {
            *out_item = *ptr;
            return true;
        });
    }

    void pop(void) {
        int readptr = read_.load(std::memory_order_relaxed);
        ++readptr;
        read_.store(readptr, std::memory_order_release);
    }

    template<typename F>
    bool front(const F &reader) {
        bool result = false;

        int writeptr = write_.load(std::memory_order_acquire);
        int readptr = read_.load(std::memory_order_relaxed);

        // As above, wraparound is ok
        int available = (int) (writeptr - readptr);
        if (available >= 1) {
            result = true;
            reader(buffer_.get() + (readptr % size_));
        }

        return result;
    }

    uint32_t size(void) {
        int writeptr = write_.load(std::memory_order_acquire);
        int readptr = read_.load(std::memory_order_relaxed);

        return (uint32_t) (writeptr - readptr);
    }

private:
    int size_;
    std::unique_ptr<T> buffer_;

    // forcing cache line alignment to eliminate false sharing of the
    // frequently-updated read and write pointers. The object is to never
    // let these get into the "shared" state where they'd cause a cache miss
    // for every write.
    alignas(CACHE_ALIGN) std::atomic<int> read_{0};
    alignas(CACHE_ALIGN) std::atomic<int> write_{0};
};

struct sample_buf {
    uint8_t *buf_;   // audio sample container
    uint32_t cap_;   // buffer capacity in byte
    uint32_t size_;  // audio sample size (n buf) in byte (=> bytes per sample * FRAMES_PER_BUF)
};

__inline__ void releaseSampleBufs(sample_buf *bufs, uint32_t &count) {
    if (!bufs || !count) return;
    for (uint32_t i = 0; i < count; i++)
        if (bufs[i].buf_) delete[] bufs[i].buf_;
    delete[] bufs;
}

__inline__ sample_buf *allocateSampleBufs(uint32_t count, uint32_t sizeInByte) {
    if (count <= 0 || sizeInByte <= 0) {
        return nullptr;
    }
    sample_buf *bufs = new sample_buf[count];
    assert(bufs);
    memset(bufs, 0, sizeof(sample_buf) * count);

    uint32_t allocSize = (sizeInByte + 3) & ~3;  // padding to 4 bytes aligned
    uint32_t i;
    for (i = 0; i < count; i++) {
        bufs[i].buf_ = new uint8_t[allocSize];
        if (bufs[i].buf_ == nullptr) {
            LOGW("====Requesting %d buffers, allocated %d in %s", count, i, __FUNCTION__);
            break;
        }
        bufs[i].cap_ = sizeInByte;
        bufs[i].size_ = 0;  // 0 data in it
    }
    if (i < 2) {
        releaseSampleBufs(bufs, i);
        bufs = nullptr;
    }
    return bufs;
}

#endif //AUD_COMMONS_H
