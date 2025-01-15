#pragma once

#include <Engine/Core.h>

struct ShaderStorageBuffer {
	ShaderStorageBuffer() = default;

	std::string name;
	GLuint id;
	std::stack<GLuint> inactiveIndexes;
	GLsizei size = 0;
	GLuint binding = 0;

	void Generate(const std::string& name, const GLuint binding) {
		glGenBuffers(1, &id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id);
		this->name = name;
		this->binding = binding;
	}

	void Bind() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	}

	void Allocate(const GLsizei size) {
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	void BufferSubData(const GLsizei size, const void* data) {
		if (!inactiveIndexes.empty()) {
			GLuint inactiveIndex = inactiveIndexes.top();
			inactiveIndexes.pop();
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, inactiveIndex * size, size, data);
		}
		else {
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, this->size, size, data);
			this->size += size;
		}
	}

	void Remove(const GLuint index) {
		inactiveIndexes.push(index); //we set this index to be free to use when new data is to be added
	}

	void BufferData(const GLsizei size, const void* data) {
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
		this->size = size;
	}

	void Unbind() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
};