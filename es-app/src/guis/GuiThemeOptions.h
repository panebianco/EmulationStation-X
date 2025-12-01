#pragma once

#include "GuiComponent.h"
#include "components/MenuComponent.h"

// Ventana de opciones específicas del tema (Theme Options)
// Lee theme.ini del tema activo y muestra las opciones definidas allí.
class GuiThemeOptions : public GuiComponent
{
public:
    explicit GuiThemeOptions(Window* window);

    bool input(InputConfig* config, Input input) override;
    HelpStyle getHelpStyle() override;
    std::vector<HelpPrompt> getHelpPrompts() override;

private:
    MenuComponent mMenu;
};
