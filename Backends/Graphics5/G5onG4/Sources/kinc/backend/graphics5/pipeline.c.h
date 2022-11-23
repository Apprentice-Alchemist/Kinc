#include <kinc/graphics4/constantlocation.h>
#include <kinc/graphics5/constantlocation.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/log.h>

#include <kinc/system.h>

#include <string.h>
#include <assert.h>

void kinc_g4_pipeline_get_constant_locations(kinc_g4_pipeline_t *state, kinc_g4_constant_location_t *vertex_locations,
                                             kinc_g4_constant_location_t *fragment_locations, int *vertex_sizes, int *fragment_sizes, int *max_vertex,
                                             int *max_fragment);

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline) {
	kinc_g4_pipeline_init(&pipeline->impl.pipe);
	// int vertex_count = 0;
	// int fragment_count = 0;
	// kinc_g4_pipeline_get_constant_locations(&pipeline->impl.pipe, NULL, NULL, NULL, NULL, &vertex_count, &fragment_count);
	// pipeline->impl.vertex_locations = malloc(vertex_count * sizeof(kinc_g4_constant_location_t));
	// pipeline->impl.fragment_locations = malloc(fragment_count * sizeof(kinc_g4_constant_location_t));
	// pipeline->impl.vertex_sizes = malloc(vertex_count * sizeof(int));
	// pipeline->impl.fragment_sizes = malloc(fragment_count * sizeof(int));
	// if (pipeline->impl.vertex_locations == NULL || pipeline->impl.fragment_locations == NULL || pipeline->impl.vertex_sizes == NULL ||
	//     pipeline->impl.fragment_sizes == NULL) {
	// 	kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to allocate pipeline reflection data.");
	// 	kinc_stop();
	// }
	// else {
	// 	pipeline->impl.vertex_location_count = vertex_count;
	// 	pipeline->impl.fragment_location_count = fragment_count;
	// 	kinc_g4_pipeline_get_constant_locations(&pipeline->impl.pipe, pipeline->impl.vertex_locations, pipeline->impl.fragment_locations,
	// pipeline->impl.vertex_sizes, pipeline->impl.fragment_sizes, &vertex_count, 	                                        &fragment_count);
	// }
}

void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipe) {
	kinc_g4_pipeline_destroy(&pipe->impl.pipe);
}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipe, const char *name) {
	kinc_g5_constant_location_t location;
	location.impl.location = kinc_g4_pipeline_get_constant_location(&pipe->impl.pipe, name);
	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipe, const char *name) {
	kinc_g4_texture_unit_t g4_unit = kinc_g4_pipeline_get_texture_unit(&pipe->impl.pipe, name);

	assert(KINC_G4_SHADER_TYPE_COUNT == KINC_G5_SHADER_TYPE_COUNT);
	kinc_g5_texture_unit_t g5_unit;
	memcpy(&g5_unit.stages[0], &g4_unit.stages[0], KINC_G5_SHADER_TYPE_COUNT * sizeof(int));

	return g5_unit;
}

kinc_g4_compare_mode_t kinc_internal_convert_compare_mode(kinc_g5_compare_mode_t mode);

static kinc_g4_stencil_action_t convert_stencil_action(kinc_g5_stencil_action_t action) {
	switch (action) {
	case KINC_G5_STENCIL_ACTION_KEEP:
		return KINC_G4_STENCIL_KEEP;
	case KINC_G5_STENCIL_ACTION_ZERO:
		return KINC_G4_STENCIL_ZERO;
	case KINC_G5_STENCIL_ACTION_REPLACE:
		return KINC_G4_STENCIL_REPLACE;
	case KINC_G5_STENCIL_ACTION_INCREMENT:
		return KINC_G4_STENCIL_INCREMENT;
	case KINC_G5_STENCIL_ACTION_INCREMENT_WRAP:
		return KINC_G4_STENCIL_INCREMENT_WRAP;
	case KINC_G5_STENCIL_ACTION_DECREMENT:
		return KINC_G4_STENCIL_DECREMENT;
	case KINC_G5_STENCIL_ACTION_DECREMENT_WRAP:
		return KINC_G4_STENCIL_DECREMENT_WRAP;
	case KINC_G5_STENCIL_ACTION_INVERT:
		return KINC_G4_STENCIL_INVERT;
	}
}

static kinc_g4_blending_factor_t convert_blending_factor(kinc_g5_blending_factor_t factor) {
	switch (factor) {
	case KINC_G5_BLEND_ONE:
		return KINC_G4_BLEND_ONE;
	case KINC_G5_BLEND_ZERO:
		return KINC_G4_BLEND_ZERO;
	case KINC_G5_BLEND_SOURCE_ALPHA:
		return KINC_G4_BLEND_SOURCE_ALPHA;
	case KINC_G5_BLEND_DEST_ALPHA:
		return KINC_G4_BLEND_DEST_ALPHA;
	case KINC_G5_BLEND_INV_SOURCE_ALPHA:
		return KINC_G4_BLEND_INV_SOURCE_ALPHA;
	case KINC_G5_BLEND_INV_DEST_ALPHA:
		return KINC_G4_BLEND_INV_DEST_ALPHA;
	case KINC_G5_BLEND_SOURCE_COLOR:
		return KINC_G4_BLEND_SOURCE_COLOR;
	case KINC_G5_BLEND_DEST_COLOR:
		return KINC_G4_BLEND_DEST_COLOR;
	case KINC_G5_BLEND_INV_SOURCE_COLOR:
		return KINC_G4_BLEND_INV_SOURCE_COLOR;
	case KINC_G5_BLEND_INV_DEST_COLOR:
		return KINC_G4_BLEND_INV_DEST_COLOR;
	case KINC_G5_BLEND_CONSTANT:
		return KINC_G4_BLEND_CONSTANT;
	case KINC_G5_BLEND_INV_CONSTANT:
		return KINC_G4_BLEND_INV_CONSTANT;
	}
}

static kinc_g4_blending_operation_t convert_blending_operation(kinc_g5_blending_operation_t op) {
	switch (op) {
	case KINC_G5_BLENDOP_ADD:
		return KINC_G4_BLENDOP_ADD;
	case KINC_G5_BLENDOP_SUBTRACT:
		return KINC_G4_BLENDOP_SUBTRACT;
	case KINC_G5_BLENDOP_REVERSE_SUBTRACT:
		return KINC_G4_BLENDOP_REVERSE_SUBTRACT;
	case KINC_G5_BLENDOP_MIN:
		return KINC_G4_BLENDOP_MIN;
	case KINC_G5_BLENDOP_MAX:
		return KINC_G4_BLENDOP_MAX;
	}
}

kinc_g4_render_target_format_t kinc_internal_convert_render_target_format(kinc_g5_render_target_format_t format) {
	switch (format) {
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT:
		return KINC_G4_RENDER_TARGET_FORMAT_32BIT;
	case KINC_G5_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		return KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		return KINC_G4_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		return KINC_G4_RENDER_TARGET_FORMAT_128BIT_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_16BIT_DEPTH:
		return KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH;
	case KINC_G5_RENDER_TARGET_FORMAT_8BIT_RED:
		return KINC_G4_RENDER_TARGET_FORMAT_8BIT_RED;
	case KINC_G5_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		return KINC_G4_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT;
	}
}
void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipe) {
	for (int i = 0; i < 16; ++i) {
		pipe->impl.pipe.input_layout[i] = pipe->inputLayout[i];
	}
	pipe->impl.pipe.vertex_shader = &pipe->vertexShader->impl.shader;
	pipe->impl.pipe.fragment_shader = &pipe->fragmentShader->impl.shader;
	pipe->impl.pipe.geometry_shader = &pipe->geometryShader->impl.shader;
	pipe->impl.pipe.tessellation_control_shader = &pipe->tessellationControlShader->impl.shader;
	pipe->impl.pipe.tessellation_evaluation_shader = &pipe->tessellationEvaluationShader->impl.shader;

	switch (pipe->cullMode) {
	case KINC_G5_CULL_MODE_CLOCKWISE:
		pipe->impl.pipe.cull_mode = KINC_G4_CULL_CLOCKWISE;
	case KINC_G5_CULL_MODE_COUNTERCLOCKWISE:
		pipe->impl.pipe.cull_mode = KINC_G4_CULL_COUNTER_CLOCKWISE;
	case KINC_G5_CULL_MODE_NEVER:
		pipe->impl.pipe.cull_mode = KINC_G4_CULL_NOTHING;
	}

	pipe->impl.pipe.depth_write = pipe->depthWrite;
	pipe->impl.pipe.depth_mode = kinc_internal_convert_compare_mode(pipe->depthMode);
	// pipe->impl.pipe.stencil_mode
	// TODO: stencil modes
	pipe->impl.pipe.stencil_reference_value = pipe->stencilReferenceValue;
	pipe->impl.pipe.stencil_read_mask = pipe->stencilReadMask;
	pipe->impl.pipe.stencil_write_mask = pipe->stencilWriteMask;

	pipe->impl.pipe.blend_source = convert_blending_factor(pipe->blend_source);
	pipe->impl.pipe.blend_destination = convert_blending_factor(pipe->blend_destination);
	pipe->impl.pipe.blend_operation = convert_blending_operation(pipe->blend_operation);
	pipe->impl.pipe.alpha_blend_source = convert_blending_factor(pipe->alpha_blend_source);
	pipe->impl.pipe.alpha_blend_destination = convert_blending_factor(pipe->alpha_blend_destination);
	pipe->impl.pipe.alpha_blend_operation = convert_blending_operation(pipe->alpha_blend_operation);

	pipe->impl.pipe.color_attachment_count = pipe->colorAttachmentCount;
	for (int i = 0; i < pipe->colorAttachmentCount; i++) {
		pipe->impl.pipe.color_attachment[i] = kinc_internal_convert_render_target_format(pipe->colorAttachment[i]);
	}

	pipe->impl.pipe.depth_attachment_bits = pipe->depthAttachmentBits;
	pipe->impl.pipe.stencil_attachment_bits = pipe->stencilAttachmentBits;

	pipe->impl.pipe.conservative_rasterization = pipe->conservativeRasterization;

	memcpy(pipe->impl.pipe.color_write_mask_red, &pipe->colorWriteMaskRed, sizeof(bool) * 8);
	memcpy(pipe->impl.pipe.color_write_mask_green, &pipe->colorWriteMaskGreen, sizeof(bool) * 8);
	memcpy(pipe->impl.pipe.color_write_mask_blue, &pipe->colorWriteMaskBlue, sizeof(bool) * 8);
	memcpy(pipe->impl.pipe.color_write_mask_alpha, &pipe->colorWriteMaskAlpha, sizeof(bool) * 8);
	kinc_g4_pipeline_compile(&pipe->impl.pipe);
}
