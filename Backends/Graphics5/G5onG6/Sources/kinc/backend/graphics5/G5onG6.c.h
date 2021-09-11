#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/pipeline.h>

void kinc_g5_destroy(int windowId) {}

void kinc_internal_g5_resize(int window, int width, int height) {}

void kinc_g5_init(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
}

void kinc_g5_set_texture_addressing(kinc_g5_texture_unit_t unit, kinc_g5_texture_direction_t dir, kinc_g5_texture_addressing_t addressing) {}

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window) {}

void kinc_g5_end(int window) {}

bool kinc_g5_swap_buffers() {
	return true;
}

void kinc_g5_flush() {}

void kinc_g5_set_texture_operation(kinc_g5_texture_operation_t operation, kinc_g5_texture_argument_t arg1, kinc_g5_texture_argument_t arg2) {}

void kinc_g5_set_texture_magnification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {}

void kinc_g5_set_texture_minification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {}

void kinc_g5_set_texture_mipmap_filter(kinc_g5_texture_unit_t texunit, kinc_g5_mipmap_filter_t filter) {}

int kinc_g5_max_bound_textures(void) {
	return 0;
}

bool kinc_g5_render_targets_inverted_y() {
	return false;
}
bool kinc_g5_non_pow2_textures_qupported() {
	return true;
}

void kinc_g5_set_render_target_face(kinc_g5_render_target_t *texture, int face) {}

void kinc_g5_set_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {}

void kinc_g5_set_image_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {}

bool kinc_g5_init_occlusion_query(unsigned *occlusionQuery) {
	return false;
}

void kinc_g5_delete_occlusion_query(unsigned occlusionQuery) {}

void kinc_g5_render_occlusion_query(unsigned occlusionQuery, int triangles) {}

bool kinc_g5_are_query_results_available(unsigned occlusionQuery) {
	return false;
}

void kinc_g5_get_query_result(unsigned occlusionQuery, unsigned *pixelCount) {}
