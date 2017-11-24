#pragma once
#include <GL/glew.h>

class VBO
{
public:
	VBO(GLenum);
	~VBO();

	void bind() const;
	void unbind() const;
	void setData(GLvoid *data, int size);

	GLuint buffer_id;
	GLenum buffer_type;
};

