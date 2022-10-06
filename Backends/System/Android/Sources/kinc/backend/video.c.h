#include "Android.h"
#include "kinc/io/filereader.h"
#include "kinc/system.h"

#include <android/hardware_buffer.h>
#include <kinc/log.h>
#include <kinc/video.h>

#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

static void mcodec_sample_to_codec(AMediaCodec *codec, AMediaExtractor *extractor) {
	int64_t presentation_time = -1;
	ssize_t idx = android_funs.AMediaCodec_dequeueInputBuffer(codec, 10);
	if (idx >= 0) {
		size_t out_size = 0;
		ssize_t actual_size = 0;
		uint8_t *buffer = android_funs.AMediaCodec_getInputBuffer(codec, idx, &out_size);
		media_status_t status = AMEDIA_OK;
		assert(buffer != NULL);
		actual_size = android_funs.AMediaExtractor_readSampleData(extractor, buffer, out_size);
		ssize_t sample_size = android_funs.AMediaExtractor_getSampleSize(extractor);
		uint32_t sample_flags = android_funs.AMediaExtractor_getSampleFlags(extractor);
		if (actual_size < 0) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "EOS");
			actual_size = 0;
		}
		else {
			assert(actual_size == sample_size);
		}
		presentation_time = android_funs.AMediaExtractor_getSampleTime(extractor);

		status = android_funs.AMediaCodec_queueInputBuffer(codec, idx, 0, actual_size, presentation_time, sample_flags);
		assert(status == AMEDIA_OK);
	}
	else if (idx < -3) {
		// assert(false);
	}
}

static void *mcodec_callback(void *param) {
	JNIEnv *env;
	(*kinc_android_get_activity()->vm)->AttachCurrentThread(kinc_android_get_activity()->vm, &env, NULL);
	kinc_video_impl_t *video = param;
	int64_t start;
	struct timespec ts = {0};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	start = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
	while (true) {
		if (!video->paused) {
			for (int i = 0; i < 2; i++) {
				int index = android_funs.AMediaExtractor_getSampleTrackIndex(video->extractor);
				if (index == video->audio_track) {
					// mcodec_sample_to_codec(video->audio_codec, video->extractor);
				}
				if (index == video->video_track) {
					mcodec_sample_to_codec(video->video_codec, video->extractor);
				}
				android_funs.AMediaExtractor_advance(video->extractor);
			}
			AMediaCodecBufferInfo info;
			ssize_t odx;
			//			odx = android_funs.AMediaCodec_dequeueOutputBuffer(video->mcodec.audio_codec, &info, 0);
			//			if (odx >= 0) {
			//				media_status_t status;
			//				if (info.size != 0) {
			//					size_t size;
			//					void *data = android_funs.AMediaCodec_getOutputBuffer(video->mcodec.audio_codec, odx, &size);
			//					kinc_internal_video_sound_stream_insert_data(&video->mcodec.sound_stream, data, size / sizeof(float));
			//				}
			//				status = android_funs.AMediaCodec_releaseOutputBuffer(video->mcodec.audio_codec, odx, false);
			//
			//				assert(status == AMEDIA_OK);
			//			}else if(odx < -3) {
			//                assert(false);
			//            }
			// AMediaCodecBufferInfo info;
			odx = android_funs.AMediaCodec_dequeueOutputBuffer(video->video_codec, &info, 0);
			if (odx >= 0) {
				media_status_t status;
				if (info.size != 0) {
					clock_gettime(CLOCK_MONOTONIC, &ts);
					int64_t time = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
					int64_t timeDiff = time - start;
					if (info.presentationTimeUs > timeDiff) {
						usleep(info.presentationTimeUs - (time - start));
					}

					// video->mcodec.last_presentation_time = (double)(info.presentationTimeUs) / 1000000.0f;
				}
				status = android_funs.AMediaCodec_releaseOutputBuffer(video->video_codec, odx, true);

				assert(status == AMEDIA_OK);
				
			}
			else if (odx < -3) {
				assert(false);
			}
		}
	}
	(*kinc_android_get_activity()->vm)->DetachCurrentThread(kinc_android_get_activity()->vm);
	return NULL;
}

void kinc_video_init(kinc_video_t *video, const char *filename) {
	ANativeWindow *native_window;
#ifdef KORE_OPENGL
	kinc_android_surface_texture_init(&video->impl.surface_texture);
	native_window = video->impl.surface_texture.native_window;
#endif
#ifdef KORE_VULKAN
	return; // video decode with vulkan does not work yet
	// AImageReader_newWithUsage and a few others require api level 26 ie Oreo
	if(android_get_device_api_level() <= __ANDROID_API_O__) {
		return;
	}
	media_status_t status = android_funs.AImageReader_newWithUsage(12, 12, AIMAGE_FORMAT_PRIVATE, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 2, &video->impl.image_reader);
	assert(status == AMEDIA_OK);
	status = android_funs.AImageReader_getWindow(video->impl.image_reader, &video->impl.native_window);
	assert(status == AMEDIA_OK);
	native_window = video->impl.native_window;
#endif
	JNIEnv *env;
	(*kinc_android_get_activity()->vm)->AttachCurrentThread(kinc_android_get_activity()->vm, &env, NULL);
	AMediaExtractor *extractor = android_funs.AMediaExtractor_new();
	kinc_file_reader_t file_reader;
	if (!kinc_file_reader_open(&file_reader, filename, KINC_FILE_TYPE_ASSET)) {
		return;
	}
	int fd;
	off_t start = 0;
	off_t size;
	if (file_reader.file != NULL) {
		fd = fileno(file_reader.file);
		size = file_reader.size;
	}
	else if (file_reader.asset != NULL) {
		fd = AAsset_openFileDescriptor(file_reader.asset, &start, &size);
	}
	else {
		return;
	}
	if (fd < 0) {
		return;
	}
	media_status_t status = android_funs.AMediaExtractor_setDataSourceFd(extractor, fd, start, size);
	close(fd);
	if (status != AMEDIA_OK) {
		return;
	}

	size_t track_count = android_funs.AMediaExtractor_getTrackCount(extractor);

	AMediaFormat *audio_format = NULL;
	AMediaFormat *video_format = NULL;
	ssize_t audio_index = -1;
	ssize_t video_index = -1;
	const char *audio_mime = NULL;
	const char *video_mime = NULL;
	AMediaCodec *audio_codec = NULL;
	AMediaCodec *video_codec = NULL;

	for (size_t i = 0; i < track_count; i++) {
		AMediaFormat *format = android_funs.AMediaExtractor_getTrackFormat(extractor, i);
		assert(format);
		const char *mime_type;
		assert(android_funs.AMediaFormat_getString(format, android_funs.AMEDIAFORMAT_KEY_MIME, &mime_type));
		if (strstr(mime_type, "audio/") != NULL) {
			assert(audio_index == -1);
			audio_index = i;
			audio_format = format;
			audio_mime = mime_type;
			// android_funs.AMediaFormat_getString(format, android_funs.AMEDIA)
			kinc_log(KINC_LOG_LEVEL_INFO, "AUDIO INPUT FORMAT: %s", android_funs.AMediaFormat_toString(format));
			android_funs.AMediaExtractor_selectTrack(extractor, i);
		}
		else if (strstr(mime_type, "video/") != NULL) {
			assert(video_index == -1);
			video_index = i;
			video_format = format;
			video_mime = mime_type;
			kinc_log(KINC_LOG_LEVEL_INFO, "VIDEO INPUT FORMAT: %s", android_funs.AMediaFormat_toString(video_format));
			android_funs.AMediaExtractor_selectTrack(extractor, i);
		}
	}

	if (audio_index > -1) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Creating MediaCodec decoder for mime type %s", audio_mime);
		audio_codec = android_funs.AMediaCodec_createDecoderByType(audio_mime);
		assert(audio_codec);

		status = android_funs.AMediaCodec_configure(audio_codec, audio_format, NULL, NULL, 0);
		assert(status == AMEDIA_OK);

		kinc_log(KINC_LOG_LEVEL_INFO, "AUDIO OUTPUT FORMAT: %s", android_funs.AMediaFormat_toString(android_funs.AMediaCodec_getOutputFormat(audio_codec)));

		status = android_funs.AMediaCodec_start(audio_codec);
		assert(status == AMEDIA_OK);
	}
	if (video_index > -1) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Creating MediaCodec decoder for mime type %s", video_mime);
		video_codec = android_funs.AMediaCodec_createDecoderByType(video_mime);
		assert(video_codec);
		// AMediaCodec_setParameters()
		status = android_funs.AMediaCodec_configure(video_codec, video_format, native_window, NULL, 0);
		assert(status == AMEDIA_OK);
		kinc_log(KINC_LOG_LEVEL_INFO, "VIDEO OUTPUT FORMAT: %s", android_funs.AMediaFormat_toString(android_funs.AMediaCodec_getOutputFormat(video_codec)));
		status = android_funs.AMediaCodec_start(video_codec);
		assert(status == AMEDIA_OK);
	}

	assert(video_codec);

	video->impl.track_count = track_count;
	video->impl.extractor = extractor;
	video->impl.video_codec = video_codec;
	video->impl.video_format = video_format;
	video->impl.video_track = video_index;
	video->impl.audio_codec = audio_codec;
	video->impl.audio_format = audio_format;
	video->impl.audio_track = audio_index;
	video->impl.start = kinc_time();
	video->impl.last_presentation_time = 0;

	assert(android_funs.AMediaFormat_getInt32(video_format, AMEDIAFORMAT_KEY_WIDTH, &video->impl.width));
	assert(android_funs.AMediaFormat_getInt32(video_format, AMEDIAFORMAT_KEY_HEIGHT, &video->impl.height));
	int64_t duration = 0;
	assert(android_funs.AMediaFormat_getInt64(video_format, AMEDIAFORMAT_KEY_DURATION, &duration));

	video->impl.duration = ((double)duration) / 1000000.0;

	video->impl.position = 0;

	pthread_attr_t attr = {0};
	pthread_attr_init(&attr);
	pthread_create(&video->impl.thread, &attr, mcodec_callback, video);

	(*kinc_android_get_activity()->vm)->DetachCurrentThread(kinc_android_get_activity()->vm);
	// return;
// 	if (!kinc_android_video_mcodec_init(&video->impl, filename, native_window)) {
// #ifdef KORE_OPENGL
// 		kinc_android_surface_texture_release(&video->impl.surface_texture);
// #endif
// #ifdef KORE_VULKAN
// 		android_funs.AImageReader_delete(video->impl.image_reader);
// #endif
// 		kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to initialize video: %s", filename);
// 	}
}

void kinc_video_destroy(kinc_video_t *video) {
	// video->impl.vtable.destroy(&video->impl);
#ifdef KORE_VULKAN
	android_funs.AImageReader_delete(video->impl.image_reader);
#endif
#ifdef KORE_OPENGL
	kinc_android_surface_texture_release(&video->impl.surface_texture);
#endif
}

void kinc_video_play(kinc_video_t *video, bool loop) {
	// video->impl.vtable.play(&video->impl, loop);
}

void kinc_video_pause(kinc_video_t *video) {
	video->impl.paused = true;
}

void kinc_video_stop(kinc_video_t *video) {
	// video->impl.vtable.stop(&video->impl);
}

void kinc_video_update(kinc_video_t *video, double time) {}

int kinc_video_width(kinc_video_t *video) {
	return video->impl.width;
}

int kinc_video_height(kinc_video_t *video) {
	return video->impl.height;
}

#ifdef KORE_VULKAN
void kinc_vulkan_init_video_texture(kinc_video_texture_impl_t *tex, AHardwareBuffer *hw_buf, AHardwareBuffer_Desc hw_buf_desc);
void kinc_vulkan_delete_video_texture(kinc_video_texture_impl_t *tex);
#endif

bool kinc_video_current_image(kinc_video_t *video, kinc_video_texture_t *vtex) {
#ifdef KORE_OPENGL
	vtex->impl.oes_id = video->impl.surface_texture.oes_id;
	kinc_android_surface_texture_update_tex_image(&video->impl.surface_texture);
#endif
#ifdef KORE_VULKAN
	return false; // video decode with vulkan does not work yet
	if (video->impl.video_tex.image != NULL) {
		kinc_vulkan_delete_video_texture(&video->impl.video_tex);
		android_funs.AImage_delete(video->impl.video_tex.image);
		video->impl.video_tex.image = NULL;
	}
	AImage *image;
	media_status_t status = android_funs.AImageReader_acquireLatestImage(video->impl.image_reader, &image);
	if(status == AMEDIA_IMGREADER_NO_BUFFER_AVAILABLE) {
		return false;
	}
	assert(status == AMEDIA_OK);
    int format = 0;
    status = android_funs.AImage_getFormat(image, &format);
	assert(status == AMEDIA_OK);
	AHardwareBuffer *hw_buf;
	status = android_funs.AImage_getHardwareBuffer(image, &hw_buf);
	assert(status == AMEDIA_OK);
	video->impl.video_tex.image = image;
	video->impl.video_tex.hw_buf = hw_buf;
	AHardwareBuffer_Desc hw_buf_desc = {0};
	android_funs.AHardwareBuffer_describe(hw_buf, &hw_buf_desc);
	kinc_vulkan_init_video_texture(&video->impl.video_tex, hw_buf, hw_buf_desc);
	vtex->impl = video->impl.video_tex;
#endif
	return true;
}


double kinc_video_duration(kinc_video_t *video) {
	return video->impl.duration;
}

double kinc_video_position(kinc_video_t *video) {
	return video->impl.position;
}

bool kinc_video_finished(kinc_video_t *video) {
	return video->impl.finished;
}

bool kinc_video_paused(kinc_video_t *video) {
	return video->impl.paused;
}

void kinc_internal_video_sound_stream_init(kinc_internal_video_sound_stream_t *stream, int channel_count, int frequency) {
	stream->bufferSize = 4096 * sizeof(*stream->buffer);
	stream->buffer = malloc(stream->bufferSize);
	memset(stream->buffer, 0, stream->bufferSize);
	stream->bufferReadPosition = 0;
	stream->bufferWritePosition = 0;
}

void kinc_internal_video_sound_stream_destroy(kinc_internal_video_sound_stream_t *stream) {}

void kinc_internal_video_sound_stream_insert_data(kinc_internal_video_sound_stream_t *stream, float *data, int sample_count) {
	//assert(stream->bufferWritePosition + sample_count * sizeof(float) <= stream->bufferSize);
	//memcpy(stream->buffer + stream->bufferWritePosition, data, sample_count * sizeof(float));
	//stream->bufferWritePosition += sample_count;
}

float kinc_internal_video_sound_stream_next_sample(kinc_internal_video_sound_stream_t *stream) {
	size_t pos = stream->bufferReadPosition++;
	if (stream->bufferReadPosition > stream->bufferSize) {
		stream->bufferReadPosition = 0;
	}
	return stream->buffer[pos];
}

bool kinc_internal_video_sound_stream_ended(kinc_internal_video_sound_stream_t *stream) {
	return false;
}