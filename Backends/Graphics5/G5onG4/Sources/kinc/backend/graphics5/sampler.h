#pragma once

#include <kinc/graphics4/graphics.h>

typedef struct kinc_g5_sampler_impl {
	kinc_g4_texture_addressing_t u_addressing;
	kinc_g4_texture_addressing_t v_addressing;
	kinc_g4_texture_addressing_t w_addressing;

	kinc_g4_texture_filter_t minification_filter;
	kinc_g4_texture_filter_t magnification_filter;
	kinc_g4_mipmap_filter_t mipmap_filter;

	float lod_min_clamp;
	float lod_max_clamp;

	uint16_t max_anisotropy;

	bool is_comparison;
	kinc_g4_compare_mode_t compare_mode;
} kinc_g5_sampler_impl_t;
