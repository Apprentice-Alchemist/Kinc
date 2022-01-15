#pragma once

#include <kinc/global.h>
#include <kinc/threads/mutex.h>

#include <kinc/backend/condition.h>

#include <stdbool.h>

/*! \file condition.h
    \brief Provides condition variables.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_condition {
	kinc_condition_impl_t impl;
} kinc_condition_t;

KINC_FUNC void kinc_condition_init(kinc_condition_t *condition);

KINC_FUNC void kinc_condition_destroy(kinc_condition_t *condition);

KINC_FUNC void kinc_condition_wait(kinc_condition_t *condition, kinc_mutex_t *mutex);
KINC_FUNC bool kinc_condition_try_to_wait(kinc_condition_t *condition, kinc_mutex_t *mutex, double timeout);

KINC_FUNC void kinc_condition_signal(kinc_condition_t *condition);

KINC_FUNC void kinc_condition_broadcast(kinc_condition_t *condition);

#ifdef __cplusplus
}
#endif
