#pragma once

#include <Core/Core.h>

class Buffer {
	GLuint id = 0;
	GLenum type;
	GLenum mode;
	GLsizeiptr size = 0;

	Buffer(const GLenum type, const GLsizeiptr size, const GLenum mode = GL_DYNAMIC_STORAGE_BIT) : type(type), size(size), mode(mode) {
		glCreateBuffers(1, &id);
		glNamedBufferStorage(id, size, nullptr, mode);
	};

	~Buffer() {
		glDeleteBuffers(1, &id);
	}

	void UpdateData(const GLintptr offset, const GLsizeiptr dataSize, const void* data) {
		if (offset + dataSize > size) {
			Resize(std::max(size * 2, offset + dataSize));
		}

		glNamedBufferSubData(id, offset, dataSize, data);
	}

	void Resize(const GLsizeiptr newSize) {
		GLuint newBffrId;
		glCreateBuffers(1, &newBffrId);
		glNamedBufferStorage(newBffrId, newSize, nullptr, mode);

		glCopyNamedBufferSubData(id, newBffrId, 0, 0, size);

		glDeleteBuffers(1, &id);
		id = newBffrId;
		size = newSize;
	}

	void Bind() const {
		glBindBuffer(type, id);
	}

	void Unbind() const {
		glBindBuffer(type, 0);
	}
};