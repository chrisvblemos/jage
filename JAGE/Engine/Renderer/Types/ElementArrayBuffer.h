#pragma once

#include <Core/Core.h>

struct ElementArrayBuffer {
	GLuint id;
	std::string name;

	void Generate(const std::string& name) {
		glGenBuffers(1, &id);
		this->name = name;
	}

	void Bind() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	}

	void Unbind() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void Allocate(GLsizei size) {
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			size,
			nullptr,
			GL_STATIC_DRAW
		);
	}

	void BufferSubData(const GLuint offset, const GLsizei size, const void* data) {
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
	}

	void BufferData(const GLsizei size, const void* data) {
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			size,
			data,
			GL_STATIC_DRAW
		);
	}
};