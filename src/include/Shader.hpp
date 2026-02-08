#pragma once

#ifdef GEODE_IS_MOBILE
    #undef glBindVertexArray
    #undef glDeleteVertexArrays
    #undef glGenVertexArrays
    #undef glBindVertexArrayOES
    #undef glDeleteVertexArraysOES
    #undef glGenVertexArraysOES
#endif

#ifdef GEODE_IS_ANDROID
    #include <GLES3/gl3.h>
#elifdef GEODE_IS_IOS
    #include <OpenGLES/ES3/gl.h>
#endif