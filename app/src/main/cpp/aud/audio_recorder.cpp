#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "audio_recorder.h"

/** Called for every buffer is full; pass directly to handler. */
void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *recorder) {
    (static_cast<AudioRecorder *>(recorder))->ProcessSLCallback(bq);
}

void AudioRecorder::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq) {
    assert(bq == recBufQueueItf_);
    sample_buf *dataBuf = nullptr;
    devShadowQueue_->front(&dataBuf);
    devShadowQueue_->pop();
    dataBuf->size_ = dataBuf->cap_;  // device only calls us when it is really full
    recQueue_->push(dataBuf);

    myfile.write((char*) dataBuf->buf_, dataBuf->size_); // TODO

    sample_buf *freeBuf;
    while (freeQueue_->front(&freeBuf) && devShadowQueue_->push(freeBuf)) {
        freeQueue_->pop();
        SLresult result = (*bq)->Enqueue(bq, freeBuf->buf_, freeBuf->cap_);
        SLASSERT(result);
    }

    ++audioBufCount;

    // should leave the device to sleep to save power if no buffers
    /*TODO devShadowQueue_ is implemented exactly as AudioEcho
     * if (devShadowQueue_->size() == 0)
        (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);*/
}

AudioRecorder::AudioRecorder(SampleFormat *sampleFormat, SLEngineItf slEngine)
        : freeQueue_(nullptr),
          recQueue_(nullptr),
          devShadowQueue_(nullptr),
          callback_() {
    SLresult result;
    /*sampleInfo_ = *sampleFormat;
    SLAndroidDataFormat_PCM_EX format_pcm;
    ConvertToSLSampleFormat(&format_pcm, &sampleInfo_);*/

    //SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};

    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE,
                                      SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT, nullptr};
    SLDataSource audioSrc = {&loc_dev, nullptr};

    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, DEVICE_SHADOW_BUFFER_QUEUE_LEN};

    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // create audio recorder (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    // MAHDI: I used the native-audio version of "id" and "req".
    result = (*slEngine)->CreateAudioRecorder(
            slEngine, &recObjectItf_, &audioSrc, &audioSnk, 1, id, req);
    SLASSERT(result);

    result = (*recObjectItf_)->Realize(recObjectItf_, SL_BOOLEAN_FALSE);
    SLASSERT(result);
    result = (*recObjectItf_)->GetInterface(recObjectItf_, SL_IID_RECORD, &recItf_);
    SLASSERT(result);

    result = (*recObjectItf_)->GetInterface(recObjectItf_, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                            &recBufQueueItf_);
    SLASSERT(result);

    result = (*recBufQueueItf_)->RegisterCallback(recBufQueueItf_, bqRecorderCallback, this);
    SLASSERT(result);

    devShadowQueue_ = new AudioQueue(DEVICE_SHADOW_BUFFER_QUEUE_LEN);
    assert(devShadowQueue_);

    myfile = std::ofstream("/data/data/ir.mahdiparastesh.mergen/files/test.pcm",
                           std::ios::binary | std::ios::out); // TODO
}

SLboolean AudioRecorder::Start() {
    if (!freeQueue_ || !recQueue_ || !devShadowQueue_) {
        LOGE("====NULL poiter to Start(%p, %p, %p)", freeQueue_, recQueue_, devShadowQueue_);
        return SL_BOOLEAN_FALSE;
    }
    audioBufCount = 0;

    SLresult result;
    // in case already recording, stop recording and clear buffer queue
    result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
    SLASSERT(result);
    result = (*recBufQueueItf_)->Clear(recBufQueueItf_);
    SLASSERT(result);

    for (int i = 0; i < RECORD_DEVICE_KICKSTART_BUF_COUNT; i++) {
        sample_buf *buf = nullptr;
        if (!freeQueue_->front(&buf)) {
            LOGE("=====OutOfFreeBuffers @ startingRecording @ (%d)", i);
            break;
        }
        freeQueue_->pop();
        assert(buf->buf_ && buf->cap_ && !buf->size_);

        result = (*recBufQueueItf_)->Enqueue(recBufQueueItf_, buf->buf_, buf->cap_);
        SLASSERT(result);
        devShadowQueue_->push(buf);
    }

    result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_RECORDING);

    return (result == SL_RESULT_SUCCESS ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE);
}

SLboolean AudioRecorder::Stop() {
    // In case already recording, stop recording and clear buffer queue.
    SLuint32 curState;
    SLresult result = (*recItf_)->GetRecordState(recItf_, &curState);
    SLASSERT(result);
    if (curState == SL_RECORDSTATE_STOPPED) {
        throw "WTF?";
        //return SL_BOOLEAN_TRUE;
    }
    result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
    SLASSERT(result);

    result = (*recBufQueueItf_)->Clear(recBufQueueItf_);
    SLASSERT(result);
    return SL_BOOLEAN_TRUE;
}

AudioRecorder::~AudioRecorder() {
    myfile.close(); // TODO

    // destroy audio recorder object, and invalidate all associated interfaces
    if (recObjectItf_ != nullptr)
        (*recObjectItf_)->Destroy(recObjectItf_);

    if (devShadowQueue_) {
        sample_buf *buf = nullptr;
        while (devShadowQueue_->front(&buf)) {
            devShadowQueue_->pop();
            freeQueue_->push(buf);
        }
        delete (devShadowQueue_);
    }
}

void AudioRecorder::SetBufQueues(AudioQueue *freeQ, AudioQueue *recQ) {
    assert(freeQ && recQ);
    freeQueue_ = freeQ;
    recQueue_ = recQ;
}
