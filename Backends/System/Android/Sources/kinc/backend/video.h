#pragma once

#include "Android.h"

#include <kinc/graphics5/sampler.h>
#include <kinc/graphics5/texture.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct AAsset;
struct AImageReader;
struct ANativeWindow;
struct AMediaFormat;
struct AMediaCodec;
struct AMediaExtractor;

typedef struct kinc_internal_video_sound_stream {
	float *buffer;
	int bufferSize;
	int bufferWritePosition;
	int bufferReadPosition;
} kinc_internal_video_sound_stream_t;

typedef struct kinc_video_mcodec {

} kinc_video_mcodec_t;

typedef struct kinc_video_texture_impl {
#ifdef KORE_VULKAN
	kinc_g5_texture_t texture;
	kinc_g5_sampler_t sampler;
	struct AImage *image;
	struct AHardwareBuffer *hw_buf;
#endif
#ifdef KORE_OPENGL
	unsigned int oes_id;
#endif
} kinc_video_texture_impl_t;

struct android_surface_texture;

typedef struct kinc_video_impl {
	struct AImageReader *reader;
	struct ANativeWindow *window;
	struct AMediaFormat *video_format;
	struct AMediaFormat *audio_format;
	struct AMediaCodec *video_codec;
	struct AMediaCodec *audio_codec;
	struct AMediaExtractor *extractor;
	int track_count;
	size_t video_track;
	size_t audio_track;
	double last_presentation_time;
	double start;
	pthread_t thread;
	bool paused;
	bool finished;

	int width;
	int height;
	double duration;
	double position;

	kinc_internal_video_sound_stream_t sound_stream;

#ifdef KORE_OPENGL
	struct android_surface_texture surface_texture;
#endif
#ifdef KORE_VULKAN
	struct AImageReader *image_reader;
	struct ANativeWindow *native_window;

	kinc_video_texture_impl_t video_tex;
#endif
} kinc_video_impl_t;

void kinc_internal_video_sound_stream_init(kinc_internal_video_sound_stream_t *stream, int channel_count, int frequency);

void kinc_internal_video_sound_stream_destroy(kinc_internal_video_sound_stream_t *stream);

void kinc_internal_video_sound_stream_insert_data(kinc_internal_video_sound_stream_t *stream, float *data, int sample_count);

float kinc_internal_video_sound_stream_next_sample(kinc_internal_video_sound_stream_t *stream);

bool kinc_internal_video_sound_stream_ended(kinc_internal_video_sound_stream_t *stream);

#ifdef __cplusplus
}
#endif
