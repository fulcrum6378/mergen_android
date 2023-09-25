#ifndef VIS_BITMAP_STREAM_H
#define VIS_BITMAP_STREAM_H

#include <fstream>
#include <media/NdkImage.h>
#include <thread>

#include "global.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

// related to Windows
struct bmpfile_magic {
    unsigned char magic[2];
};

struct bmpfile_header {
    uint32_t file_size;
    uint16_t creator1;
    uint16_t creator2;
    uint32_t bmp_offset;
};

struct bmpfile_dib_info {
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t num_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t bmp_byte_size;
    int32_t hres;
    int32_t vres;
    uint32_t num_colors;
    uint32_t num_important_colors;
};

#pragma clang diagnostic pop

class BitmapStream {
private:
    const char *filesDir = "/data/data/ir.mahdiparastesh.mergen/cache/";
    std::ofstream *store{nullptr};
    std::mutex store_mutex;
    int skipped_count = 0;
    std::pair<int16_t, int16_t> dimensions_;
    const int kMaxChannelValue = 262143;

public:
    BitmapStream(std::pair<int16_t, int16_t> dimensions) {
        store = new std::ofstream(
                (filesDir + std::string("vis.rgb")).c_str(),
                std::ios::out | std::ios::binary);
        dimensions_ = dimensions;
    }

    [[maybe_unused]] void BakeMetadata() {
        int32_t width = dimensions_.first;
        int32_t height = dimensions_.second;
        std::ofstream metadata(
                filesDir + std::to_string(width) + "x" + std::to_string(height),
                std::ios::out | std::ios::binary);

        bmpfile_magic magic{'B', 'M'};
        metadata.write((char *) (&magic), sizeof(magic));
        bmpfile_header header = {0};
        header.bmp_offset =
                sizeof(bmpfile_magic) + sizeof(bmpfile_header) + sizeof(bmpfile_dib_info);
        header.file_size = header.bmp_offset + (height * 3 + width % 4) * height;
        metadata.write((char *) (&header), sizeof(header));
        bmpfile_dib_info dib_info = {0};
        dib_info.header_size = sizeof(bmpfile_dib_info);
        dib_info.width = width;
        dib_info.height = height;
        dib_info.num_planes = 1;
        dib_info.bits_per_pixel = 24;
        dib_info.compression = 0;
        dib_info.bmp_byte_size = 0;
        dib_info.hres = 2835;
        dib_info.vres = 2835;
        dib_info.num_colors = 0;
        dib_info.num_important_colors = 0;
        metadata.write((char *) (&dib_info), sizeof(dib_info));
        metadata.close();
    }

    bool HandleImage(AImage *image) {
        bool ret = false;
        if (skipped_count == 0) {
            std::thread(&BitmapStream::Append, this, image).detach();
            ret = true;
        }
        skipped_count++;
        if (skipped_count == 5) skipped_count = 0;
        return ret;
    }

    /**
     * Yuv2Rgb algorithm is from:
     * https://github.com/tensorflow/tensorflow/blob/5dcfc51118817f27fad5246812d83e5dccdc5f72/
     * tensorflow/tools/android/test/jni/yuv2rgb.cc
     */
    void Append(AImage *image) {
        store_mutex.lock();

        AImageCropRect srcRect;
        AImage_getCropRect(image, &srcRect);
        int32_t yStride, uvStride;
        uint8_t *yPixel, *uPixel, *vPixel;
        int32_t yLen, uLen, vLen;
        AImage_getPlaneRowStride(image, 0, &yStride);
        AImage_getPlaneRowStride(image, 1, &uvStride);
        AImage_getPlaneData(image, 0, &yPixel, &yLen);
        AImage_getPlaneData(image, 1, &vPixel, &vLen);
        AImage_getPlaneData(image, 2, &uPixel, &uLen);
        int32_t uvPixelStride;
        AImage_getPlanePixelStride(image, 1, &uvPixelStride);
        /*AImage_getWidth(image, &width);
        AImage_getHeight(image, &height);
        height = MIN(width, (srcRect.bottom - srcRect.top));
        width = MIN(height, (srcRect.right - srcRect.left));*/
        int16_t width = dimensions_.first, height = dimensions_.second;

        for (int16_t y = height - 1; y >= 0; y--) {
            const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;

            int32_t uv_row_start = uvStride * ((y + srcRect.top) >> 1);
            const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
            const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

            for (int16_t x = 0; x < width; x++) {
                const int32_t uv_offset = (x >> 1) * uvPixelStride;

                int nY = pY[x] - 16;
                int nU = pU[uv_offset] - 128;
                int nV = pV[uv_offset] - 128;
                if (nY < 0) nY = 0;

                int nR = (int) (1192 * nY + 1634 * nV);
                int nG = (int) (1192 * nY - 833 * nV - 400 * nU);
                int nB = (int) (1192 * nY + 2066 * nU);

                int maxR = MAX(0, nR), maxG = MAX(0, nG), maxB = MAX(0, nB);
                nR = MIN(kMaxChannelValue, maxR);
                nG = MIN(kMaxChannelValue, maxG);
                nB = MIN(kMaxChannelValue, maxB);

                nR = (nR >> 10) & 0xff;
                nG = (nG >> 10) & 0xff;
                nB = (nB >> 10) & 0xff;

                store->put((char) nR);
                store->put((char) nG);
                store->put((char) nB);
            }
            for (int i = 0; i < width % 4; i++) store->put(0);
        }
        /*LOGE("%s", (std::to_string(store->tellp()) +
                    " bytes - dimensions: " + std::to_string(width) + "x" + std::to_string(height)
        ).c_str());*/
        store_mutex.unlock();
        AImage_delete(image);
    }

    ~BitmapStream() {
        delete store;
        store = nullptr;
    }
};

#endif //VIS_BITMAP_STREAM_H
