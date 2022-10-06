#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned int texture;
	uint8_t pixfmt;
} kinc_g4_texture_impl_t;

#ifdef __cplusplus
}
#endif
