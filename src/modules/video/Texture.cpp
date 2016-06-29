/**
 * @file
 */

#include "Texture.h"
#include "core/Log.h"
#include "core/String.h"
#include "video/GLFunc.h"

namespace video {

Texture::Texture(const std::string& name) :
		io::IOResource(), _name(name) {
	glGenTextures(1, &_handle);
	static const int empty = 0x00000000;
	upload((const uint8_t*)&empty, 1, 1, 4);
	unbind();
	GL_checkError();
}

Texture::Texture(const std::string& name, const uint8_t* data, int width, int height, int depth) :
		io::IOResource(), _name(name) {
	glGenTextures(1, &_handle);
	upload(data, width, height, depth);
	unbind();
	GL_checkError();
}

Texture::~Texture() {
	// in case of a texture we don't want this check, as it might be shared between multiple resources
	// and it should only be destroyed once it's completely destroyed by the shared_ptr
	//core_assert_msg(_handle == 0u, "Texture %s was not properly shut down", _name.c_str());
	shutdown();
}

void Texture::shutdown() {
	if (_handle != 0) {
		glDeleteTextures(1, &_handle);
		_handle = 0;
	}
}

void Texture::upload(const uint8_t* data, int width, int height, int depth) {
	const GLenum mode = depth == 4 ? GL_RGBA : GL_RGB;
	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode, GL_UNSIGNED_BYTE, (const void*)data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	_state = io::IOSTATE_LOADED;
	GL_checkError();
}

void Texture::bind(int unit) {
	if (unit != 0) {
		glActiveTexture(GL_TEXTURE0 + unit);
	}
	glBindTexture(GL_TEXTURE_2D, _handle);
	GL_checkError();
	_boundUnit = unit;
	if (unit != 0) {
		glActiveTexture(GL_TEXTURE0);
	}
}

void Texture::unbind() {
	if (_boundUnit != 0) {
		glActiveTexture(GL_TEXTURE0 + _boundUnit);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	GL_checkError();
	if (_boundUnit != 0) {
		glActiveTexture(GL_TEXTURE0);
	}
	_boundUnit = 0;
}

}
