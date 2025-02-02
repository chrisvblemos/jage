#pragma once

#include <Core/Core.h>
#include "IBindable.h"

struct VertexAttrib {
	VertexAttrib(const GLuint index, const GLint size, const GLenum type, const GLboolean normalized, GLuint offset)
		: index(index), size(size), type(type), normalized(normalized), offset(offset) {
	};

	GLuint index;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLuint offset;
};

class VertexArray {
public:
	VertexArray() {
		glCreateVertexArrays(1, &id);
	}
	
	void Configure(const GLuint vbo, const GLsizei stride, const GLuint ebo, const std::vector<VertexAttrib>& attribs) {
		glVertexArrayVertexBuffer(id, 0, vbo, 0, stride);
		glVertexArrayElementBuffer(id, ebo);

		for (const auto& attrib : attribs) {
			glEnableVertexArrayAttrib(id, attrib.index);
			glVertexArrayAttribFormat(id, attrib.index, attrib.size, attrib.type, attrib.normalized, attrib.offset);
			glVertexArrayAttribBinding(id, attrib.index, 0);
		}

		this->attribs = attribs;
		this->attachedEBO = ebo;
		this->attachedVBO = vbo;
	}

	void Bind() const {
		glBindVertexArray(id);
	}

	void Unbind() const {
		glBindVertexArray(0);
	}

private:
	GLuint id;
	GLuint attachedVBO = 0;
	GLuint attachedEBO = 0;
	std::vector<VertexAttrib> attribs;
};