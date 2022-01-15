#pragma once

typedef struct kinc_microsoft_condition_variable {
	void *Ptr;
} kinc_microsoft_condition_variable_t;

typedef struct kinc_condition {
	kinc_microsoft_condition_variable_t cond;
} kinc_condition_t;