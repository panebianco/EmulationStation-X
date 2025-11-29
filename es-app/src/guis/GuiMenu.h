#pragma once

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/TextComponent.h"

class GuiMenu : public GuiComponent
{
public:
    GuiMenu(Window* window);

    bool input(InputConfig* config, Input input) override;
    void onSizeChanged() override;

    HelpStyle getHelpStyle() override;
    std::vector<HelpPrompt> getHelpPrompts() override;

private:
    void openScraperSettings();
    void openSoundSettings();
    void openUISettings();
    void openOtherSettings();
    void openConfigInput();
    void openQuitMenu();
    void openScreensaverOptions();
    void openCollectionSystemSettings();

    // NUEVO: menú para opciones del tema
    void openThemeOptions();

    void addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func);
    void addVersionInfo();

    MenuComponent mMenu;
    TextComponent mVersion;
};
