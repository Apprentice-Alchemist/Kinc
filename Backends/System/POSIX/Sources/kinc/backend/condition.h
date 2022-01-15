#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_condition_impl {
	pthread_cond_t cond;
} kinc_condition_impl_t;

#ifdef __cplusplus
}
#endif