#pragma once

#include <Engine/Core.h>

struct UniformBuffer {
	UniformBuffer() = default;

	std::string name;
	GLuint id;
	GLuint binding = 0;
	GLsizei size = 0;
	const void* data = nullptr;

	void Generate(const std::string& name, GLuint binding, GLsizei size) {
		glGenBuffers(1, &id);
		glBindBufferRange(GL_UNIFORM_BUFFER, binding, id, 0, size);
		this->name = name;
		this->binding = binding;
		this->size = size;
	}

	void Bind() {
		glBindBuffer(GL_UNIFORM_BUFFER, id);
	}

	void Allocate(const GLsizei size) {
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		this->size = 0;
	}

	void BufferSubData(const GLsizei offset, const GLsizei size, const void* data) {
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	}

	void BufferData(size_t size, const void* data) {
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
		this->data = data;
	}

	void Unbind() {
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
};