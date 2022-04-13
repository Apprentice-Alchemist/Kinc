#pragma once

#include "types.h"

/*! \file float32x4.h
    \brief Provides 128bit four-element floating point SIMD operations which are mapped to equivalent SSE or Neon operations.
*/

#ifdef __cplusplus
extern "C" {
#endif

#if defined(KINC_SSE)

static inline kinc_float32x4_t kinc_float32x4_load(float a, float b, float c, float d) {
	return _mm_set_ps(d, c, b, a);
}

static inline kinc_float32x4_t kinc_float32x4_load_all(float t) {
	return _mm_set_ps1(t);
}

static inline float kinc_float32x4_get(kinc_float32x4_t t, int index) {
	union {
		__m128 value;
		float elements[4];
	} converter;
	converter.value = t;
	return converter.elements[index];
}

static inline kinc_float32x4_t kinc_float32x4_abs(kinc_float32x4_t t) {
	__m128 mask = _mm_set_ps1(-0.f);
	return _mm_andnot_ps(mask, t);
}

static inline kinc_float32x4_t kinc_float32x4_add(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_add_ps(a, b);
}

static inline kinc_float32x4_t kinc_float32x4_div(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_div_ps(a, b);
}

static inline kinc_float32x4_t kinc_float32x4_mul(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_mul_ps(a, b);
}

static inline kinc_float32x4_t kinc_float32x4_neg(kinc_float32x4_t t) {
	__m128 negative = _mm_set_ps1(-1.0f);
	return _mm_mul_ps(t, negative);
}

static inline kinc_float32x4_t kinc_float32x4_reciprocal_approximation(kinc_float32x4_t t) {
	return _mm_rcp_ps(t);
}

static inline kinc_float32x4_t kinc_float32x4_reciprocal_sqrt_approximation(kinc_float32x4_t t) {
	return _mm_rsqrt_ps(t);
}

static inline kinc_float32x4_t kinc_float32x4_sub(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_sub_ps(a, b);
}

static inline kinc_float32x4_t kinc_float32x4_sqrt(kinc_float32x4_t t) {
	return _mm_sqrt_ps(t);
}

static inline kinc_float32x4_t kinc_float32x4_max(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_max_ps(a, b);
}

static inline kinc_float32x4_t kinc_float32x4_min(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_min_ps(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpeq(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_cmpeq_ps(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpge(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_cmpge_ps(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpgt(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_cmpgt_ps(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmple(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_cmple_ps(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmplt(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_cmplt_ps(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpneq(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_cmpneq_ps(a, b);
}

static inline kinc_float32x4_t kinc_float32x4_sel(kinc_float32x4_t a, kinc_float32x4_t b, kinc_float32x4_mask_t mask) {
	return _mm_xor_ps(b, _mm_and_ps(mask, _mm_xor_ps(a, b)));
}

#elif defined(KINC_NEON)

static inline kinc_float32x4_t kinc_float32x4_load(float a, float b, float c, float d) {
	return (kinc_float32x4_t){a, b, c, d};
}

static inline kinc_float32x4_t kinc_float32x4_load_all(float t) {
	return (kinc_float32x4_t){t, t, t, t};
}

static inline float kinc_float32x4_get(kinc_float32x4_t t, int index) {
	return t[index];
}

static inline kinc_float32x4_t kinc_float32x4_abs(kinc_float32x4_t t) {
	return vabsq_f32(t);
}

static inline kinc_float32x4_t kinc_float32x4_add(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vaddq_f32(a, b);
}

static inline kinc_float32x4_t kinc_float32x4_div(kinc_float32x4_t a, kinc_float32x4_t b) {
#if defined(KORE_SWITCH) || defined(__aarch64__)
	return vdivq_f32(a, b);
#else
	float32x4_t inv = vrecpeq_f32(b);
	float32x4_t restep = vrecpsq_f32(b, inv);
	inv = vmulq_f32(restep, inv);
	return vmulq_f32(a, inv);
#endif
}

static inline kinc_float32x4_t kinc_float32x4_mul(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vmulq_f32(a, b);
}

static inline kinc_float32x4_t kinc_float32x4_neg(kinc_float32x4_t t) {
	return vnegq_f32(t);
}

static inline kinc_float32x4_t kinc_float32x4_reciprocal_approximation(kinc_float32x4_t t) {
	return vrecpeq_f32(t);
}

static inline kinc_float32x4_t kinc_float32x4_reciprocal_sqrt_approximation(kinc_float32x4_t t) {
	return vrsqrteq_f32(t);
}

static inline kinc_float32x4_t kinc_float32x4_sub(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vsubq_f32(a, b);
}

static inline kinc_float32x4_t kinc_float32x4_sqrt(kinc_float32x4_t t) {
#if defined(KORE_SWITCH) || defined(__aarch64__)
	return vsqrtq_f32(t);
#else
	return vmulq_f32(t, vrsqrteq_f32(t));
#endif
}

static inline kinc_float32x4_t kinc_float32x4_max(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vmaxq_f32(a, b);
}

static inline kinc_float32x4_t kinc_float32x4_min(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vminq_f32(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpeq(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vceqq_f32(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpge(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vcgeq_f32(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpgt(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vcgtq_f32(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmple(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vcleq_f32(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmplt(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vcltq_f32(a, b);
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpneq(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vmvnq_u32(vceqq_f32(a, b));
}

static inline kinc_float32x4_t kinc_float32x4_sel(kinc_float32x4_t a, kinc_float32x4_t b, kinc_float32x4_mask_t mask) {
	return vbslq_f32(mask, a, b);
}

#else

static inline kinc_float32x4_t kinc_float32x4_load(float a, float b, float c, float d) {
	kinc_float32x4_t value;
	value.values[0] = a;
	value.values[1] = b;
	value.values[2] = c;
	value.values[3] = d;
	return value;
}

static inline kinc_float32x4_t kinc_float32x4_load_all(float t) {
	kinc_float32x4_t value;
	value.values[0] = t;
	value.values[1] = t;
	value.values[2] = t;
	value.values[3] = t;
	return value;
}

static inline float kinc_float32x4_get(kinc_float32x4_t t, int index) {
	return t.values[index];
}

static inline kinc_float32x4_t kinc_float32x4_abs(kinc_float32x4_t t) {
	kinc_float32x4_t value;
	value.values[0] = kinc_abs(t.values[0]);
	value.values[1] = kinc_abs(t.values[1]);
	value.values[2] = kinc_abs(t.values[2]);
	value.values[3] = kinc_abs(t.values[3]);
	return value;
}

static inline kinc_float32x4_t kinc_float32x4_add(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_t value;
	value.values[0] = a.values[0] + b.values[0];
	value.values[1] = a.values[1] + b.values[1];
	value.values[2] = a.values[2] + b.values[2];
	value.values[3] = a.values[3] + b.values[3];
	return value;
}

static inline kinc_float32x4_t kinc_float32x4_div(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_t value;
	value.values[0] = a.values[0] / b.values[0];
	value.values[1] = a.values[1] / b.values[1];
	value.values[2] = a.values[2] / b.values[2];
	value.values[3] = a.values[3] / b.values[3];
	return value;
}

static inline kinc_float32x4_t kinc_float32x4_mul(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_t value;
	value.values[0] = a.values[0] * b.values[0];
	value.values[1] = a.values[1] * b.values[1];
	value.values[2] = a.values[2] * b.values[2];
	value.values[3] = a.values[3] * b.values[3];
	return value;
}

static inline kinc_float32x4_t kinc_float32x4_neg(kinc_float32x4_t t) {
	kinc_float32x4_t value;
	value.values[0] = -t.values[0];
	value.values[1] = -t.values[1];
	value.values[2] = -t.values[2];
	value.values[3] = -t.values[3];
	return value;
}

static inline kinc_float32x4_t kinc_float32x4_reciprocal_approximation(kinc_float32x4_t t) {
	kinc_float32x4_t value;
	value.values[0] = 1.0f / t.values[0];
	value.values[1] = 1.0f / t.values[1];
	value.values[2] = 1.0f / t.values[2];
	value.values[3] = 1.0f / t.values[3];
	return value;
}

static inline kinc_float32x4_t kinc_float32x4_reciprocal_sqrt_approximation(kinc_float32x4_t t) {
	kinc_float32x4_t value;
	value.values[0] = 1.0f / kinc_sqrt(t.values[0]);
	value.values[1] = 1.0f / kinc_sqrt(t.values[1]);
	value.values[2] = 1.0f / kinc_sqrt(t.values[2]);
	value.values[3] = 1.0f / kinc_sqrt(t.values[3]);
	return value;
}

static inline kinc_float32x4_t kinc_float32x4_sub(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_t value;
	value.values[0] = a.values[0] - b.values[0];
	value.values[1] = a.values[1] - b.values[1];
	value.values[2] = a.values[2] - b.values[2];
	value.values[3] = a.values[3] - b.values[3];
	return value;
}

static inline kinc_float32x4_t kinc_float32x4_sqrt(kinc_float32x4_t t) {
	kinc_float32x4_t value;
	value.values[0] = kinc_sqrt(t.values[0]);
	value.values[1] = kinc_sqrt(t.values[1]);
	value.values[2] = kinc_sqrt(t.values[2]);
	value.values[3] = kinc_sqrt(t.values[3]);
	return value;
}

static inline kinc_float32x4_t kinc_float32x4_max(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_t value;
	value.values[0] = kinc_max(a.values[0], b.values[0]);
	value.values[1] = kinc_max(a.values[1], b.values[1]);
	value.values[2] = kinc_max(a.values[2], b.values[2]);
	value.values[3] = kinc_max(a.values[3], b.values[3]);
	return value;
}

static inline kinc_float32x4_t kinc_float32x4_min(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_t value;
	value.values[0] = kinc_min(a.values[0], b.values[0]);
	value.values[1] = kinc_min(a.values[1], b.values[1]);
	value.values[2] = kinc_min(a.values[2], b.values[2]);
	value.values[3] = kinc_min(a.values[3], b.values[3]);
	return value;
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpeq(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_mask_t mask;
	mask.values[0] = a.values[0] == b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] == b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] == b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] == b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpge(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_mask_t mask;
	mask.values[0] = a.values[0] >= b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] >= b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] >= b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] >= b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpgt(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_mask_t mask;
	mask.values[0] = a.values[0] > b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] > b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] > b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] > b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmple(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_mask_t mask;
	mask.values[0] = a.values[0] <= b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] <= b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] <= b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] <= b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmplt(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_mask_t mask;
	mask.values[0] = a.values[0] < b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] < b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] < b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] < b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_float32x4_mask_t kinc_float32x4_cmpneq(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_mask_t mask;
	mask.values[0] = a.values[0] != b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] != b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] != b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] != b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_float32x4_t kinc_float32x4_sel(kinc_float32x4_t a, kinc_float32x4_t b, kinc_float32x4_mask_t mask) {
	kinc_float32x4_t value;
	value.values[0] = mask.values[0] != 0 ? a.values[0] : b.values[0];
	value.values[1] = mask.values[1] != 0 ? a.values[1] : b.values[1];
	value.values[2] = mask.values[2] != 0 ? a.values[2] : b.values[2];
	value.values[3] = mask.values[3] != 0 ? a.values[3] : b.values[3];
	return value;
}

#endif

#ifdef __cplusplus
}
#endif
