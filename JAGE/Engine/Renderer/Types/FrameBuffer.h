#pragma once

#include <Core/Core.h>

struct FrameBuffer {
	FrameBuffer() = default;

	std::string name;
	GLuint id;
	GLuint width = 0;
	GLuint height = 0;
	GLuint rbo = 0;
	std::vector<GLuint> colorTextAttachments;
	GLuint depthTextAttachment;

	void Generate(const std::string name, uint32_t width, uint32_t height) {
		glGenFramebuffers(1, &id);

		this->name = name;
		this->width = width;
		this->height = height;
	}

	void Clear() {
		colorTextAttachments.clear();
		glDeleteFramebuffers(1, &id);
	}

	void CreateTextDepthAttachment(GLenum format) {
		glGenTextures(1, &depthTextAttachment);
		glBindTexture(GL_TEXTURE_2D, depthTextAttachment);
		glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextAttachment, 0);
	}

	void Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, id);
	}

	void Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ActiveAndBindDepthAttachment(const GLuint i) {
		if (depthTextAttachment == 0) {
			LOG(LogGeneric, LOG_CRITICAL, std::format("Failed to active and bind texture depth attachment for framebuffer."));
		}

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, depthTextAttachment);
	}

	void CreateRenderBuffer() {
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void AttachColorTexture2D(const std::string& name, const GLuint texture, const GLuint i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture, 0);
		colorTextAttachments.push_back(texture);
	}

	void AttachDepthTexture2D(const std::string& name, const GLuint texture) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
		this->depthTextAttachment = texture;
	}

	void AttachCubeMapTexture(const std::string& name, const GLuint cubemapTexture) {
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubemapTexture, 0);
		this->depthTextAttachment = cubemapTexture;
	}

	void DettachDepthTexture() {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	}

	void DrawBuffer(const GLuint i) {
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
	}

	void DrawColorBuffers() {
		std::vector<GLenum> colorAttachmentEnums;
		for (int i = 0; i < colorTextAttachments.size(); i++) {
			colorAttachmentEnums.push_back(GL_COLOR_ATTACHMENT0 + i);
		}

		glDrawBuffers(static_cast<GLsizei>(colorTextAttachments.size()), colorAttachmentEnums.data());
	}

	void DisableColorBuffer() {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	void SetViewport() {
		glViewport(0, 0, width, height);
	}

	bool CheckComplete() {
		return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}
};


