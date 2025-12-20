Temas (Themes)

EmulationStation-X (ES-X) permite que cada sistema tenga su propio tema.
Un tema es una colección de vistas (views); cada vista define elementos, y cada elemento tiene propiedades que controlan su apariencia y comportamiento.

Normalmente, un tema se define en un archivo llamado theme.xml.

Orden de búsqueda de temas

Para cada sistema, ES-X intenta cargar un theme.xml en el siguiente orden:

Tema por sistema (ruta del sistema):

[SYSTEM_PATH]/theme.xml

Theme set del usuario:

[HOME]/.emulationstation/themes/[CURRENT_THEME_SET]/[SYSTEM_THEME]/theme.xml

Theme set del sistema:

/etc/emulationstation/themes/[CURRENT_THEME_SET]/[SYSTEM_THEME]/theme.xml

Si existen varios archivos, ES-X usa el primero que encuentre (la ruta del sistema tiene prioridad, luego HOME, luego /etc).

¿Qué es [SYSTEM_THEME]?

[SYSTEM_THEME] proviene del tag <theme> definido para el sistema en es_systems.cfg.
Si no se define <theme>, ES-X utiliza el <name> del sistema.

¿Qué es [CURRENT_THEME_SET]?

Es el theme set activo, seleccionado en el menú UI Settings.
Si no hay ninguno seleccionado o el anterior no existe, ES-X usa el primer theme set disponible.

Estructura de un theme set

Un theme set es una carpeta que contiene temas por sistema y recursos compartidos opcionales.

Ejemplo:

themes/
  mi_theme_set/
    snes/
      theme.xml
      fondo.jpg

    nes/
      theme.xml
      header.svg

    common_resources/
      scroll_sound.wav

    theme.xml   (tema por defecto)


El theme.xml ubicado en la raíz del theme set puede actuar como tema por defecto cuando no existe una carpeta específica para un sistema.

Conceptos básicos
Vistas (Views)

Una vista representa un tipo de pantalla dentro de ES-X.

Ejemplo:

<view name="detailed">
  ...
</view>


El atributo name puede contener varias vistas separadas por espacios o comas:

<view name="system, basic, detailed, grid">
  ...
</view>

Elementos

Un elemento es un componente visual: imagen, texto, lista, video, carrusel, etc.

Podés:

A) Modificar un elemento existente
<text name="md_description">
  <color>FFFFFF</color>
</text>

B) Crear un elemento extra

Los elementos nuevos deben usar extra="true" y un nombre único:

<image name="e_overlay_fondo" extra="true">
  <path>./overlay.png</path>
</image>


Los elementos extra se dibujan en el orden en que aparecen, por lo que conviene definir fondos primero.

Propiedades

Una propiedad es un tag dentro de un elemento:

<pos>0.1 0.2</pos>
<color>FFFFFFFF</color>


No es obligatorio definir todas las propiedades; solo las necesarias.

Funciones avanzadas
<formatVersion>

Todo tema debe definir <formatVersion>:

<formatVersion>4</formatVersion>


Este valor indica la versión del sistema de temas para la cual fue diseñado el tema.

<resolution>

Permite diseñar temas usando valores en píxeles en lugar de porcentajes.

<resolution>1920 1080</resolution>


Cuando se define resolution, los valores de tipo RESOLUTION_* se normalizan automáticamente.

Nota: no se puede usar “parenting” de elementos cuando la resolución es distinta de 1 1.

<include>

Permite incluir otros archivos de tema, de forma similar a #include en C:

<include>./../comun.xml</include>


Las propiedades se fusionan, y el archivo actual puede sobrescribir valores incluidos.

Orden de renderizado (z-index)

ES-X soporta la propiedad zIndex para controlar el orden de dibujo.

Valores bajos → se dibujan primero

Valores altos → se dibujan encima

Ejemplos típicos:

Vista system

Elementos extra → 10

systemcarousel → 40

systemInfo → 50

Vistas basic / detailed / grid / video

Fondo → 0

Elementos extra → 10

Listas / grillas → 20

Media (imagen / video) → 30

Metadatos → 40

Logo / texto del sistema → 50

Variables

Las variables permiten reutilizar valores y simplificar los temas.

Variables del sistema

Derivadas de es_systems.cfg y del estado del sistema:

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

Ejemplo:

<text>${system.fullName}</text>
<path>./_inc/consoles/${system.theme}.png</path>

Variables definidas por el tema

Un tema puede definir sus propias variables:

<variables>
  <themeColor>8b0000</themeColor>
</variables>


Uso:

<color>${themeColor}c0</color>

Extensiones ES-X

Esta sección documenta funciones modernas utilizadas por temas avanzados en ES-X.

Carrusel (vista system)

El carrusel muestra los sistemas mediante logos o texto.

Ejemplo:

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

      <minLogoOpacity>0.94</minLogoOpacity>
      <scaledLogoSpacing>1</scaledLogoSpacing>
    </carousel>
  </view>
</feature>

Propiedades importantes del carrusel

type: dirección del carrusel (horizontal, vertical, horizontal_wheel, vertical_wheel)

pos, size, origin: posición y tamaño

logoSize: tamaño base del logo

logoScale: escala aplicada al logo seleccionado

logoAlignment: alineación dentro del carrusel

maxLogoCount: cantidad de logos visibles

Opacidad y separación

minLogoOpacity
Controla la opacidad mínima de los logos no seleccionados.
Valores bajos generan más profundidad visual.

scaledLogoSpacing
Controla la separación entre logos considerando el logoScale.
Valores altos evitan que el logo central “aplastado” a los laterales.

Sonidos de navegación

Los temas pueden definir sonidos de navegación al estilo Batocera / ES-DE:

<feature supported="navigationsounds">
  <view name="all">
    <sound name="systembrowse"><path>./audio/systembrowse.wav</path></sound>
    <sound name="select"><path>./audio/select.wav</path></sound>
    <sound name="back"><path>./audio/back.wav</path></sound>
    <sound name="scroll"><path>./audio/scroll.wav</path></sound>
    <sound name="launch"><path>./audio/launch.wav</path></sound>
  </view>
</feature>


Solo se admiten archivos .wav.

Theme Options en ES-X
El núcleo de los temas modernos

En EmulationStation-X, el concepto más importante para los temas no es el XML, sino Theme Options.

Un tema moderno en ES-X no es un skin fijo, es un sistema configurable.

¿Qué son las Theme Options?

Las Theme Options permiten que un tema exponga opciones configurables desde la interfaz gráfica, sin que el usuario edite archivos XML.

Estas opciones permiten cambiar:

layouts completos

comportamientos visuales

fuentes de arte

nivel de detalle o rendimiento

👉 Todo desde el menú, en tiempo real.

Dónde se definen: theme.ini

Las Theme Options se definen en un archivo opcional llamado:

theme.ini


Este archivo:

define qué opciones ve el usuario

define variables internas del tema

controla qué valores se aplican según la opción elegida

El theme.xml no tiene lógica, solo consume variables.

Flujo real de funcionamiento

Este es el flujo correcto (y clave para entender ES-X):

El tema define opciones en theme.ini

El usuario selecciona una opción en Theme Options

ES-X carga la sección correspondiente del INI

Las variables se exponen al theme.xml

El tema se redibuja usando esas variables

🔹 El XML no decide nada
🔹 El INI no dibuja nada
🔹 ES-X conecta ambos

Ejemplos reales (Mini, ArtBook, Alekfull)
Mini (ejemplo directo y simple)

Mini es un ejemplo claro de Theme Options bien usadas, sin complejidad innecesaria.

Opción visible
[layout]
type=select
label=Layout
apply_to=layout
values=classic|Classic,
       carousel|Carousel
default=classic


El usuario ve:

Theme Options →
  Layout →
    Classic
    Carousel

Secciones internas
[layout_classic]
logoScale=1.0
logoOpacity=1.0
bgMain=./graphics/bg_classic.png

[layout_carousel]
logoScale=1.3
logoOpacity=0.7
bgMain=./graphics/bg_carousel.png

Uso en theme.xml
<carousel name="systemcarousel">
    <logoScale>${logoScale}</logoScale>
    <minLogoOpacity>${logoOpacity}</minLogoOpacity>
</carousel>

<image name="background">
    <path>${bgMain}</path>
</image>


✔ Un solo XML
✔ Dos layouts
✔ Sin duplicación

ArtBook (Theme Options como selector de experiencia)

ArtBook usa Theme Options para cambiar cómo se presenta la información, no solo el look.

Ejemplos típicos:

mostrar u ocultar descripciones

elegir fuente principal de imágenes

cambiar densidad visual

Concepto clave

ArtBook no duplica vistas, usa variables para:

posiciones

tamaños

fuentes de arte

Esto permite:

pantallas grandes

pantallas chicas

hardware potente o limitado

Todo con el mismo theme.xml.

Alekfull (Theme Options como control fino)

Alekfull es un ejemplo de Theme Options usadas para ajuste fino del carrusel y la navegación.

Usa opciones para controlar:

escala del logo central

opacidad de logos laterales

separación entre logos

sonidos de navegación

Ejemplo conceptual:

[layout_modern]
carouselLogoScale=1.35
carouselLogoOpacity=0.6
carouselSpacing=1.2


Y en XML:

<carousel name="systemcarousel">
    <logoScale>${carouselLogoScale}</logoScale>
    <minLogoOpacity>${carouselLogoOpacity}</minLogoOpacity>
    <scaledLogoSpacing>${carouselSpacing}</scaledLogoSpacing>
</carousel>


Esto permite que el mismo tema se sienta:

más “consola moderna”

o más “clásico”
según la opción elegida.

Tipos de Theme Options en ES-X
select

Selector con múltiples valores.

Usado para:

layouts

estilos visuales

fuentes de arte

bool

Interruptor ON/OFF.

Usado para:

overlays

animaciones

indicadores

efectos visuales

Variables internas (sin type)

No aparecen en el menú.

Usadas para:

posiciones

rutas

tamaños

ajustes por layout

Por qué Theme Options son el corazón de ES-X

Porque permiten:

✔ Un solo tema → muchos estilos
✔ Adaptación a hardware débil o potente
✔ Personalización sin romper compatibilidad
✔ Temas más fáciles de mantener
✔ UX moderna tipo consola real

Un tema sin Theme Options sigue funcionando,
pero no aprovecha ES-X.

Filosofía correcta para crear temas en ES-X

theme.xml → estructura visual

theme.ini → configuración y decisiones

Theme Options → experiencia del usuario

👉 Pensar el tema como un sistema, no como una imagen fija.

Conclusión

Theme Options no son un extra.
Son la base de los temas modernos en ES-X.
