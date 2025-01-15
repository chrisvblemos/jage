#pragma once

#include <Engine/Core.h>

struct VertexBuffer {
	VertexBuffer() = default;

	std::string name;
	GLuint id;
	const void* data = nullptr;
	GLsizei size = 0;
	GLenum drawMode = 0;

	std::stack<GLuint> freeIndexes;

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

	void Allocate(GLsizei size, GLenum drawMode) {
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, drawMode);
		this->drawMode = drawMode;
	}

	void BufferSubData(const GLsizei size, const void* data) {
		if (!freeIndexes.empty()) {
			GLuint freeIndex = freeIndexes.top();
			freeIndexes.pop();
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, freeIndex * size, size, data);
		}
		else {
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, this->size, size, data);
			this->size += size;
		}
	}

	void BufferData(GLsizei size, const void* data, GLenum drawMode) {
		glBufferData(GL_ARRAY_BUFFER, size, data, drawMode);

		this->size = size;
		this->data = data;
		this->drawMode = drawMode;
	}


};