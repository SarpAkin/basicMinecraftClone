#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <assert.h>

#ifdef ASSERT
static_assert("ASSERT is defined!\n");
#endif

#define DEBUG

#ifdef DEBUG
#define ASSERT(x) assert(x);
#define GLCALL(x) clearGLErrors();\
    x;\
    assert(logGLCalls());

#else
    #define GLCALL(x) x;
    #define ASSERT(x) x;
#endif

void clearGLErrors();
bool logGLCalls();