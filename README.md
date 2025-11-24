# EmulationStation-X (ES-X)

EmulationStation-X (ES-X) is a personal and experimental fork of EmulationStation.  
Its goal is to explore new concepts for localization, theme flexibility, and visual customization, while remaining compatible with RetroPie.

ES-X focuses on:
- Runtime language localization using editable .ini files (no recompilation)
- Theme behavior improvements (spacing, opacity, scale, alignment)
- Proposed support for center logo positioning and zIndex control
- Experimental work on menu sound hooks (not yet functional)

This project does not replace the original EmulationStation.  
It is a sandbox for testing ideas that may help theme creators and RetroPie users.

---

## Current Status

| Feature | State |
|--------|-------|
| .ini language system | Implemented |
| Menu and popup translations | Implemented |
| Scraper, GameList and Options translations | In progress |
| Supported theme parameters | logoScale, scaledLogoSpacing, minLogoOpacity |
| Proposed engine tags | centerLogoOffsetY, zIndexOffset (not active yet) |
| Menu sound system | Experimental, not functional |
| Avatar and username | Not part of ES-X (belongs to themes only) |

---

## Language System

Language files go in:
