#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "audio_recorder.h"

/** Called for every buffer is full; pass directly to handler. */
void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *recorder) {
    //LOGE("bqRecorderCallback");
    (static_cast<AudioRecorder *>(recorder))->ProcessSLCallback(bq);
}

void AudioRecorder::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq) {
    assert(bq == recBufQueueItf_);
    sample_buf *dataBuf = nullptr;
    devShadowQueue_->front(&dataBuf);
    devShadowQueue_->pop();
    dataBuf->size_ = dataBuf->cap_;  // device only calls us when it is really full
    recQueue_->push(dataBuf);

    sample_buf *freeBuf;
    while (freeQueue_->front(&freeBuf) && devShadowQueue_->push(freeBuf)) {
        freeQueue_->pop();
        SLresult result = (*bq)->Enqueue(bq, freeBuf->buf_, freeBuf->cap_);
        SLASSERT(result);
    }

    ++audioBufCount;

    // should leave the device to sleep to save power if no buffers
    /*TODO if (devShadowQueue_->size() == 0)
        (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);*/
    // assert(devShadowQueue_->size() != 0);
    LOGE("%s", ("Shadow: " + std::to_string(dataBuf->size_)).c_str());
    /*02:47:15.082  E  Shadow: 4
02:47:15.086  E  Shadow: 4
02:47:15.090  E  Shadow: 4
02:47:15.094  E  Shadow: 4
02:47:15.098  E  Shadow: 4
02:47:15.102  E  Shadow: 4
02:47:15.106  E  Shadow: 4
02:47:15.110  E  Shadow: 4
02:47:15.114  E  Shadow: 4
02:47:15.118  E  Shadow: 4
02:47:15.123  E  Shadow: 4
02:47:15.126  E  Shadow: 4
02:47:15.130  E  Shadow: 3
02:47:15.134  E  Shadow: 2
02:47:15.138  E  Shadow: 1
02:47:15.142  E  Shadow: 0
     AND NOTHING ELSE*/

    AudioQueue *playQueue_ = recQueue_;

    // std::lock_guard<std::mutex> lock(stopMutex_);

    // retrieve the finished device buf and put onto the free queue
    // so recorder could re-use it
    sample_buf *buf = dataBuf;
    //if (!devShadowQueue_->front(&buf)) {
    /*
     * This should not happen: we got a callback,
     * but we have no buffer in deviceShadowedQueue
     * we lost buffers this way...(ERROR)
     */
    //    return;
    //}
    devShadowQueue_->pop();

    //if (buf != &silentBuf_) {
    buf->size_ = 0;
    freeQueue_->push(buf);

    if (!playQueue_->front(&buf)) return;

    devShadowQueue_->push(buf);
    (*bq)->Enqueue(bq, buf->buf_, buf->size_);
    playQueue_->pop();
    return;
    //}

    if (playQueue_->size() < PLAY_KICKSTART_BUFFER_COUNT) {
        (*bq)->Enqueue(bq, buf->buf_, buf->size_);
        // FIXME devShadowQueue_->push(&silentBuf_);
        return;
    }

    assert(PLAY_KICKSTART_BUFFER_COUNT <=
           (DEVICE_SHADOW_BUFFER_QUEUE_LEN - devShadowQueue_->size()));
    for (int32_t idx = 0; idx < PLAY_KICKSTART_BUFFER_COUNT; idx++) {
        playQueue_->front(&buf);
        playQueue_->pop();
        devShadowQueue_->push(buf);
        (*bq)->Enqueue(bq, buf->buf_, buf->size_);
    }
}

AudioRecorder::AudioRecorder(SampleFormat *sampleFormat, SLEngineItf slEngine)
        : freeQueue_(nullptr),
          recQueue_(nullptr),
          devShadowQueue_(nullptr),
          callback_() {
    SLresult result;
    sampleInfo_ = *sampleFormat;
    SLAndroidDataFormat_PCM_EX format_pcm;
    ConvertToSLSampleFormat(&format_pcm, &sampleInfo_);

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
    const SLInterfaceID id[2] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_ANDROIDCONFIGURATION};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*slEngine)->CreateAudioRecorder(
            slEngine, &recObjectItf_, &audioSrc, &audioSnk, 1, id, req);
    SLASSERT(result);

    // Configure the voice recognition preset which has no signal processing for lower latency.
    SLAndroidConfigurationItf inputConfig;
    result = (*recObjectItf_)->GetInterface(
            recObjectItf_, SL_IID_ANDROIDCONFIGURATION, &inputConfig);
    if (SL_RESULT_SUCCESS == result) {
        SLuint32 presetValue = SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION;
        (*inputConfig)->SetConfiguration(
                inputConfig, SL_ANDROID_KEY_RECORDING_PRESET, &presetValue, sizeof(SLuint32));
    }
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
    if (curState == SL_RECORDSTATE_STOPPED) return SL_BOOLEAN_TRUE;
    assert(curState != SL_RECORDSTATE_STOPPED);
    result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
    SLASSERT(result);

    result = (*recBufQueueItf_)->Clear(recBufQueueItf_);
    SLASSERT(result);
    return SL_BOOLEAN_TRUE;
}

AudioRecorder::~AudioRecorder() {
    /*myfile = std::ofstream(path, std::ios::binary | std::ios::out);
    myfile.write((char *) dataBuf->buf_, dataBuf->size_);
    myfile.close();*/
    char path[] = "/data/data/ir.mahdiparastesh.mergen/files/test.txt";
    remove(path);
    std::ofstream text;
    text.open(path);
    text << "freeQueue_ size: " + std::to_string(freeQueue_->size()) + "\n";
    text << "recQueue_ size: " + std::to_string(recQueue_->size()) + "\n";
    text << "devShadowQueue_ size: " + std::to_string(devShadowQueue_->size()) + "\n";
    text << "audioBufCount: " + std::to_string(audioBufCount) + "\n";
    text.close();

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
