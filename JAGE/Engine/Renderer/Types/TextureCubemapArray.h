#pragma once

#include <Core/Core.h>

struct TextureCubeMapArray {
	std::string name;
	GLuint id = 0;
	const void* data = nullptr;
	GLenum format = 0;
	GLsizei width = 0;
	GLsizei height = 0;
	GLuint nCubemaps = 0;

	TextureCubeMapArray() = default;

	void Generate(GLenum format, GLuint width, GLuint height, GLuint nCubemaps) {
		assert(nCubemaps % 6 == 0 && "TextureCubeMapArray: Failed to generate. N of cube maps must be a multiple of 6.");

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, id);

		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, format, width, height, nCubemaps);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

		this->format = format;
		this->width = width;
		this->height = height;
		this->nCubemaps = nCubemaps;
	}

	void SetLevelParameters(const GLuint index) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BASE_LEVEL, index * 6);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAX_LEVEL, index * 6 + 5);
	}

	void Allocate(GLenum format, GLuint width, GLuint height, GLuint nElements) {
		glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, format, width, height, nElements);
	}

	void Bind() {
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, id);
	}

	void ActiveAndBind(const GLuint index) {
		glActiveTexture(GL_TEXTURE0 + index);
		Bind();
	}

	void Unbind() {
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
	}

};