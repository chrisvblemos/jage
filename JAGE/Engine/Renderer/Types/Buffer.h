#pragma once

#include <Core/Core.h>
#include "IBindable.h"

class IBuffer {
public:
	IBuffer() = default;
	IBuffer(const GLsizeiptr size, const GLenum mode = GL_DYNAMIC_STORAGE_BIT) : size(size), mode(mode) {};

	void UpdateData(const GLintptr offset, const GLsizeiptr dataSize, const void* data) {
		if (offset + dataSize > size) {
			Resize(std::max(size * 2, offset + dataSize));
		}

		glNamedBufferSubData(id, offset, dataSize, data);
	}

	void Resize(const GLsizeiptr newSize) {
		LOG(LogOpenGL, LOG_WARNING, std::format("Resizing buffer {} from {}b to {}b.", id, size, newSize));

		GLuint newBffrId;
		glCreateBuffers(1, &newBffrId);
		glNamedBufferStorage(newBffrId, newSize, nullptr, mode);

		glCopyNamedBufferSubData(id, newBffrId, 0, 0, size);

		glDeleteBuffers(1, &id);
		id = newBffrId;
		size = newSize;
	}

	GLuint GetID() { return id; }
	GLsizeiptr GetSize() { return size; }
	GLenum GetMode() { return mode; }

protected:
	GLuint id = 0;
	GLsizeiptr size = 0;
	GLenum mode = 0;
};

class VertexArrayBuffer : public IBuffer {
public:
	VertexArrayBuffer() = default;
	VertexArrayBuffer(const GLsizeiptr size, const GLenum mode = GL_DYNAMIC_STORAGE_BIT)
		: IBuffer(size, mode) {
		glCreateBuffers(1, &id);
		glNamedBufferStorage(id, size, nullptr, mode);
	}

	void Bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, id);
	}

	void Unbind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
};

class ElementArrayBuffer : public IBuffer {
public:
	ElementArrayBuffer() = default;
	ElementArrayBuffer(const GLsizeiptr size, const GLenum mode = GL_DYNAMIC_STORAGE_BIT)
		: IBuffer(size, mode) {
		glCreateBuffers(1, &id);
		glNamedBufferStorage(id, size, nullptr, mode);

		if (id == 0)
			LOG(LogOpenGL, LOG_CRITICAL, "Failed to create element array buffer.");
	}

	void Bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	}

	void Unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
};

class UniformBuffer : public IBuffer {
public:
	UniformBuffer() = default;
	UniformBuffer(const GLuint bindingPoint, const GLsizeiptr size, const GLenum mode = GL_DYNAMIC_STORAGE_BIT)
		:	IBuffer(size, mode), bindingPoint(bindingPoint) {
		glCreateBuffers(1, &id);
		glNamedBufferStorage(id, size, nullptr, mode);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, id);
	}

	GLuint GetBindingPoint() const { return bindingPoint;  }

private:
	GLuint bindingPoint = 0;
};

class ShaderStorageBuffer : public IBuffer {
public:
	ShaderStorageBuffer() = default;
	ShaderStorageBuffer(const GLuint bindingPoint, const GLsizeiptr size, const GLenum mode = GL_DYNAMIC_STORAGE_BIT)
		: IBuffer(size, mode), bindingPoint(bindingPoint) {
		glCreateBuffers(1, &id);
		glNamedBufferStorage(id, size, nullptr, mode);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, id);
	}

	GLuint GetBindingPoint() const { return bindingPoint; }

private:
	GLuint bindingPoint = 0;
};

class DrawIndirectBuffer : public IBuffer {
public:
	DrawIndirectBuffer() = default;
	DrawIndirectBuffer(const GLuint bindingPoint, const GLsizeiptr size, const GLenum mode = GL_DYNAMIC_STORAGE_BIT)
		: IBuffer(size, mode), bindingPoint(bindingPoint) {
		glCreateBuffers(1, &id);
		glNamedBufferStorage(id, size, nullptr, mode);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, id);
	}

	void Bind() const {
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, id);
	}

	void Unbind() const {
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	}

	GLuint GetBindingPoint() const { return bindingPoint; }

private:
	GLuint bindingPoint = 0;
};