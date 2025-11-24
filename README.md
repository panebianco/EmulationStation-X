# EmulationStation-X (ES-X)

**ES-X** is a personal and experimental fork of **EmulationStation**, focused on:

🎨 Enhancing theme capabilities (alignment, spacing, zIndex behavior, carousel layout)  
🌎 Adding runtime language localization using simple `.ini` files (no recompilation)  
🧩 Exploring optional theme attributes inspired by ES-DE and Batocera  
🔊 Experimental work toward sound hooks (not yet functional)  

> ⚠️ ES-X does **not** include avatar systems, usernames, or profile management.  
> Those features belong to custom themes (like Pi-Station-X), not to the ES-X core.

---

## 🎯 Purpose

ES-X does **not** aim to replace EmulationStation.  
It is a **sandbox for new ideas** — testing approaches to localization, UI flexibility, theme expressiveness, and user accessibility, while maintaining compatibility with **RetroPie**.

---

## 🔑 Current Features

| Feature | Status |
|---------|--------|
| `.ini` language system (runtime) | ✔ Implemented |
| Translation in main menus, popups, scraper start | ✔ Implemented |
| Extendable to other GUI components | 🔄 In progress |
| Theme enhancements (logoScale, scaledLogoSpacing, minLogoOpacity) | ✔ Working |
| Text color and shadow improvements | ✔ Working / experimental |
| Proposed tags for theme flexibility (`centerLogoOffsetY`, `zIndexOffset`) | 🧪 Design / planned |
| Sound engine hooks | 🚧 Very early / not functional |
| Username, avatar, dynamic profile | ✖ Not part of ES-X (theme only) |

---

## 🗂️ Language System

Languages are stored in:

~/.emulationstation/lang/



Example `.ini` file:

```ini
[MAIN]
MAIN MENU=MENÚ PRINCIPAL
YES=SÍ
BACK=VOLVER
CANCEL=CANCELAR
✔ No need to recompile
✔ Anyone can edit or add new languages
✔ Text is loaded dynamically at runtime

🎨 Theme Extensions (Engine-Level)
Supported or partially tested theme tags:


<logoScale>1.1</logoScale>
<scaledLogoSpacing>0.20</scaledLogoSpacing>
<minLogoOpacity>0.25</minLogoOpacity>
Concept / planned (not active yet):


<centerLogoOffsetY>
<zIndexOffset>
<centerLogoPos>
⚠️ Avatar, username, WiFi icon, or floating system information
are theme-specific additions and not part of ES-X core.
<img width="1366" height="768" alt="Captura de pantalla_2025-11-24_16-19-55" src="https://github.com/user-attachments/assets/6e37ffc7-9c35-450e-b274-72d31b12a4f5" />

🚀 Build Instructions (Linux / Raspberry Pi / Orange Pi)

sudo apt update
sudo apt install cmake libsdl2-dev libboost-dev libfreeimage-dev \
libfreetype6-dev libcurl4-openssl-dev libvlc-dev libvlccore-dev \
libasound2-dev

git clone https://github.com/Renetrox/EmulationStation-X.git
cd EmulationStation-X
mkdir build && cd build
cmake ..
make -j4
sudo make install

📂 Suggested User Folder Structure
<img width="1366" height="768" alt="Captura de pantalla_2025-11-24_16-20-34" src="https://github.com/user-attachments/assets/ee725018-2b51-4ab1-98b9-0750e303d0e7" />

~/.emulationstation/
 ├── lang/              # Language files (.ini)
 ├── themes/
 │   └── Pi-Station-X/   # Example theme using ES-X features
 │       ├── _inc/
 │       ├── sounds/
 │       ├── layout/
 │       └── ...
🛣️ Roadmap
Priority	Feature
🔄	Complete language coverage for all GUIs
🧪	Experiment with theme flexibility (spacing, alignment)
🚧	Build test branch for basic sound hooks
📘	Document new theme attributes
💭	Keep ES-X experimental — fork, not replacement

👤 Author
Dino René Caballero Marquez — Renetrox
Educator • RetroPie Enthusiast • Theme Developer
Paraguay
GitHub: @Renetrox

📜 License
Same license as original EmulationStation (MIT License).
Experimental features may change, evolve, or be removed at any time.

"Accessibility means not just using it — but reading it, hearing it, and shaping it your way."
