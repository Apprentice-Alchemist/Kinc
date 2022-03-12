#include "kinc/log.h"
#include "kinc/memory.h"
#include <kinc/hardware_video.h>

#include <va/va.h>
#include <va/va_drmcommon.h>
#include <va/va_wayland.h>
#include <vulkan/vulkan_core.h>

static VADisplay *display = NULL;
static int va_major = 0;
static int va_minor = 0;

inline void VA_CHECK(VAStatus status) {
	if (status != VA_STATUS_SUCCESS) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "VA-API Error: %x", status);
	}
}

void kinc_hardware_video_init() {
	display = vaGetDisplayWl(NULL);
	vaInitialize(display, &va_major, &va_minor);
	int max_num_profiles = vaMaxNumProfiles(display);
	int num_profiles = 0;
	VAProfile *profile_list = kinc_allocate(sizeof *profile_list * max_num_profiles);
	vaQueryConfigProfiles(display, profile_list, &num_profiles);
	for (int i = 0; i < num_profiles; i++) {
		VAProfile profile = profile_list[i];
		int max_num_entrypoints = vaMaxNumEntrypoints(display);
		VAEntrypoint *entrypoint_list = kinc_allocate(sizeof *entrypoint_list * max_num_entrypoints);
		int num_entrypoints = 0;
		vaQueryConfigEntrypoints(display, profile, entrypoint_list, &num_entrypoints);
	}
	// vaQueryConfigAttributes(VADisplay dpy, VAConfigID config_id, VAProfile *profile, VAEntrypoint *entrypoint, VAConfigAttrib *attrib_list, int *num_attribs)
	VASurfaceID surface;
	vaCreateBuffer;
	VASurfaceAttrib a;
	// vaQuerySurfaceAttributes(display, config, VASurfaceAttrib *attrib_list, unsigned int *num_attribs)
	vaCreateSurfaces(display, VA_RT_FORMAT_YUV444, 0, 0, &surface, 1, NULL, 0);
	// vaCreateSurfaces(display, VA_, unsigned int width, unsigned int height, VASurfaceID *surfaces, unsigned int num_surfaces, VASurfaceAttrib *attrib_list,
	// unsigned int num_attribs) vaCreateBuffer(VADisplay dpy, VAContextID context, VABufferType type, unsigned int size, unsigned int num_elements, void *data,
	// VABufferID *buf_id) vaExportSurfaceHandle(display, render_targets[0], VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2, VA_EXPORT_SURFACE_READ_ONLY |
	// VA_EXPORT_SURFACE_SEPARATE_LAYERS,
	//                       0);
	// for (;;) {
	// 	// Parse one frame and decode
	// 	vaBeginPicture(display, context, render_targets[0]);
	// 	vaRenderPicture(display, context, NULL, 0);
	// 	vaEndPicture(display, context);
	// 	vaSyncSurface(display, render_targets[0]);
	// 	VASurfaceStatus status;
	// 	vaQuerySurfaceStatus(display, render_targets[0], &status);
	// 	if (status != VASurfaceReady) {
	// 		kinc_log(KINC_LOG_LEVEL_ERROR, "VA API surface not ready.");
	// 	}
	// 	else {
	// 	}
	// }
}

KINC_FUNC void kinc_hardware_decoder_init(kinc_hardware_video_decoder_t *decoder, kinc_hardware_video_profile_t *profile, int width, int height) {
	VAConfigAttrib attrib_list[] = {};
	int num_attribs = 0;
	vaCreateConfig(display, VAProfileH264Main, VAEntrypointVLD, attrib_list, num_attribs, &decoder->impl.config_id);
	int flag = 0;
	VASurfaceID *render_targets = NULL;
	int num_render_targets = 0;
	vaCreateContext(display, decoder->impl.config_id, width, height, flag, render_targets, num_render_targets, &decoder->impl.context_id);
}

KINC_FUNC void kinc_hardware_decoder_destroy(kinc_hardware_video_decoder_t *decoder) {
	vaDestroyContext(display, decoder->impl.context_id);
	vaDestroyConfig(display, decoder->impl.config_id);
	decoder->impl.context_id = VA_INVALID_ID;
	decoder->impl.config_id = VA_INVALID_ID;
}

// KINC_FUNC void kinc_hardware_video_buffer_init(kinc_hardware_video_buffer_t *buffer);
// KINC_FUNC void kinc_hardware_video_buffer_destroy(kinc_hardware_video_buffer_t *buffer);

// KINC_FUNC void kinc_hardware_decoder_begin_frame(kinc_hardware_video_decoder_t *decoder);
// KINC_FUNC void kinc_hardware_decoder_submit_buffer(kinc_hardware_video_decoder_t *decoder, kinc_hardware_video_buffer_t *buffer);
// KINC_FUNC void kinc_hardware_decoder_end_frame(kinc_hardware_video_decoder_t *decoder);