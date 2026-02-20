Temas (Themes)

En EmulationStation-X (ES-X), cada sistema puede tener su propio tema.

Un tema es:

Una colección de vistas (views)

Cada vista define elementos

Cada elemento tiene propiedades que controlan su apariencia y comportamiento

Normalmente, un tema se define en un archivo llamado:

<theme>
  <formatVersion>4</formatVersion>
  ...
</theme>

Pero lo más importante no es el XML.

Lo importante es que en ES-X crear y mantener temas es simple, escalable y modular.

Por qué crear temas en ES-X es sencillo

En ES-X podés:

✔ Crear un tema con un solo archivo
✔ Agregar múltiples layouts sin duplicar recursos
✔ Reutilizar imágenes, audio y fuentes
✔ Separar estructura, configuración y experiencia

La arquitectura está pensada para que crecer no signifique complicarse.

Orden de búsqueda de temas

Para cada sistema, ES-X intenta cargar theme.xml en este orden:

1️⃣ Tema directo del sistema

[SYSTEM_PATH]/theme.xml

2️⃣ Theme set del usuario

[HOME]/.emulationstation/themes/[CURRENT_THEME_SET]/[SYSTEM_THEME]/theme.xml

3️⃣ Theme set del sistema

/etc/emulationstation/themes/[CURRENT_THEME_SET]/[SYSTEM_THEME]/theme.xml

Si existen varios, se usa el primero encontrado.

Prioridad:

Ruta del sistema → HOME → /etc

Esto permite:

Personalización por sistema

Overrides locales

Organización limpia por theme sets

Estructura de un Theme Set

Un theme set es simplemente una carpeta organizada.

Ejemplo:

themes/
  mi_theme_set/
    snes/
      theme.xml

    nes/
      theme.xml

    _inc/
      backgrounds/
      logos/
      audio/

    theme.xml   ← fallback opcional

El theme.xml raíz puede actuar como tema por defecto.

No hay complejidad innecesaria.
Solo carpetas claras.

Conceptos Básicos
Vistas

Una vista representa una pantalla:

<view name="detailed">
  ...
</view>

Se pueden combinar vistas:

<view name="system, basic, detailed, grid">
  ...
</view>
Elementos

Un elemento es un componente visual:

image

text

textlist

video

carousel

imagegrid

rating

datetime

helpsystem

Modificar uno existente:

<text name="md_description">
  <color>FFFFFF</color>
</text>

Crear uno nuevo:

<image name="e_overlay" extra="true">
  <path>./overlay.png</path>
</image>

No necesitás definir todo.
Solo lo que querés cambiar.

Múltiples Layouts: Simple y Directo

En ES-X podés crear múltiples variaciones usando solo una carpeta:

layout/

Ejemplo:

mi_tema/
  layout/
    ps4/
      theme.xml
    ps3/
      theme.xml
    lite/
      theme.xml

  _inc/
    backgrounds/
    logos/

Eso es todo.

ES-X:

Detecta las carpetas

Las muestra en Theme Options

Activa la seleccionada

Mantiene los mismos recursos

No duplicás imágenes.
No duplicás audio.
No duplicás fuentes.

Solo cambiás lo necesario.

Variables

Las variables permiten reutilizar valores.

Variables del sistema:

${system.name}
${system.fullName}
${system.theme}
${system.mostPlayedImage}

Ejemplo:

<path>./_inc/consoles/${system.theme}.png</path>

Variables definidas por el tema:

<variables>
  <themeColor>8b0000</themeColor>
</variables>
Carrusel (System View)

El carrusel es personalizable:

<carousel name="systemcarousel">
  <logoScale>1.32</logoScale>
  <minLogoOpacity>0.94</minLogoOpacity>
  <scaledLogoSpacing>1</scaledLogoSpacing>
</carousel>

Podés ajustar:

Escala

Opacidad

Separación

Cantidad de logos

Sin tocar el core.
Solo XML.

Sonidos de navegación
<feature supported="navigationsounds">
  <sound name="scroll"><path>./audio/scroll.wav</path></sound>
</feature>

Solo .wav.

Simple.

Theme Options en ES-X
Donde la simplicidad se vuelve poderosa

Aquí está la diferencia clave.

Un tema en ES-X no es solo un XML.

Es:

theme.xml  → estructura visual
theme.ini  → decisiones y menú
Theme Options → experiencia del usuario
theme.ini

theme.ini define:

Qué opciones ve el usuario

Qué layouts existen

Qué valores se aplican

Ejemplo real:

[layout]
type=select
label=Layout del tema
apply_to=layout
values=ps4|PlayStation 4,
       ps3|PlayStation 3,
       lite|Lite
default=ps4

Las carpetas en layout/ contienen la estructura.
El theme.ini define cómo se eligen.

Separación clara.
Sin lógica compleja.
Sin scripts.

Flujo real

El tema define opciones en theme.ini

El usuario elige

ES-X carga la sección correspondiente

theme.xml usa los valores

El tema se redibuja

El XML no decide.
El INI no dibuja.
ES-X conecta ambos.

Ejemplo: Mini

Un solo XML.
Dos layouts.

[layout]
type=select
label=Layout
apply_to=layout
values=classic|Classic, carousel|Carousel
default=classic

✔ Sin duplicación
✔ Recursos compartidos
✔ Cambio inmediato

Filosofía de ES-X

Un tema debe ser:

Fácil de crear

Fácil de mantener

Fácil de ampliar

Múltiples variaciones no deberían implicar complejidad.

En ES-X, no la implican.

Conclusión

Crear un tema en ES-X es:

Estructura clara

Recursos reutilizables

Layouts simples

Opciones configurables

Sin sistemas complicados

Theme Options no son un extra.

Son la base que permite que un mismo tema tenga muchas versiones sin duplicación ni caos.

Pensá el tema como un sistema.
No como una imagen fija.