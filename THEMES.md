Themes

EmulationStation-X (ES-X) loads a theme for each system.
A theme is a collection of views, each view defines elements, and each element has properties.

A theme is usually stored in a theme.xml file.

Theme lookup order

For each system, ES-X will try to load a theme.xml in this order:

System path (per-system theme):

[SYSTEM_PATH]/theme.xml

Theme set in the user folder:

[HOME]/.emulationstation/themes/[CURRENT_THEME_SET]/[SYSTEM_THEME]/theme.xml

Theme set in the system folder:

/etc/emulationstation/themes/[CURRENT_THEME_SET]/[SYSTEM_THEME]/theme.xml

If more than one exists, the first match wins (system path overrides home theme set, home overrides /etc).

What is [SYSTEM_THEME]?

[SYSTEM_THEME] is taken from the system’s <theme> tag in es_systems.cfg.
If <theme> is not set, ES-X uses the system <name>.

What is [CURRENT_THEME_SET]?

[CURRENT_THEME_SET] is selected in UI Settings.
If not set or missing, ES-X uses the first available theme set.

Theme set folder structure

A theme set is a folder containing per-system theme folders plus optional shared resources.

Example:

themes/
  my_theme_set/
    snes/
      theme.xml
      my_background.jpg

    nes/
      theme.xml
      my_header.svg

    common_resources/
      scroll_sound.wav

    theme.xml   (default theme)


The theme.xml at the root of the theme set can act as a default theme if a system theme folder is missing.

Core concepts
Views

A view is a screen type inside ES-X.

Example:

<view name="detailed">
  ...
</view>


The name attribute can contain multiple views separated by whitespace and/or commas:

<view name="system, basic, detailed, grid">
  ...
</view>

Elements

An element is a UI component: image, text, list, video, carousel, etc.

You can:

A) Modify an existing element
<text name="md_description">
  <color>FFFFFF</color>
</text>

B) Create an extra element

Extra elements must use extra="true" and a unique name:

<image name="e_background_overlay" extra="true">
  <path>./my_overlay.png</path>
</image>


Extra elements are drawn in the order they appear, so define backgrounds first.

Properties

A property is a tag inside an element:

<pos>0.1 0.2</pos>
<color>FFFFFFFF</color>


You do not need to specify every property.

Advanced features
<formatVersion>

Every theme must define <formatVersion>.

<formatVersion>4</formatVersion>

<resolution>

Themes can optionally define a resolution so you can use pixel values instead of normalized values.

<resolution>1920 1080</resolution>


When resolution is set, ES-X will divide RESOLUTION_* values to normalize them.

Note: Parenting of elements is not available when using a resolution other than 1 1.

<include>

You can include one theme file into another:

<include>./../all_themes.xml</include>


Included properties get merged, and the current file can override values.

z-index rendering order

ES-X supports zIndex to control draw order.
Lower values draw first; higher values draw last.

Typical defaults (examples):

system view

Extra elements (extra="true") → 10

carousel name="systemcarousel" → 40

text name="systemInfo" → 50

basic/detailed/grid/video

background → 0

Extra elements → 10

textlist / imagegrid → 20

media (md_image, md_video) → 30

metadata → 40

logo/logoText → 50

Variables

Variables can be used in properties and paths.

System variables

Derived from es_systems.cfg and runtime data:

${system.name}

${system.fullName}

${system.theme}

${system.gameCount}

${system.displayedGameCount}

${system.favoriteCount}

${system.mostPlayedCount}

${system.mostPlayedName}

${system.mostPlayedFull}

${system.mostPlayedImage}

Example:

<text>${system.fullName}</text>
<path>./_inc/consoles/${system.theme}.png</path>

Theme-defined variables

You can define variables inside a theme:

<variables>
  <themeColor>8b0000</themeColor>
</variables>


Usage:

<color>${themeColor}c0</color>

ES-X extensions

This section documents features commonly used by modern ES-X themes.

Carousel (system view)

The system carousel displays system logos (or text) and supports customization.

Example:

<feature supported="carousel">
  <view name="system">
    <carousel name="systemcarousel">
      <type>horizontal</type>
      <pos>0.08 0.11</pos>
      <size>1.55 0.40</size>

      <logoAlignment>top</logoAlignment>
      <logoSize>0.21 0.23</logoSize>
      <logoScale>1.32</logoScale>

      <maxLogoCount>12</maxLogoCount>

      <!-- Opacity and spacing -->
      <minLogoOpacity>0.94</minLogoOpacity>
      <scaledLogoSpacing>1</scaledLogoSpacing>
    </carousel>
  </view>
</feature>

Carousel properties (important ones)

type: scroll direction (horizontal, vertical, horizontal_wheel, vertical_wheel)

pos, size, origin: position and size

logoSize: base logo size

logoScale: scale applied to the selected logo

logoAlignment: alignment inside carousel (top/bottom/center for horizontal)

maxLogoCount: how many logos appear

Opacity and spacing (the ones you asked)

minLogoOpacity
Controls how transparent the non-selected logos can be.

Closer to 1.0 → almost solid

Lower values → more “depth” / fading

scaledLogoSpacing
Controls spacing behavior when the selected logo is scaled.

Higher values → more separation (prevents crowding when center logo grows)

Lower values → tighter, more compact row

Navigation sounds (Batocera/ES-DE style)

Themes can define navigation sounds using:

<feature supported="navigationsounds">
  <view name="all">
    <sound name="systembrowse"><path>./_inc/audio/systembrowse.wav</path></sound>
    <sound name="quicksysselect"><path>./_inc/audio/quicksysselect.wav</path></sound>
    <sound name="select"><path>./_inc/audio/select.wav</path></sound>
    <sound name="back"><path>./_inc/audio/back.wav</path></sound>
    <sound name="scroll"><path>./_inc/audio/scroll.wav</path></sound>
    <sound name="favorite"><path>./_inc/audio/favorite.wav</path></sound>
    <sound name="launch"><path>./_inc/audio/launch.wav</path></sound>
  </view>
</feature>


Only .wav files are supported for <sound>.

Theme Options (ES-X)

ES-X can expose theme-defined options in the UI (Theme Options).
These options allow users to switch layouts or behaviors without editing XML.

Common examples:

Layout selection (ps4 / ps3 / lite)

Artwork source selection (boxart / image / marquee / thumbnail)

Performance mode toggles

If a theme defines no options, the Theme Options menu can be hidden.

theme.ini (ES-X)

ES-X supports an optional theme.ini alongside theme.xml.

Purpose:

Keep layout-specific positions and values in one place

Avoid duplicating XML blocks

Provide “knobs” for theme options (variables the theme can reuse)

Lookup order

theme.ini follows the same lookup idea as theme.xml (system folder first, then theme sets).

Syntax

INI key/value style:

Sections: [section]

Keys: key=value

Comments: ; or #

Using values in theme.xml

Keys from theme.ini are exposed as ${variables}.

Example:

<pos>${systemNamePos}</pos>
<maxSize>${mostPlayedMaxSize}</maxSize>

Layout sections

A common pattern is:

[layout.ps4]

[layout.ps3]

[layout.lite]

And a Theme Option chooses which layout section is active.

Reference
Property types

RESOLUTION_RECT: top left bottom right

RESOLUTION_PAIR: x y (pos/size)

RESOLUTION_FLOAT: 0.03 etc

NORMALIZED_PAIR: 0..1 percentages

PATH: ./relative/path or ~ expanded

STRING, COLOR (RGB/RGBA), FLOAT, BOOLEAN

Common element types

image, text, textlist, video, carousel, rating, datetime, helpsystem, sound, ninepatch, imagegrid

