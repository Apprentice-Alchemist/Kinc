#include <kinc/threads/condition.h>

void kinc_condition_init(kinc_condition_t *condition) {
	InitializeConditionVariable(&condition->impl.cond);
}

void kinc_condition_destroy(kinc_condition_t *condition) {}

void kinc_condition_wait(kinc_condition_t *condition, kinc_mutex_t *mutex) {
	SleepConditionVariableCS(&condition->impl.cond, &mutex->impl.criticalSection, INFINITE);
}

bool kinc_condition_try_to_wait(kinc_condition_t *condition, kinc_mutex_t *mutex, double timeout) {
	SleepConditionVariableCS(&condition->impl.cond, &mutex->impl.criticalSection, (DWORD)(timeout * 1000));
}

void kinc_condition_signal(kinc_condition_t *condition) {
	WakeConditionVariable(&condition->impl.cond);
}

void kinc_condition_broadcast(kinc_condition_t *condition) {
	WakeAllConditionVariable(&condition->impl.cond);
}