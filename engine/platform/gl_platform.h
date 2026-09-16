#pragma once

// Cross-platform OpenGL headers.
// Desktop: OpenGL compatibility profile via GLEW + GLFW.
// Android: OpenGL ES 1.1 (same fixed-function matrix/state API as desktop's
// compatibility profile, minus glBegin/glEnd immediate mode) via EGL.
#if defined(__ANDROID__)
    #include <GLES/gl.h>
    #include <GLES/glext.h>
#else
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
#endif
