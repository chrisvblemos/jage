#pragma once

#include <Core/Core.h>

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

	void SetAttribPointer(GLuint index, GLuint nElements, GLenum dataType, GLsizei stride, void* offset, const uint32_t divisor = 0) {
		glVertexAttribPointer(index, nElements, GL_FLOAT, GL_FALSE, stride, offset);

		if (divisor > 0) {
			glVertexAttribDivisor(index, divisor);
		}

		glEnableVertexAttribArray(index);

		arrayAttribPointers.push_back(VertexArrayAttribPointer(index, nElements, dataType, stride, offset));
	}
};