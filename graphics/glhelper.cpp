#include "glhelper.h"

namespace GLHelper
{
	GLuint createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat)
	{
		GLuint texid;

		if (ID == 0)
		{
			glGenTextures(1, &texid);
		}
		else
		{
			glDeleteTextures(1, &ID);
			texid = ID;
			glGenTextures(1, &texid);
		}
		glBindTexture(target, texid);


		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// https://stackoverflow.com/questions/15405869/is-gltexstorage2d-imperative-when-auto-generating-mipmaps
		//glTexImage2D(target, 0, internalformat, w, h, 0, format, type, 0); // cretes mutable storage that requires glTexImage2D

		if (target == GL_TEXTURE_1D)
		{
			glTexStorage1D(target, levels, internalformat, w);
		}
		else if (target == GL_TEXTURE_2D)
		{
			glTexStorage2D(target, levels, internalformat, w, h); // creates immutable storage and requires glTexSubImage2D

		}
		else if (target == GL_TEXTURE_3D || d > 0)
		{
			glTexStorage3D(target, levels, internalformat, w, h, d);
		}
		return texid;
	}
	
	


}
