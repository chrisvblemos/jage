#pragma once

#include <Core/Core.h>

struct VertexBuffer {
	VertexBuffer() = default;

	std::string name;
	GLuint id;
	const void* data = nullptr;

	void Generate(const std::string& name) {
		glGenBuffers(1, &id);
		this->name = name;
	}

	void Bind() {
		glBindBuffer(GL_ARRAY_BUFFER, id);
	}

	void Unbind() {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Allocate(GLsizei size) {
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
	}

	void BufferSubData(const GLuint offset, const GLsizei size, const void* data) {
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	}

	void BufferData(GLsizei size, const void* data) {
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	}


};