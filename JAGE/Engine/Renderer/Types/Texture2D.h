#pragma once

#include <Core/Core.h>
#include "IBindable.h"

template <typename T>
struct TexParameter {
	GLenum name;
	T value;
};

class TextureBase {
public:
	TextureBase() = default;
	TextureBase(
		const std::string name,
		const GLenum type,
		const GLenum format,
		const GLenum internalFormat,
		const GLuint width,
		const GLuint height)
		:	name(name),
			type(type),
			format(format),
			internalFormat(internalFormat),
			width(width),
			height(height) {};
	
	void BindToUnit(const GLuint unit) {
		glBindTextureUnit(unit, id);
		this->unit = unit;
	}

	void UnbindFromUnit(const GLuint unit) {
		glBindTextureUnit(unit, 0);
		this->unit = -1;
	}

	void Bind() const {
		glBindTexture(type, id);
	}

	void Unbind() const {
		glBindTexture(type, 0);
	}

	template <typename T>
	void SetParam(const GLenum pname, const T& pvalue) {
		if (handle != 0) {
			LOG(LogOpenGL, LOG_WARNING, std::format("Attempting to set param in immutable texture {}.", name));
			return;
		}

		if constexpr (std::is_value_v<T, GLint>) {
			glTextureParameteri(id, pname, pvalue);
		}
		else if constexpr (std::is_value_v<T, GLfloat>) {
			glTextureParameterf(id, pname, pvalue);
		}
		else if constexpr (std::is_value_v<T, std::vector<GLfloat>>) {
			glTextureParameterfv(id, pname, pvalue.data());
		}
		else {
			static_assert(!std::is_same_v<T, T>, "Unsupported texture parameter type.");
		}
	}

	void MakeResident() const {
		if (!isResident && handle != 0)
			glMakeTextureHandleResidentARB(handle);
	}

	void MakeNonResident() const {
		if (isResident && handle != 0)
			glMakeTextureHandleNonResidentARB(handle);
	}

	void Clear() {
		if (id != 0) {
			glDeleteTextures(1, &id);
			id = -1;
		}
	}

	void GenerateMipMap() const {
		if (handle != 0) {
			LOG(LogOpenGL, LOG_WARNING, std::format("Attempting to set param in immutable texture {}.", name));
			return;
		}

		glGenerateTextureMipmap(id);
	}

	uint32_t GetHadleIndex() const { return handleIdx; }
	std::string GetName() const { return name; }

protected:
	std::string name;
	GLuint id = 0;
	GLenum type = 0;
	GLenum format = 0;
	GLenum internalFormat = 0;
	GLuint width = 0;
	GLuint height = 0;
	bool isBound = false;
	bool isResident = false;
	GLuint64 handle = 0;
	int32_t unit = -1;
	int32_t handleIdx = -1;
};

class TextureCubeMapArray : public TextureBase {
public:
	TextureCubeMapArray() = default;
	TextureCubeMapArray(
		const std::string name,
		const GLenum type,
		const GLenum format,
		const GLenum internalFormat,
		const GLuint width,
		const GLuint height,
		const GLuint nCubemaps)
		: TextureBase(
			name,
			type,
			format,
			internalFormat,
			width,
			height), nCubemaps(nCubemaps) {
	
		if (nCubemaps % 6 == 0) {
			LOG(LogOpenGL, LOG_CRITICAL, "Failed to generate texture cubemap array. nCubemaps isn't a multiple of 6.");
			return;
		}

		glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &id);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureStorage3D(id, 1, format, width, height, nCubemaps);
	};

	void SetSubImage(const GLint level, const GLsizei width, const GLsizei height, const GLsizei depth, const GLenum format, const GLenum type, const void* data) {
		if (handle != 0) {
			LOG(LogOpenGL, LOG_WARNING, std::format("Attempting to set param in immutable texture {}.", name));
			return;
		}

		glTextureSubImage3D(id, level, 0, 0, 0, width, height, depth, format, type, data);
	}

private:
	GLuint nCubemaps = 0;
};

class Texture2D : public TextureBase {
public:
	Texture2D() = default;

	Texture2D(
		const std::string name,
		const GLenum type,
		const GLenum format,
		const GLenum internalFormat,
		const GLuint width,
		const GLuint height,
		const GLuint nCubemaps,
		const void* data)
		: TextureBase(
			name,
			type,
			format,
			internalFormat,
			width,
			height) {

		glCreateTextures(GL_TEXTURE_2D, 1, &id);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTextureStorage2D(id, 1, internalFormat, width, height);
		
		if (data == nullptr)
		{
			LOG(LogOpenGL, LOG_CRITICAL, std::format("Texture {} has invalid data.", name));
			return;
		}
	}

	void SetSubImage2D(const GLuint width, const GLuint height, const GLenum format, const GLenum type, const void* data) {
		if (handle != 0) {
			LOG(LogOpenGL, LOG_WARNING, std::format("Attempting to set param in immutable texture {}.", name));
			return;
		}

		glTextureSubImage2D(id, 0, 0, 0, width, height, format, type, data);
	}
	
};