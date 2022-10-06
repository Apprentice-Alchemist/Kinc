#ifdef DEFINE_LIB
DEFINE_LIB(libmediandk, 21, "libmediandk.so")
DEFINE_LIB(libnativewindow, 21, "libmediandk.so")
#endif

#ifdef DEFINE_FUN
DEFINE_FUN(AMediaCodec_dequeueInputBuffer, libmediandk, 21, ssize_t (*AMediaCodec_dequeueInputBuffer)(AMediaCodec *, int64_t timeoutUs))
DEFINE_FUN(AMediaCodec_getInputBuffer, libmediandk, 21, uint8_t *(*AMediaCodec_getInputBuffer)(AMediaCodec *, size_t idx, size_t *out_size))
DEFINE_FUN(AMediaCodec_queueInputBuffer, libmediandk, 21,
           media_status_t (*AMediaCodec_queueInputBuffer)(AMediaCodec *, size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags))
DEFINE_FUN(AMediaCodec_dequeueOutputBuffer, libmediandk, 21,
           ssize_t (*AMediaCodec_dequeueOutputBuffer)(AMediaCodec *, AMediaCodecBufferInfo *info, int64_t timeoutUs))
DEFINE_FUN(AMediaCodec_getOutputBuffer, libmediandk, 21, uint8_t * (*AMediaCodec_getOutputBuffer)(
  AMediaCodec *,
  size_t idx,
  size_t *out_size
))
DEFINE_FUN(AMediaExtractor_getSampleFlags, libmediandk, 21, uint32_t (*AMediaExtractor_getSampleFlags)(
  AMediaExtractor *
))
DEFINE_FUN(AMediaExtractor_getSampleSize, libmediandk, 21, ssize_t (*AMediaExtractor_getSampleSize)(AMediaExtractor *))
DEFINE_FUN(AMediaCodec_releaseOutputBuffer, libmediandk, 21, media_status_t (*AMediaCodec_releaseOutputBuffer)(AMediaCodec *, size_t idx, bool render))
DEFINE_FUN(AMediaCodec_stop, libmediandk, 21, media_status_t (*AMediaCodec_stop)(AMediaCodec *))
DEFINE_FUN(AMediaCodec_configure, libmediandk, 21,
           media_status_t (*AMediaCodec_configure)(AMediaCodec *, const AMediaFormat *format, ANativeWindow *surface, AMediaCrypto *crypto, uint32_t flags))
DEFINE_FUN(AMediaCodec_start, libmediandk, 21, media_status_t (*AMediaCodec_start)(AMediaCodec *))
DEFINE_FUN(AMediaCodec_createDecoderByType, libmediandk, 21, AMediaCodec *(*AMediaCodec_createDecoderByType)(const char *mime_type))
DEFINE_FUN(AMediaCodec_getOutputFormat, libmediandk, 21, AMediaFormat *(*AMediaCodec_getOutputFormat)(AMediaCodec *))

DEFINE_FUN(AMediaExtractor_new, libmediandk, 21, AMediaExtractor *(*AMediaExtractor_new)())
DEFINE_FUN(AMediaExtractor_setDataSourceFd, libmediandk, 21,
           media_status_t (*AMediaExtractor_setDataSourceFd)(AMediaExtractor *, int fd, off64_t offset, off64_t length))
DEFINE_FUN(AMediaExtractor_readSampleData, libmediandk, 21, ssize_t (*AMediaExtractor_readSampleData)(AMediaExtractor *, uint8_t *buffer, size_t capacity))
DEFINE_FUN(AMediaExtractor_getSampleTime, libmediandk, 21, int64_t (*AMediaExtractor_getSampleTime)(AMediaExtractor *))
DEFINE_FUN(AMediaExtractor_advance, libmediandk, 21, bool (*AMediaExtractor_advance)(AMediaExtractor *))
DEFINE_FUN(AMediaExtractor_getTrackCount, libmediandk, 21, size_t (*AMediaExtractor_getTrackCount)(AMediaExtractor *))
DEFINE_FUN(AMediaExtractor_getTrackFormat, libmediandk, 21, AMediaFormat *(*AMediaExtractor_getTrackFormat)(AMediaExtractor *, size_t idx))
DEFINE_FUN(AMediaExtractor_selectTrack, libmediandk, 21, media_status_t (*AMediaExtractor_selectTrack)(AMediaExtractor *, size_t idx))
DEFINE_FUN(AMediaExtractor_unselectTrack, libmediandk, 21, media_status_t (*AMediaExtractor_unselectTrack)(AMediaExtractor *, size_t idx))
DEFINE_FUN(AMediaExtractor_getSampleTrackIndex, libmediandk, 21, int (*AMediaExtractor_getSampleTrackIndex)(AMediaExtractor *))
DEFINE_FUN(AMediaFormat_delete, libmediandk, 21, media_status_t (*AMediaFormat_delete)(AMediaFormat *))
DEFINE_FUN(AMediaFormat_getString, libmediandk, 21, bool (*AMediaFormat_getString)(AMediaFormat *, const char *name, const char **out))
DEFINE_FUN(AMediaFormat_getInt32, libmediandk, 21, bool (*AMediaFormat_getInt32)(AMediaFormat *, const char *name, int32_t *out))
DEFINE_FUN(AMediaFormat_getInt64, libmediandk, 21, bool (*AMediaFormat_getInt64)(AMediaFormat *, const char *name, int64_t *out))
DEFINE_FUN(AMediaFormat_toString, libmediandk, 21, const char *(*AMediaFormat_toString)(AMediaFormat *))

DEFINE_VAR(AMEDIAFORMAT_KEY_MIME, libmediandk, 21, const char *)
DEFINE_VAR(AMEDIAFORMAT_KEY_SAMPLE_RATE, libmediandk, 21, const char *)
DEFINE_VAR(AMEDIAFORMAT_KEY_PCM_ENCODING, libmediandk, 21, const char *)
DEFINE_VAR(AMEDIAFORMAT_KEY_CHANNEL_COUNT, libmediandk, 21, const char *)

DEFINE_FUN(AImageReader_newWithUsage, libmediandk, 26,
           media_status_t (*AImageReader_newWithUsage)(int32_t width, int32_t height, int32_t format, uint64_t usage, int32_t maxImages, AImageReader **reader))
DEFINE_FUN(AImageReader_getWindow, libmediandk, 24, media_status_t (*AImageReader_getWindow)(AImageReader *reader, ANativeWindow **window))
DEFINE_FUN(AImageReader_delete, libmediandk, 24, void (*AImageReader_delete)(AImageReader *reader))
DEFINE_FUN(AImageReader_acquireLatestImage, libmediandk, 24, media_status_t (*AImageReader_acquireLatestImage)(AImageReader *reader, AImage **image))
DEFINE_FUN(AHardwareBuffer_describe, libnativewindow, 26,
           void (*AHardwareBuffer_describe)(const AHardwareBuffer *buffer, AHardwareBuffer_Desc *outDesc))
DEFINE_FUN(AImage_getHardwareBuffer, libmediandk, 26, media_status_t (*AImage_getHardwareBuffer)(const AImage *image, AHardwareBuffer **buffer))
DEFINE_FUN(AImage_delete, libmediandk, 26, void (*AImage_delete)(const AImage *image))
DEFINE_FUN(AImage_getFormat, libmediandk, 23, media_status_t (*AImage_getFormat)(const AImage *image, int32_t *format))
#endif