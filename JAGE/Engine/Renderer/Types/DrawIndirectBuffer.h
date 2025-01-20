#pragma once

#include <Core/Core.h>

// 1. we build a VAO for the meshes (it's fixed once a mesh is loaded)
// aPosition: {baseIndex = 0, stride = 3}, aNormal: {baseIndex = 1, stride = 3}, aTexCoord: {baseIndex = 2, stride = 3}, 

// 2. we build a meshes data VBO in the following way:
// | vertices[0] | normals[0] | texCoords[0] | vertices[1] | normals[1] | texCoords[1] ... | texCoords[n] |

// 3. we build a VBO for instances data (position, etc) N >>> n:
// instance data of the same meshes need to be packed
// | model[0] | inverseModel[0] | model[1] | inverseModel[1] | ... | inverseModel[N] |



struct DrawIndirectBuffer{
	std::string name;
	GLuint id = 0;
	bool asSSBO = false;

	void Generate(const std::string& name,  const GLuint binding) {
		glGenBuffers(1, &id);
		Bind();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id);
		Unbind();

		this->name = name;
	}

	void Allocate(const GLsizeiptr size) {
		glBufferData(GL_DRAW_INDIRECT_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	void BufferData(const GLsizeiptr size, const void* data) {
		glBufferData(GL_DRAW_INDIRECT_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}

	void Bind() const {
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	}

	void BufferSubData(const GLuint offset, const GLsizeiptr size, const void* data) {
		glBufferSubData(GL_DRAW_INDIRECT_BUFFER, offset, size, data);
	}

	void Unbind() const {
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void Draw(const GLsizeiptr size, const GLsizeiptr stride) const {
		glMultiDrawElementsIndirect(
			GL_TRIANGLES,
			GL_UNSIGNED_INT,
			(const void*)0,
			size,
			stride
		);
	}
};