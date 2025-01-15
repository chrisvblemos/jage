#pragma once

#include <Engine/Core.h>

struct ElementArrayBuffer {
	GLuint id;
	std::string name;
	std::vector<GLuint> indices;
	GLsizei size = 0;

	void Generate(const std::string& name) {
		this->indices = indices;
		this->name = name;
		glGenBuffers(1, &id);
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

		this->size = 0;
	}

	void BufferData(const std::vector<GLuint>& indices) {
		GLsizei size = static_cast<GLsizei>(indices.size() * sizeof(GLuint));
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			size,
			indices.data(),
			GL_STATIC_DRAW
		);

		this->size = size;
		this->indices = indices;
	}
};