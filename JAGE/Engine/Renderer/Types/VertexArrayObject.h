#pragma once

#include <Core/Core.h>
#include <Core/Types/Mesh.h>

struct VertexAttribData {
	GLuint index;
	GLsizeiptr size;
	GLenum type;
	GLboolean normalized;
	GLuint offset;
};

class VertexArrayObject {
	GLuint id = 0;

	VertexArrayObject() {
		glCreateVertexArrays(1, &id);
	}

	~VertexArrayObject() {
		glDeleteVertexArrays(1, &id);
	}

	void Configure(const GLuint vbo, const GLuint ebo, const std::vector<VertexAttribData>& attribs) {
		glVertexArrayVertexBuffer(id, 0, vbo, 0, sizeof(Vertex));
		glVertexArrayElementBuffer(id, ebo);

		for (const auto& attrib : attribs) {
			glEnableVertexArrayAttrib(id, attrib.index);
			glVertexArrayAttribFormat(id, attrib.index, attrib.size, attrib.type, attrib.normalized, attrib.offset);
			glVertexArrayAttribBinding(id, attrib.index, 0);
		}
	}

	void Bind() const {
		glBindVertexArray(id);
	}

	void Unbind() const {
		glBindVertexArray(0);
	}
};