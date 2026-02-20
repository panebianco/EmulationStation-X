Theme Options in ES-X
The Core of Modern Themes

In ES-X, Theme Options are not an extra feature.

They are the foundation that allows themes to grow without becoming complex.

A modern theme in ES-X is not a fixed skin.
It is a configurable system.

theme.ini

Theme Options are defined in an optional file called:

theme.ini

This file:

Defines which options the user sees

Defines internal theme variables

Controls which values are applied based on the selected option

theme.xml contains no logic.
It only consumes variables.

That separation is intentional.

Real Execution Flow

The theme defines options in theme.ini

The user selects an option in Theme Options

ES-X loads the corresponding INI section

Variables are exposed to theme.xml

The theme redraws using those variables

The XML does not decide.
The INI does not draw.
ES-X connects both.

This makes themes easier to understand and easier to maintain.

Theme Architecture in ES-X
Building Themes the Right Way

Themes in ES-X are flexible by design.

You are not locked into one structure.
You can choose the approach that fits your project.

There are two supported ways to build a theme:

1️⃣ Layout-Based (classic structure)
2️⃣ Variable-Based (modern ES-X approach)

Both work.
Both are valid.
But they solve different problems.

1️⃣ Layout-Based Themes (Classic Approach)

This is the traditional method inherited from EmulationStation.

You create separate layout folders:

layout/
 ├─ ps4/
 ├─ ps5/
 ├─ mini/

Each layout contains its own XML structure.

Why use it?

Ideal for porting existing themes

Fully compatible with classic EmulationStation

Very simple mental model

Good when layouts are radically different

What to consider

If layouts share many elements, XML duplication can happen.

That’s normal in the classic method.

It works well, but can grow harder to maintain over time.

2️⃣ Variable-Based Themes (Modern ES-X Approach)

This is the recommended method for new ES-X themes.

Instead of switching entire XML files, you keep a single structure and control it with:

theme.ini

You are not switching layouts.

You are switching configurations.

How it works

theme.xml defines the visual structure

theme.ini defines options and internal values

Theme Options exposes them to the user

ES-X applies the selected configuration

The same XML adapts dynamically

Why this method is powerful

One XML structure

No structural duplication

Easier long-term maintenance

Cleaner scalability

More flexibility for users

You can control:

Layout density

Artwork sources

Description visibility

Carousel scale and spacing

Navigation sounds

Performance presets

All without rewriting the theme.

Real-World Examples
Mini

✔ One XML
✔ Multiple layout modes
✔ No duplication

A clear demonstration of simplicity.

ArtBook

Uses Theme Options to control experience:

Density

Artwork source

Description visibility

One structure. Many variations.

Alekfull

Uses Theme Options for fine control:

Carousel scale

Logo opacity

Logo spacing

Navigation sounds

The same theme can feel modern or classic depending on configuration.

When Should You Use Each Method?
Situation	Recommended Approach
Porting older themes	Layout-Based
Supporting classic ES	Layout-Based
Creating a new ES-X theme	Variable-Based
Building a configurable modern system	Variable-Based

You can even mix both approaches.

The ES-X Philosophy
theme.xml   → visual structure
theme.ini   → configuration logic
Theme Options → user experience

Themes should grow without becoming complicated.

In ES-X:

Structure is clear

Configuration is separate

Resources are reusable

Variations are easy

Modern themes are not about complexity.

They are about flexibility.

Final Thought

You can start simple.

You can expand later.

You can port.
You can innovate.
You can combine approaches.

ES-X does not force a rigid system.

It gives you a clean architecture that stays simple — even as your theme evolves.

Think of a theme as a system.
Not as a static image.