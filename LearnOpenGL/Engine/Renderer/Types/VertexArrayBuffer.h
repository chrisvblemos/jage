#pragma once

#include <Engine/Core.h>

struct VertexArrayBuffer {
	VertexArrayBuffer() = default;

	std::string name;
	GLuint id;

	struct VertexArrayAttribPointer {
		GLuint index;
		GLuint nElements;
		GLenum dataType;
		GLsizei stride;
		void* offset;
	};

	std::vector<VertexArrayAttribPointer> arrayAttribPointers;

	void Generate(const std::string& name) {
		glGenVertexArrays(1, &id);
		this->name = name;
	}

	void Bind() {
		glBindVertexArray(id);
	}

	void Unbind() {
		glBindVertexArray(0);
	}

	void SetAttribPointer(GLuint index, GLuint nElements, GLenum dataType, GLsizei stride, void* offset) {
		glVertexAttribPointer(index, nElements, GL_FLOAT, GL_FALSE, stride, offset);
		glEnableVertexAttribArray(index);

		arrayAttribPointers.push_back(VertexArrayAttribPointer(index, nElements, dataType, stride, offset));
	}
};