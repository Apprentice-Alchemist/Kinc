#pragma once

// The android NDK seems to be slightly broken?
struct AMediaCodecOnAsyncNotifyCallback;
typedef struct AMediaCodecOnAsyncNotifyCallback AMediaCodecOnAsyncNotifyCallback;

#include <android_native_app_glue.h>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaError.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct android_funs {
	bool loaded;
	#define DEFINE_FUN(name, lib, api_level, def) def;
	#define DEFINE_VAR(name, lib, api_level, type) type name;
	#include "android_funs.h"
	#undef DEFINE_FUN
	#undef DEFINE_VAR
};

extern struct android_funs android_funs;
extern int device_api_level;

// name in usual Java syntax (points, no slashes)
jclass kinc_android_find_class(JNIEnv *env, const char *name);

ANativeActivity *kinc_android_get_activity(void);

AAssetManager *kinc_android_get_asset_manager(void);

#ifdef KORE_OPENGL
struct android_surface_texture {
	jobject surface_texture;
	jobject surface;
	ANativeWindow *native_window;
	unsigned int oes_id;
};

void kinc_android_surface_texture_init(struct android_surface_texture *tex);
void kinc_android_surface_texture_update_tex_image(struct android_surface_texture *tex);
void kinc_android_surface_texture_release(struct android_surface_texture *tex);
#endif

#ifdef __cplusplus
}
#endif
