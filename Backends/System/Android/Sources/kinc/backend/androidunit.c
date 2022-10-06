#include "Android.h"

#include "audio.c.h"
#include "display.c.h"
#include "system.c.h"
#include "video.c.h"
#include "window.c.h"

struct android_funs android_funs = {0};
int device_api_level = -1;

#ifdef KORE_OPENGL
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

void kinc_android_surface_texture_init(struct android_surface_texture *tex) {
	glGenTextures(1, &tex->oes_id);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex->oes_id);
	glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	JNIEnv *env;
	(*kinc_android_get_activity()->vm)->AttachCurrentThread(kinc_android_get_activity()->vm, &env, NULL);
	jclass surface_texture_class = kinc_android_find_class(env, "android.graphics.SurfaceTexture");
	jmethodID constructor = (*env)->GetMethodID(env, surface_texture_class, "<init>", "(I)V");
	tex->surface_texture = (*env)->NewObject(env, surface_texture_class, constructor, tex->oes_id);
	tex->surface_texture = (*env)->NewGlobalRef(env, tex->surface_texture);

	jclass surface_class = kinc_android_find_class(env, "android.view.Surface");
	constructor = (*env)->GetMethodID(env, surface_class, "<init>", "(Landroid/graphics/SurfaceTexture;)V");
	tex->surface = (*env)->NewObject(env, surface_class, constructor, tex->surface_texture);
	tex->surface = (*env)->NewGlobalRef(env, tex->surface);

	tex->native_window = ANativeWindow_fromSurface(env, tex->surface);

	(*kinc_android_get_activity()->vm)->DetachCurrentThread(kinc_android_get_activity()->vm);
}

void kinc_android_surface_texture_update_tex_image(struct android_surface_texture *tex) {
	JNIEnv *env;
	(*kinc_android_get_activity()->vm)->AttachCurrentThread(kinc_android_get_activity()->vm, &env, NULL);
	jclass surface_texture_class = kinc_android_find_class(env, "android.graphics.SurfaceTexture");
	jmethodID method = (*env)->GetMethodID(env, surface_texture_class, "updateTexImage", "()V");
	(*env)->CallVoidMethod(env, tex->surface_texture, method);

	(*kinc_android_get_activity()->vm)->DetachCurrentThread(kinc_android_get_activity()->vm);
}

void kinc_android_surface_texture_release(struct android_surface_texture *tex) {
	JNIEnv *env;
	(*kinc_android_get_activity()->vm)->AttachCurrentThread(kinc_android_get_activity()->vm, &env, NULL);

	ANativeWindow_release(tex->native_window);
	jclass surface_class = kinc_android_find_class(env, "android.view.Surface");
	jmethodID method = (*env)->GetMethodID(env, surface_class, "release", "()V");
	(*env)->CallVoidMethod(env, tex->surface, method);
	(*env)->DeleteGlobalRef(env, tex->surface);

	jclass surface_texture_class = kinc_android_find_class(env, "android.graphics.SurfaceTexture");
	method = (*env)->GetMethodID(env, surface_texture_class, "release", "()V");
	(*env)->CallVoidMethod(env, tex->surface_texture, method);
	(*env)->DeleteGlobalRef(env, tex->surface_texture);


	glDeleteTextures(1, &tex->oes_id);

	(*kinc_android_get_activity()->vm)->DetachCurrentThread(kinc_android_get_activity()->vm);
}
#endif