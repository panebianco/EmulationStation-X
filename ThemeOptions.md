Theme Options in ES-X 
The core of modern themes

In ES-X, Theme Options are the most important concept in the theme system.

A modern theme is not a fixed skin — it is a configurable system.

theme.ini

Theme Options are defined in an optional file called:

theme.ini


This file:

defines which options the user sees

defines internal theme variables

controls which values are applied based on the selected option

theme.xml contains no logic, it only consumes variables.

Real execution flow

The theme defines options in theme.ini

The user selects an option in Theme Options

ES-X loads the corresponding INI section

Variables are exposed to theme.xml

The theme is redrawn using those variables

Real-world examples
Mini

Simple and direct use of layouts.

✔ One XML
✔ Multiple layouts
✔ No duplication

ArtBook

Theme Options used to select the visual experience
(density, artwork sources, descriptions).

Alekfull

Theme Options used for fine control of the carousel
(scale, opacity, spacing, navigation sounds).

ES-X philosophy

theme.xml → visual structure

theme.ini → configuration

Theme Options → user experience

👉 Think of a theme as a system, not as a static image.

Conclusion

Theme Options are not an extra.
They are the foundation of modern themes in ES-X.
