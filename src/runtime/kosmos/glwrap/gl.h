#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/glew.h>
#pragma comment(lib, "glew32s.lib")

#else

#include <OpenGL/gl.h>

#endif
