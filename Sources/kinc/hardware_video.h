#pragma once

#include <kinc/global.h>

#include <kinc/backend/hardware_video.h>

typedef struct kinc_hardware_video_decoder {
	kinc_hardware_video_decoder_impl_t impl;
} kinc_hardware_video_decoder_t;

typedef enum kinc_video_codec {
	KINC_VIDEO_CODEC_H264,
	KINC_VIDEO_CODEC_MPEG2,
	KINC_VIDEO_CODE_VC1,
	KINC_VIDEO_CODEC_VP8,
	KINC_VIDEO_CODEC_VP9,
	KINC_VIDEO_CODEC_HVEC
} kinc_video_codec_t;

typedef struct kinc_hardware_video_profile {
	kinc_video_codec_t codec;
} kinc_hardware_video_profile_t;

typedef struct kinc_hardware_video_buffer {

} kinc_hardware_video_buffer_t;

KINC_FUNC void kinc_hardware_video_init();

KINC_FUNC void kinc_hardware_decoder_init(kinc_hardware_video_decoder_t *decoder, kinc_hardware_video_profile_t *profile, int width, int height);
KINC_FUNC void kinc_hardware_decoder_destroy(kinc_hardware_video_decoder_t *decoder);

KINC_FUNC void kinc_hardware_video_buffer_init(kinc_hardware_video_buffer_t *buffer);
KINC_FUNC void kinc_hardware_video_buffer_destroy(kinc_hardware_video_buffer_t *buffer);

KINC_FUNC void kinc_hardware_decoder_begin_frame(kinc_hardware_video_decoder_t *decoder);
KINC_FUNC void kinc_hardware_decoder_submit_buffer(kinc_hardware_video_decoder_t *decoder, kinc_hardware_video_buffer_t *buffer);
KINC_FUNC void kinc_hardware_decoder_end_frame(kinc_hardware_video_decoder_t *decoder);
// KINC_FUNC void kinc_hardware_decoder_get_texture(kinc_hardware_video_decoder_t *decoder, )