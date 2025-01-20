#pragma once

#include <Core/Core.h>

struct VertexBuffer {
	VertexBuffer() = default;

	std::string name;
	GLuint id;
	const void* data = nullptr;
	GLsizeiptr size = 0;

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

	void Allocate(GLsizeiptr size) {
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
		this->size = size;
	}

	void BufferSubData(const GLuint offset, const GLsizeiptr size, const void* data) {
		if (offset + size > this->size) {
			GLsizeiptr newSize = std::max(this->size * 2, offset + size);
			LOG(LogGeneric, LOG_WARNING, std::format("Vertex buffer {} out of memory. Resizing from {} to {} bytes", id, this->size, newSize));

			glBindBuffer(GL_ARRAY_BUFFER, 0);										// prepare for copying

			GLuint targetBffr;
			glGenBuffers(1, &targetBffr);

			glBindBuffer(GL_COPY_WRITE_BUFFER, targetBffr);
			glBufferData(GL_COPY_WRITE_BUFFER, newSize, nullptr, GL_STATIC_DRAW);

			glBindBuffer(GL_COPY_READ_BUFFER, this->id);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, this->size);

			glBindBuffer(GL_COPY_READ_BUFFER, 0);
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			glDeleteBuffers(1, &this->id);

			this->id = targetBffr;
			this->size = newSize;

			glBindBuffer(GL_ARRAY_BUFFER, this->id);										// return things to normal
		}
		
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	}

	void BufferData(GLsizeiptr size, const void* data) {
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
		this->size = size;
	}


};