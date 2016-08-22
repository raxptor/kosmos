#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/glew.h>
#else

#ifdef KOSMOS_GL3
#include <OpenGL/gl3.h>
#else
#include <OpenGL/gl.h>
#endif

#endif
