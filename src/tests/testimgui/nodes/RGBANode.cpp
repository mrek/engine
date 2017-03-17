#include "RGBANode.h"
#include "image/Image.h"
#include "core/GLM.h"
#include "core/Log.h"
#include "NNode.h"

namespace ImGui {

RGBANode::~RGBANode() {
	video::deleteTexture(_texture);
}

const char* RGBANode::getTooltip() const {
	return "Save noise as png.";
}

const char* RGBANode::getInfo() const {
	return "RGBANode info.\n\nSave the noise input data as png.";
}

void RGBANode::onEdited() {
	if (imageName[0] == '\0') {
		Log::info("No imagename set");
		return;
	}
	NNode* red   = dynamic_cast<NNode*>(nge->getInputNodeForNodeAndSlot(this, 0));
	NNode* green = dynamic_cast<NNode*>(nge->getInputNodeForNodeAndSlot(this, 1));
	NNode* blue  = dynamic_cast<NNode*>(nge->getInputNodeForNodeAndSlot(this, 2));
	NNode* alpha = dynamic_cast<NNode*>(nge->getInputNodeForNodeAndSlot(this, 3));

	if (red == nullptr && green == nullptr && blue == nullptr && alpha == nullptr) {
		Log::info("No input node set");
		return;
	}

	// TODO: thread me

	constexpr int components = 4;
	uint8_t buffer[imageWidth * imageHeight * components];

	for (int x = 0; x < imageWidth; ++x) {
		for (int y = 0; y < imageHeight; ++y) {
			const float r = red   != nullptr ? red->getNoise(x, y)   : 0.0f;
			const float g = green != nullptr ? green->getNoise(x, y) : 0.0f;
			const float b = blue  != nullptr ? blue->getNoise(x, y)  : 0.0f;
			const float a = alpha != nullptr ? alpha->getNoise(x, y) : 1.0f;
			const uint8_t br = glm::clamp(r, 0.0f, 1.0f) * 255.0f;
			const uint8_t bg = glm::clamp(g, 0.0f, 1.0f) * 255.0f;
			const uint8_t bb = glm::clamp(b, 0.0f, 1.0f) * 255.0f;
			const uint8_t ba = glm::clamp(a, 0.0f, 1.0f) * 255.0f;
			const int index = y * (imageWidth * components) + (x * components);
			buffer[index + 0] = br;
			buffer[index + 1] = bg;
			buffer[index + 2] = bb;
			buffer[index + 3] = ba;
		}
	}
	if (!image::Image::writePng(imageName, buffer, imageWidth, imageHeight, components)) {
		Log::error("Failed to write image %s", imageName);
		return;
	}
	Log::info("Wrote image %s", imageName);

	if (_texture == video::InvalidId) {
		_texture = video::genTexture();
		video::bindTexture(video::TextureUnit::Upload, video::TextureType::Texture2D, _texture);
		video::setupTexture(video::TextureType::Texture2D, video::TextureWrap::None);
	} else {
		video::bindTexture(video::TextureUnit::Upload, video::TextureType::Texture2D, _texture);
	}
	video::uploadTexture(video::TextureType::Texture2D, video::TextureFormat::RGBA, imageWidth, imageHeight, buffer, 0);
}

void RGBANode::getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut, ImU32& defaultTitleBgColorOut, float& defaultTitleBgColorGradientOut) const {
	defaultTitleTextColorOut = IM_COL32(230, 180, 180, 255);
	defaultTitleBgColorOut = IM_COL32(40, 55, 55, 200);
	defaultTitleBgColorGradientOut = 0.025f;
}

bool RGBANode::render(float nodeWidth) {
	const bool retVal = Node::render(nodeWidth);
	if (_texture != video::InvalidId) {
		ImGui::Image((ImTextureID) (intptr_t) _texture, ImVec2(200, 100));
	}
	return retVal;
}

RGBANode* RGBANode::Create(const ImVec2& pos, ImGui::NodeGraphEditor& nge) {
	RGBANode* node = imguiAlloc<RGBANode>();
	node->init("RGBANode", pos, "r;g;b;a", "", int(NodeType::RGBA));
	node->fields.addFieldTextEdit(node->imageName, IM_ARRAYSIZE(node->imageName), "Image", "Image filename", ImGuiInputTextFlags_EnterReturnsTrue);
	node->fields.addField(&node->imageWidth, 1, "Width", "Image width", 0, 100, 4096);
	node->fields.addField(&node->imageHeight, 1, "Height", "Image height", 0, 100, 4096);
	node->nge = &nge;

	return node;
}

}
