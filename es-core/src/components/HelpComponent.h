#pragma once
#ifndef ES_CORE_COMPONENTS_HELP_COMPONENT_H
#define ES_CORE_COMPONENTS_HELP_COMPONENT_H

#include "GuiComponent.h"
#include "HelpPrompt.h"
#include "HelpStyle.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class ComponentGrid;
class TextureResource;

class HelpComponent : public GuiComponent
{
public:
	HelpComponent(Window* window);

	void clearPrompts();
	void setPrompts(const std::vector<HelpPrompt>& prompts);

	void render(const Transform4x4f& parent) override;
	void setOpacity(unsigned char opacity) override;

	void setStyle(const HelpStyle& style);

private:
	void updateGrid();

	std::shared_ptr<TextureResource> getIconTexture(const char* logicalName);

	static std::string normalizeKey(const std::string& key);
	static std::string getSettingSafe(const std::string& key, const std::string& def);

	static std::vector<std::string> getSearchBases();
	static std::vector<std::string> buildCandidateRelativeFiles(const std::string& logicalName);

	static std::string joinPath(const std::string& base, const std::string& rel);
	static bool existsAny(const std::string& path);

private:
	std::map<std::string, std::shared_ptr<TextureResource>> mIconCache;

	std::shared_ptr<ComponentGrid> mGrid;

	std::vector<HelpPrompt> mPrompts;
	HelpStyle mStyle;
};

#endif // ES_CORE_COMPONENTS_HELP_COMPONENT_H
