# EmulationStation-X (ES-X)
<img width="1280" height="720" alt="LOGO" src="https://github.com/user-attachments/assets/e8560813-41dd-4ad8-bf01-ecc6e03e922c" />

ES-X is a personal fork of EmulationStation focused on extending the theming engine, accessibility, and UI flexibility, while remaining compatible with RetroPie and inspired by modern frontends like ES-DE and Batocera.

ES-X is experimental as a project, not unstable as a platform. Core features are usable today, while new capabilities continue to evolve.

## 🎯 Purpose

ES-X does not aim to replace EmulationStation. It exists as a sandbox for innovation:

- Improving theme expressiveness
- Adding runtime localization
- Giving creators more control without breaking compatibility
- Exploring modern UI/audio ideas in a controlled way

## ✨ Highlights

- 🌍 Runtime language system (.ini, no recompilation)
- 🎨 Advanced theme variables (logo spacing, scale, opacity)
- 🖌️ Text borders & shadows (engine-level)
- 🎶 Background music & navigation sounds (menu-controlled)
- 🌙 Dark menu support
- 🔊 Sound tags compatible with ES-DE / Batocera (partial)
- 🧩 Theme options driven by variables
- 🛠️ Designed for theme creators first

## 🖼 Preview

| Theme Options | Language Demo |
|-----------|---------------|
| <img width="450" height="768" alt="Captura de pantalla_2025-12-01_12-11-25" src="https://github.com/user-attachments/assets/7de22896-7b6b-43fb-910e-700d56ddb11d" />
| <img width="450" src="https://github.com/user-attachments/assets/6e37ffc7-9c35-450e-b274-72d31b12a4f5" /> |<img width="1366" height="768" alt="Captura de pantalla_2025-12-01_12-24-53" src="https://github.com/user-attachments/assets/f4912b70-b738-46d7-8767-1d36995bdb6c" /><img width="1366" height="768" alt="Captura de pantalla_2025-12-05_07-08-05" src="https://github.com/user-attachments/assets/4d9add99-955b-4b05-9b1f-37597fda2f1b" />



## 🔑 Current Feature Status

| Feature | Status |
|---------|--------|
| .ini language system (runtime) | ✅ Stable |
| Translation in menus, popups, scraper | ✅ Stable |
| Theme variables (logoScale, spacing, opacity) | ✅ Available |
| Text borders & shadow support | ✅ Available |
| Variable-driven theme options | ✅ Available |
| Dark menu mode | ✅ Available |
| Background music (SDL_mixer) | 🟡 Available / evolving |
| Menu sounds (scroll, select, back) | 🟡 Available / evolving |
| ES-DE / Batocera sound tags | 🟡 Partially supported |
| Advanced audio mixing / ducking | 🔄 In progress |
| User profiles / avatars | ❌ Not part of ES-X |

## 🌍 Language System (Runtime)

Languages are stored in:

```
~/.emulationstation/lang/
```

Example .ini:

```ini
[MAIN]
MAIN MENU=MENÚ PRINCIPAL
YES=SÍ
BACK=VOLVER
CANCEL=CANCELAR
```

✔ No recompilation  
✔ Editable by users  
✔ Loaded dynamically at runtime

## 🎨 Theme Engine Extensions

**Supported / Available**

```
<logoScale>1.1</logoScale>
<scaledLogoSpacing>0.20</scaledLogoSpacing>
<minLogoOpacity>0.25</minLogoOpacity>

<textBorderColor>#000000</textBorderColor>
<textShadowColor>#000000</textShadowColor>
<textShadowOffset>2 2</textShadowOffset>
```

These extensions are engine-level, not hacks, and are designed to remain compatible with existing themes.

**Planned / Proposed**

```
<centerLogoOffsetY>
<centerLogoPos>
<zIndexOffset>
```

## 🔊 Audio & Music System

- SDL_mixer-based background music
- Music control directly from the menu
- Navigation sounds (scroll / select / back)
- Sound tag structure inspired by Batocera / ES-DE

Stable base, advanced behaviors still evolving. Audio is intentionally kept simple and robust before adding complexity.

## 🧩 Installation Notes (Important)

ES-X uses git submodules (e.g. pugixml).

When building manually, the repository must be cloned recursively.

## 🚀 Installation

### ✅ Option A — Recommended (RetroPie)

For RetroPie users, use the official module:

- https://github.com/Renetrox/EmulationStation-X-Module-for-retropie

✔ Handles dependencies  
✔ Correct build flags  
✔ Clean RetroPie integration

### ⚙️ Option B — Manual Build (Advanced)

```
sudo apt update
sudo apt install cmake libsdl2-dev libboost-dev libfreeimage-dev \
libfreetype6-dev libcurl4-openssl-dev libvlc-dev libvlccore-dev \
libasound2-dev

git clone --recursive https://github.com/Renetrox/EmulationStation-X.git
cd EmulationStation-X
mkdir build && cd build
cmake ..
make -j4
sudo make install
```

If already cloned without submodules:

```
git submodule update --init --recursive
```

## 📂 Suggested Folder Structure

```
~/.emulationstation/
 ├── lang/
 ├── themes/
 │   └── Pi-Station-X/
 │       ├── _inc/
 │       ├── sounds/
 │       ├── layout/
 │       └── ...
```

## 🧭 Roadmap

| Priority | Focus |
|----------|-------|
| 🔄 | Full language coverage |
| 🎨 | More theme-driven layout control |
| 🔊 | Audio refinement & mixing |
| 📘 | Theme creator documentation |
| 🧪 | Careful experimentation without breaking stability |

## 👤 Author

Dino René Caballero Márquez (Renetrox)  
Educator • RetroPie Enthusiast • Theme Developer  
Paraguay  
GitHub: @Renetrox

## 📜 License

Same license as the original EmulationStation (MIT).

> Accessibility means not just using it — but reading it, hearing it, and shaping it your way.
