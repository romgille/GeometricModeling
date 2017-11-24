#include "VAO.h"
#include <iostream>
#include <glm/vec3.hpp>


VAO::VAO()
{
	glGenVertexArrays(1, &id);
	indices_buffer = nullptr;
	num_triangles = 0;

	assert(glBindBuffer != 0);
	assert(glBindVertexArray != 0);
	assert(glBufferData != 0);
	assert(glClear != 0);
	assert(glClearColor != 0);
	assert(glCullFace != 0);
	assert(glDepthFunc != 0);
	assert(glDeleteBuffers != 0);
	assert(glDeleteVertexArrays != 0);
	assert(glDisableVertexAttribArray != 0);
	assert(glDrawArrays != 0);
	assert(glEnable != 0);
	assert(glGenVertexArrays != 0);
	assert(glGenBuffers != 0);
	assert(glUseProgram != 0);
	assert(glUniformMatrix4fv != 0);
	assert(glVertexAttribPointer != 0);
	assert(glViewport != 0);
}


VAO::~VAO()
{
	clear();
}

void VAO::clear()
{
	if (indices_buffer) delete indices_buffer;

	for (std::map<Attribute, VBO *>::iterator it = attribute_buffers.begin(); it != attribute_buffers.end(); ++it)
		delete it->second;

	attribute_buffers.clear();
}

void VAO::storeIndices(std::vector<glm::ivec3> indices)
{
	if (indices_buffer)
		delete indices_buffer;

	indices_buffer = new VBO(GL_ELEMENT_ARRAY_BUFFER);

	bind();
	indices_buffer->bind();
	indices_buffer->setData(&indices[0], indices.size() * sizeof(glm::ivec3));
	unbind();

	num_triangles = indices.size();
}

void VAO::storeAttribute(Attribute c, int num_dimensions, GLvoid* data, int size_in_bytes, GLuint shader_location)
{
	if (attribute_buffers.find(c) != attribute_buffers.end())
		delete attribute_buffers[c];
	
	attribute_buffers[c] = new VBO(GL_ARRAY_BUFFER);

	bind();
	attribute_buffers[c]->bind();
	attribute_buffers[c]->setData(data, size_in_bytes);
	glVertexAttribPointer(shader_location, num_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(shader_location);
	unbind();
}


void VAO::storePositions(std::vector<glm::vec3> data, int shader_location)
{
	storeAttribute(POSITION, 3, &data[0], data.size() * sizeof(glm::vec3), shader_location);
}

void VAO::storeNormals(std::vector<glm::vec3> data, int shader_location)
{
	storeAttribute(NORMAL, 3, &data[0], data.size() * sizeof(glm::vec3), shader_location);
}

void VAO::storeTexturecoordinates(std::vector<glm::vec2> data, int shader_location)
{
	storeAttribute(TEXTURECOORDINATE, 2, &data[0], data.size() * sizeof(glm::vec2), shader_location);
}

void VAO::storeTangents(std::vector<glm::vec3> data, int shader_location)
{
	storeAttribute(TANGENT, 3, &data[0], data.size() * sizeof(glm::vec3), shader_location);
}

void VAO::draw()
{
	bind();
	glDrawElements(GL_TRIANGLES, num_triangles * 3, GL_UNSIGNED_INT, 0);
	unbind();
}

void VAO::draw(int start, int end )
{
	bind();
	glDrawElements(GL_TRIANGLES, (end - start) * 3, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * start * 3));
	unbind();
}

void VAO::bind()
{
	glBindVertexArray(id);
}

void VAO::unbind()
{
	glBindVertexArray(0);
}
