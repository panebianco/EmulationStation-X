# EmulationStation-X (ES-X)
<img width="1280" height="720" alt="LOGO" src="https://github.com/user-attachments/assets/e8560813-41dd-4ad8-bf01-ecc6e03e922c" />

EmulationStation-X (ES-X) is a personal fork of EmulationStation focused on modern theming, runtime customization, and creator-driven UI control, while remaining compatible with RetroPie.

ES-X is inspired by modern frontends such as ES-DE, Batocera, and Recalbox, but follows a different philosophy:
👉 extend the engine, not replace it.

ES-X is experimental as a project, but stable as a platform.
Core features are usable today, while new capabilities evolve carefully to avoid breaking compatibility.

🎯 Purpose

ES-X does not aim to replace EmulationStation.

It exists as a sandbox for innovation, focused on:

Extending the theming engine without hacks

Giving theme creators real control through variables

Adding runtime localization (no recompilation)

Exploring modern UI & audio ideas safely

Staying RetroPie-friendly by design

Think of ES-X as:

“What EmulationStation could do if themes were systems, not skins.”

✨ Highlights

🌍 Runtime language system (.ini, no recompilation)

🎨 Theme Options (variable-driven layouts & styles)

🧩 Advanced theme variables (scale, spacing, opacity, layout logic)

🖌️ Text borders & shadows (engine-level)

🎶 Background music & navigation sounds (menu-controlled)

🌙 Dark menu mode

🔊 Sound tags compatible with ES-DE / Batocera (partial)

🛠️ Designed for theme creators first

🖼 Preview
<table>
  <tr>
    <td align="center">
      <strong>Theme Options</strong><br/>
      <img width="450" height="768" src="https://github.com/user-attachments/assets/7de22896-7b6b-43fb-910e-700d56ddb11d" alt="Theme options preview" />
    </td>
    <td align="center">
      <strong>Language Demo</strong><br/>
      <img width="450" height="768" src="https://github.com/user-attachments/assets/6e37ffc7-9c35-450e-b274-72d31b12a4f5" alt="Language demo preview" />
    </td>
  </tr>
  <tr>
    <td align="center">
      <strong>Variable Grid</strong><br/>
      <img width="450" height="252" src="https://github.com/user-attachments/assets/f4912b70-b738-46d7-8767-1d36995bdb6c" alt="Grid layout preview" />
    </td>
    <td align="center">
      <strong>Menu Styling</strong><br/>
      <img width="450" height="252" src="https://github.com/user-attachments/assets/4d9add99-955b-4b05-9b1f-37597fda2f1b" alt="Menu styling preview" />
    </td>
  </tr>
</table>

🔑 Current Feature Status

Feature	Status

Runtime language system (.ini)	✅ Stable

Menu, popup & scraper translation	✅ Stable

Theme variables (scale, spacing, opacity)	✅ Available

Theme Options (INI-driven)	✅ Available

Text borders & shadows	✅ Available

Dark menu mode	✅ Available

Background music (SDL_mixer)	🟡 Available / evolving

Menu sounds (scroll / select / back)	🟡 Available / evolving

ES-DE / Batocera sound tags	🟡 Partial

Advanced audio mixing / ducking	🔄 In progress

User profiles / avatars	❌ Not part of ES-X

🧩 Theme Options (Core Concept)

Theme Options are the heart of ES-X.

A modern ES-X theme is not a fixed skin, but a configurable system.

Key idea

theme.xml → draws the UI

theme.ini → defines options & decisions

ES-X → connects both at runtime

The XML does not decide.
The INI does not draw.
ES-X binds them together.

This allows:

multiple layouts with one XML

performance-friendly variants

console-like UX customization

zero duplication

📘 See: THEME_OPTIONS.md for full documentation and real examples
(Mini, ArtBook, Alekfull).

🌍 Language System (Runtime)

Languages are stored in:

~/.emulationstation/lang/


Example:

[MAIN]

MAIN MENU=MAIN MENU

YES=YES

BACK=BACK

CANCEL=CANCEL


✔ No recompilation
✔ Editable by users
✔ Loaded dynamically at runtime

🎨 Theme Engine Extensions

Available

<logoScale>1.1</logoScale>
<scaledLogoSpacing>0.20</scaledLogoSpacing>
<minLogoOpacity>0.25</minLogoOpacity>

<textBorderColor>000000</textBorderColor>
<textShadowColor>000000</textShadowColor>
<textShadowOffset>2 2</textShadowOffset>


Engine-level features — not hacks — designed to stay compatible with existing themes.

Planned / Proposed

<centerLogoOffsetY>
<centerLogoPos>
<zIndexOffset>

🔊 Audio & Music System

SDL_mixer-based background music

Menu-controlled playback

Navigation sounds (scroll / select / back)

Tag structure inspired by Batocera / ES-DE

The base is stable; advanced behavior is added carefully to preserve reliability.

---
🧩 Installation Notes (Important)

ES-X uses git submodules (e.g. pugixml).

When building manually, clone recursively or initialize submodules before running CMake.

🚀 Installation
✅ Option A — Recommended (RetroPie)

Use the official RetroPie module:

👉 https://github.com/Renetrox/EmulationStation-X-Module-for-retropie

✔ Handles dependencies
✔ Correct build flags
✔ Clean RetroPie integration

⚙️ Option B — Manual Build (Advanced)

sudo apt update
sudo apt install build-essential cmake libsdl2-dev libsdl2-mixer-dev \
libboost-dev libfreeimage-dev libfreetype6-dev libcurl4-openssl-dev \
libvlc-dev libvlccore-dev libasound2-dev

git clone --recursive https://github.com/Renetrox/EmulationStation-X.git

cd EmulationStation-X
mkdir build && cd build
cmake ..
make -j4


If already cloned without submodules:

git submodule update --init --recursive

---
🧭 Roadmap
Focus
Full language coverage
More theme-driven layout control
Audio refinement & mixing
Theme creator documentation
Careful experimentation without breaking stability

👤 Author

- Dino René Caballero Márquez (Renetrox)
- Educator • RetroPie Enthusiast • Theme Developer
- Paraguay
- GitHub: @Renetrox

---
📜 License

Same license as the original EmulationStation (MIT).

Accessibility is not just using it —
it’s reading it, hearing it, and shaping it your way.
