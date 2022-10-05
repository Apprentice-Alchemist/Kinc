#include "ogl.h"

#include <kinc/compute/compute.h>

kinc_shader_storage_buffer_t *currentStorageBuffer = NULL;

static void unset(kinc_shader_storage_buffer_t *buffer) {
	if (currentStorageBuffer == buffer) currentStorageBuffer = NULL;
}

void kinc_shader_storage_buffer_init(kinc_shader_storage_buffer_t *buffer, int indexCount, kinc_g4_vertex_data_t type) {
	buffer->impl.myCount = indexCount;
	buffer->impl.myStride = 0;
	buffer->impl.myStride += kinc_g4_vertex_data_size(type);
	if (gl_features.compute) {
		glGenBuffers(1, &buffer->impl.bufferId);
		glCheckErrors();
	}
	buffer->impl.data = (int *)malloc(sizeof(int) * indexCount);
}

void kinc_shader_storage_buffer_destroy(kinc_shader_storage_buffer_t *buffer) {
	unset(buffer);
	free(buffer->impl.data);
}

int *kinc_shader_storage_buffer_lock(kinc_shader_storage_buffer_t *buffer) {
	return buffer->impl.data;
}

void kinc_shader_storage_buffer_unlock(kinc_shader_storage_buffer_t *buffer) {
	if (gl_features.compute) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->impl.bufferId);
		glCheckErrors();
		glBufferData(GL_SHADER_STORAGE_BUFFER, buffer->impl.myCount * buffer->impl.myStride, buffer->impl.data, GL_STATIC_DRAW);
		glCheckErrors();
	}
}

void kinc_shader_storage_buffer_internal_set(kinc_shader_storage_buffer_t *buffer) {
	currentStorageBuffer = buffer;
	if (gl_features.compute) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->impl.bufferId);
		glCheckErrors();
	}
}

int kinc_shader_storage_buffer_count(kinc_shader_storage_buffer_t *buffer) {
	return buffer->impl.myCount;
}
