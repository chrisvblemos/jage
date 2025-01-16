#pragma once

#include <Core/Core.h>

struct ShaderStorageBuffer {
	ShaderStorageBuffer() = default;

	std::string name;
	GLuint id;
	GLuint binding = 0;

	void Generate(const std::string& name, const GLuint binding) {
		glGenBuffers(1, &id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		this->name = name;
		this->binding = binding;
	}

	void Bind() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	}

	void Allocate(const GLsizei size) {
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	void BufferSubData(const GLuint offset, const GLsizei size, const void* data) {
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
	}

	void BufferData(const GLsizei size, const void* data) {
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}

	void Unbind() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
};