#include <kinc/threads/condition.h>

#include <time.h>

void kinc_condition_init(kinc_condition_t *condition) {
	pthread_condattr_t attr;
	pthread_condattr_init(&attr);
	pthread_cond_init(&condition->impl.cond, &attr);
	pthread_condattr_destroy(&attr);
}

void kinc_condition_destroy(kinc_condition_t *condition) {
	pthread_cond_destroy(&condition->impl.cond);
}

void kinc_condition_wait(kinc_condition_t *condition, kinc_mutex_t *mutex) {
	pthread_cond_wait(&condition->impl.cond, &mutex->impl.mutex);
}

bool kinc_condition_try_to_wait(kinc_condition_t *condition, kinc_mutex_t *mutex, double timeout) {
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	t.tv_sec += (time_t)timeout;
	t.tv_nsec += (long)((timeout - (long)timeout) * 1e9);
	return pthread_cond_timedwait(&condition->impl.cond, &mutex->impl.mutex, &t) == 0;
}

void kinc_condition_signal(kinc_condition_t *condition) {
	pthread_cond_signal(&condition->impl.cond);
}

void kinc_condition_broadcast(kinc_condition_t *condition) {
	pthread_cond_broadcast(&condition->impl.cond);
}