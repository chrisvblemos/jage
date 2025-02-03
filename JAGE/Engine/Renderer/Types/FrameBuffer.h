#pragma once

#include <Core/Core.h>
#include "IBindable.h"

class FrameBuffer {
public:
	FrameBuffer() = default;

	FrameBuffer(const GLuint width, const GLuint height) {
		glCreateFramebuffers(1, &id);
		this->width = width;
		this->height = height;
	}

	void Clear() {
		if (id != 0) {
			glDeleteFramebuffers(1, &id);
			id = 0;
		}

		colorTextAttachments.clear();
		glDeleteFramebuffers(1, &id);
		rbo = 0;
	}

	void CreateTextDepthAttachment(const GLenum format, const GLenum wrapMethod = GL_CLAMP_TO_BORDER, const glm::vec4& borderColor = glm::vec4(1.0f)) {
		glCreateTextures(GL_TEXTURE_2D, 1, &depthTextAttachment);
		glTextureStorage2D(depthTextAttachment, 1, format, width, height);
		glTextureParameteri(depthTextAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(depthTextAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(depthTextAttachment, GL_TEXTURE_WRAP_S, wrapMethod);
		glTextureParameteri(depthTextAttachment, GL_TEXTURE_WRAP_T, wrapMethod);
		glTextureParameterfv(depthTextAttachment, GL_TEXTURE_BORDER_COLOR, &borderColor[0]);
		glNamedFramebufferTexture(id, GL_DEPTH_ATTACHMENT, depthTextAttachment, 0);
	}

	void Bind() const {
		glBindFramebuffer(GL_FRAMEBUFFER, id);
	}

	void Unbind() const {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CreateRenderBuffer() {
		glCreateRenderbuffers(1, &rbo);
		glNamedRenderbufferStorage(rbo, GL_DEPTH24_STENCIL8, width, height);
		glNamedFramebufferRenderbuffer(id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	}

	void AttachColorTex2D(const GLuint texture, const GLuint attachmentIndex) {
		glNamedFramebufferTexture(id, GL_COLOR_ATTACHMENT0 + attachmentIndex, texture, 0);
		if (attachmentIndex >= colorTextAttachments.size()) {
			colorTextAttachments.resize(attachmentIndex + 1);
		}
		colorTextAttachments[attachmentIndex] = texture;
	}

	void AttachColorTex2Ds(const std::vector<GLuint>& textures) {
		size_t i = 0;
		for (const auto& texture : textures) {
			AttachColorTex2D(texture, static_cast<GLuint>(i));
		}
	}

	void AttachDepthTex2D(const GLuint texture) {
		glNamedFramebufferTexture(id, GL_DEPTH_ATTACHMENT, texture, 0);
		depthTextAttachment = texture;
	}

	void AttachDepthCubeMapTex(const GLuint texture) {
		glNamedFramebufferTexture(id, GL_DEPTH_ATTACHMENT, texture, 0);
		depthTextAttachment = texture;
	}

	void DettachDepthTexture() {
		glNamedFramebufferTexture(id, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);
		depthTextAttachment = 0;
	}

	void DrawBuffer(const GLuint attachmentIndex) {
		GLenum attachment = GL_COLOR_ATTACHMENT0 + attachmentIndex;
		glNamedFramebufferDrawBuffer(id, attachment);
	}

	void DrawColorBuffers() {
		std::vector<GLenum> attachments(colorTextAttachments.size());
		for (uint32_t i = 0; i < colorTextAttachments.size(); ++i) {
			attachments[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLuint>(i);
		}

		glNamedFramebufferDrawBuffers(id, static_cast<GLsizei>(attachments.size()), attachments.data());
	}

	void DisableColorBuffer() {
		glNamedFramebufferDrawBuffer(id, GL_NONE);
		glNamedFramebufferReadBuffer(id, GL_NONE);
	}

	void SetViewport() {
		glViewport(0, 0, width, height);
	}

	bool CheckComplete() {
		return (glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

private:
	GLuint id = 0;
	GLuint width = 0;
	GLuint height = 0;
	GLuint rbo = 0;
	std::vector<GLuint> colorTextAttachments;
	GLuint depthTextAttachment;
};


