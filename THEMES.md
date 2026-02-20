Themes

EmulationStation-X (ES-X) loads a theme for each system.

A theme is simply:

A collection of views

Each view contains elements

Each element has properties

Most themes are defined in:

theme.xml

That’s the visual structure.

But ES-X is built so themes can stay simple even as they grow.

Theme Lookup Order

For each system, ES-X searches for theme.xml in this order:

1️⃣ System path (per-system theme)

[SYSTEM_PATH]/theme.xml

2️⃣ User theme set

[HOME]/.emulationstation/themes/[CURRENT_THEME_SET]/[SYSTEM_THEME]/theme.xml

3️⃣ System theme set

/etc/emulationstation/themes/[CURRENT_THEME_SET]/[SYSTEM_THEME]/theme.xml

If multiple exist, the first match wins.

Priority:

System path → Home theme set → /etc theme set

This allows:

Per-system overrides

User customization

Clean separation of global vs local themes

What is [SYSTEM_THEME]?

It comes from the <theme> tag in es_systems.cfg.

If <theme> is not defined, ES-X uses the system <name>.

What is [CURRENT_THEME_SET]?

Selected in UI Settings.

If missing, ES-X loads the first available theme set.

Theme Set Folder Structure

A theme set is just a structured folder:

themes/
  my_theme_set/
    snes/
      theme.xml

    nes/
      theme.xml

    _inc/
      backgrounds/
      logos/
      audio/

    theme.xml   ← optional fallback

The root theme.xml acts as a default if a system folder is missing.

Simple structure.
No hidden complexity.

Core Concepts
Views

A view represents a screen type:

<view name="detailed">
  ...
</view>

Multiple views can share the same structure:

<view name="system, basic, detailed, grid">
  ...
</view>
Elements

Elements are UI components:

image

text

video

carousel

textlist

imagegrid

rating

datetime

helpsystem

You can:

Modify an existing element
<text name="md_description">
  <color>FFFFFF</color>
</text>
Create a new element
<image name="e_overlay" extra="true">
  <path>./overlay.png</path>
</image>

Extra elements render in order.
Define backgrounds first.

You only define what you want to change.

Properties

Properties define behavior:

<pos>0.1 0.2</pos>
<color>FFFFFFFF</color>

You don’t need to define every property.
Only override what matters.

Required & Advanced Tags
<formatVersion>

Required:

<formatVersion>4</formatVersion>
<resolution>

Optional pixel-based positioning:

<resolution>1920 1080</resolution>

ES-X normalizes coordinates automatically.

⚠ Parenting is not supported when resolution ≠ 1 1.

<include>

Merge multiple theme files:

<include>./shared.xml</include>

Clean modular structure.
Reusable components.

zIndex Rendering Order

zIndex controls layering.

Lower values → drawn first
Higher values → drawn on top

Typical System View
Element	zIndex
Extra elements	10
systemcarousel	40
systemInfo	50
Gamelist Views
Element	zIndex
background	0
Extra	10
list/grid	20
media	30
metadata	40
logo	50
Variables

Variables simplify reuse and scaling.

System Variables

Available automatically:

${system.name}
${system.fullName}
${system.theme}
${system.gameCount}
${system.mostPlayedImage}

Example:

<path>./_inc/consoles/${system.theme}.png</path>
Theme-Defined Variables
<variables>
  <themeColor>8b0000</themeColor>
</variables>

Usage:

<color>${themeColor}c0</color>
ES-X Extensions
Carousel (System View)
<carousel name="systemcarousel">
  <logoScale>1.32</logoScale>
  <minLogoOpacity>0.94</minLogoOpacity>
  <scaledLogoSpacing>1</scaledLogoSpacing>
</carousel>

Important properties:

logoScale

minLogoOpacity

scaledLogoSpacing

Depth and spacing are fully adjustable without touching core code.

Navigation Sounds
<feature supported="navigationsounds">
  <view name="all">
    <sound name="scroll">
      <path>./audio/scroll.wav</path>
    </sound>
  </view>
</feature>

Only .wav supported.

Theme Options (ES-X)

This is where ES-X becomes truly flexible.

Themes can expose configurable options in the UI:

Layout selection (ps4 / ps3 / lite)

Artwork source selection

Performance toggles

Density modes

If no options are defined, the Theme Options menu can be hidden.

theme.ini (ES-X)

theme.ini is optional — but powerful.

It allows you to:

Centralize configuration

Avoid XML duplication

Control layout variations

Define user-selectable options

Syntax
[section]
key=value

Comments:

; comment
# comment
Using INI values in theme.xml

Keys become variables:

<pos>${systemNamePos}</pos>
<maxSize>${mostPlayedMaxSize}</maxSize>
Layout Section Pattern

Common structure:

[layout.ps4]
[layout.ps3]
[layout.lite]

Theme Options selects which section becomes active.

Why This Architecture Matters

ES-X separates:

theme.xml   → structure
theme.ini   → configuration
Theme Options → user experience

This makes themes:

Easier to maintain

Easier to expand

Easier to reuse

Easier to adapt to different hardware

Modern themes are not about complexity.

They are about flexibility without chaos.

Reference
Property Types

RESOLUTION_RECT

RESOLUTION_PAIR

RESOLUTION_FLOAT

NORMALIZED_PAIR

PATH

STRING

COLOR

FLOAT

BOOLEAN

Common Element Types

image

text

textlist

video

carousel

rating

datetime

helpsystem

sound

ninepatch

imagegrid