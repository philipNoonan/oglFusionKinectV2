#ifndef GLUTILS_H
#define GLUTILS_H

//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

//#include "cookbookogl.h"

namespace GLUtils
{
    int checkForOpenGLError(const char *, int);
    
    void dumpGLInfo(bool dumpExtensions = false);
    

}

#endif // GLUTILS_H
