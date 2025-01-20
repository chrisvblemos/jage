#pragma once

#include <Core/Core.h>

struct ShaderStorageBuffer {
	ShaderStorageBuffer() = default;

	std::string name;
	GLuint id;
	GLuint binding = 0;
	GLenum mode = GL_DYNAMIC_DRAW;

	void Generate(const std::string& name, const GLuint binding, const GLenum mode = GL_DYNAMIC_DRAW) {
		glGenBuffers(1, &id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		this->name = name;
		this->binding = binding;
		this->mode = mode;
	}

	void Bind() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	}

	void Allocate(const GLsizeiptr size) {
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, mode);
	}

	void BufferSubData(const GLuint offset, const GLsizeiptr size, const void* data) {
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
	}

	void BufferData(const GLsizeiptr size, const void* data) {
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, mode);
	}

	void Unbind() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
};