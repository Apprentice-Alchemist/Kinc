#include "kinc/graphics4/pipeline.h"
#include <kinc/graphics5/sampler.h>

static kinc_g4_texture_addressing_t convert_addressing(kinc_g5_texture_addressing_t a) {
	switch (a) {
	case KINC_G5_TEXTURE_ADDRESSING_REPEAT:
		return KINC_G4_TEXTURE_ADDRESSING_REPEAT;
	case KINC_G5_TEXTURE_ADDRESSING_MIRROR:
		return KINC_G4_TEXTURE_ADDRESSING_MIRROR;
	case KINC_G5_TEXTURE_ADDRESSING_CLAMP:
		return KINC_G4_TEXTURE_ADDRESSING_CLAMP;
	case KINC_G5_TEXTURE_ADDRESSING_BORDER:
		return KINC_G4_TEXTURE_ADDRESSING_BORDER;
	}
}

static kinc_g4_texture_filter_t convert_texture_filter(kinc_g5_texture_filter_t filter) {
	switch (filter) {
	case KINC_G5_TEXTURE_FILTER_POINT:
		return KINC_G4_TEXTURE_FILTER_POINT;
	case KINC_G5_TEXTURE_FILTER_LINEAR:
		return KINC_G4_TEXTURE_FILTER_LINEAR;
	case KINC_G5_TEXTURE_FILTER_ANISOTROPIC:
		return KINC_G4_TEXTURE_FILTER_ANISOTROPIC;
	}
}

static kinc_g4_mipmap_filter_t convert_mipmap_filter(kinc_g5_mipmap_filter_t filter) {
	switch (filter) {
	case KINC_G5_MIPMAP_FILTER_NONE:
		return KINC_G4_MIPMAP_FILTER_NONE;
	case KINC_G5_MIPMAP_FILTER_POINT:
		return KINC_G4_MIPMAP_FILTER_POINT;
	case KINC_G5_MIPMAP_FILTER_LINEAR:
		return KINC_G4_MIPMAP_FILTER_LINEAR;
	}
}

kinc_g4_compare_mode_t kinc_internal_convert_compare_mode(kinc_g5_compare_mode_t mode) {
	switch (mode) {
	case KINC_G5_COMPARE_MODE_ALWAYS:
		return KINC_G4_COMPARE_ALWAYS;
	case KINC_G5_COMPARE_MODE_NEVER:
		return KINC_G4_COMPARE_NEVER;
	case KINC_G5_COMPARE_MODE_EQUAL:
		return KINC_G4_COMPARE_EQUAL;
	case KINC_G5_COMPARE_MODE_NOT_EQUAL:
		return KINC_G4_COMPARE_NOT_EQUAL;
	case KINC_G5_COMPARE_MODE_LESS:
		return KINC_G4_COMPARE_LESS;
	case KINC_G5_COMPARE_MODE_LESS_EQUAL:
		return KINC_G4_COMPARE_LESS_EQUAL;
	case KINC_G5_COMPARE_MODE_GREATER:
		return KINC_G4_COMPARE_GREATER;
	case KINC_G5_COMPARE_MODE_GREATER_EQUAL:
		return KINC_G4_COMPARE_GREATER_EQUAL;
	}
}

void kinc_g5_sampler_init(kinc_g5_sampler_t *sampler, const kinc_g5_sampler_options_t *options) {
	sampler->impl.u_addressing = convert_addressing(options->u_addressing);
	sampler->impl.v_addressing = convert_addressing(options->v_addressing);
	sampler->impl.w_addressing = convert_addressing(options->w_addressing);

	sampler->impl.magnification_filter = convert_texture_filter(options->magnification_filter);
	sampler->impl.minification_filter = convert_texture_filter(options->minification_filter);
	sampler->impl.mipmap_filter = convert_mipmap_filter(options->mipmap_filter);

	sampler->impl.lod_min_clamp = options->lod_min_clamp;
	sampler->impl.lod_max_clamp = options->lod_max_clamp;

	sampler->impl.max_anisotropy = options->max_anisotropy;

	sampler->impl.is_comparison = options->is_comparison;
	sampler->impl.compare_mode = kinc_internal_convert_compare_mode(options->compare_mode);
}

void kinc_g5_sampler_destroy(kinc_g5_sampler_t *sampler) {}
