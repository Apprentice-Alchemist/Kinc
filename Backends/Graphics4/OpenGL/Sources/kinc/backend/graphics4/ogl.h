#pragma once

#include <kinc/global.h>

#ifdef KORE_WINDOWS
#include <GL/glew.h>

#include <GL/gl.h>
#endif

#ifdef KORE_MACOS
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif

#ifdef KORE_IOS
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/gl.h>
#endif

#ifdef KORE_ANDROID
#if KORE_ANDROID_API >= 18
#include <GLES3/gl3.h>
#endif
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#ifdef KORE_EMSCRIPTEN
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#ifdef KORE_LINUX
#ifdef KORE_OPENGL_ES
#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#endif

#ifdef KORE_PI
//#define GL_GLEXT_PROTOTYPES
#include "GLES2/gl2.h"

#include "EGL/egl.h"
#include "EGL/eglext.h"
#endif

#ifdef KORE_TIZEN
#include <gl2.h>
#endif

#ifdef KORE_WASM
#include <GL/gl.h>
#endif

#include <kinc/log.h>

#ifdef NDEBUG
#define glCheckErrors()                                                                                                                                        \
	{}
#else
#define glCheckErrors()                                                                                                                                        \
	{                                                                                                                                                          \
		GLenum code = glGetError();                                                                                                                            \
		if (code != GL_NO_ERROR) {                                                                                                                             \
			kinc_log(KINC_LOG_LEVEL_ERROR, "GL Error %d %s %d\n", code, __FILE__, __LINE__);                                                                   \
		}                                                                                                                                                      \
	}
#endif

struct gl_ctx {
	PFNGLDRAWBUFFERSPROC DrawBuffers;
	PFNGLDRAWELEMENTSINSTANCEDPROC DrawElementsInstanced;
	PFNGLVERTEXATTRIBDIVISORPROC VertexAttribDivisor;

	PFNGLGENQUERIESPROC GenQueries;
	PFNGLDELETEQUERIESPROC DeleteQueries;
	PFNGLBEGINQUERYPROC BeginQuery;
	PFNGLENDQUERYPROC EndQuery;
	PFNGLGETQUERYOBJECTUIVPROC GetQueryObjectuiv;

	PFNGLDISPATCHCOMPUTEPROC DispatchCompute;
	PFNGLMEMORYBARRIERPROC MemoryBarrier;

	PFNGLBINDIMAGETEXTUREPROC BindImageTexture;
};

struct gl_features {
	bool draw_buffers;
	bool instanced_rendering;
	bool queries;
	bool compute;
	bool vertex_arrays;
	bool bind_image_texture;
};

extern struct gl_ctx gl;
extern struct gl_features gl_features;