#pragma once

#include <Core/Core.h>

struct UniformBuffer {
	UniformBuffer() = default;

	std::string name;
	GLuint id;
	GLuint binding = 0;

	void Generate(const std::string& name, GLuint binding, GLsizeiptr size) {
		glGenBuffers(1, &id);
		glBindBuffer(GL_UNIFORM_BUFFER, id);
		glBindBufferRange(GL_UNIFORM_BUFFER, binding, id, 0, size);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		this->name = name;
		this->binding = binding;
	}

	void Bind() {
		glBindBuffer(GL_UNIFORM_BUFFER, id);
	}

	void Allocate(const GLsizeiptr size) {
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	void BufferSubData(const GLsizeiptr offset, const GLsizeiptr size, const void* data) {
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	}

	void BufferData(size_t size, const void* data) {
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}

	void Unbind() {
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
};