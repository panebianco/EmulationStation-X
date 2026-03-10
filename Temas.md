Sistema de Temas ES-X
EmulationStation-X (ES-X) carga un tema para cada sistema.

Un tema se compone simplemente de:

Vistas (Views)

Elementos (Elements)

Propiedades (Properties)

La mayoría de los temas se definen en:

theme.xml

Este archivo define la estructura visual de la interfaz.

ES-X está diseñado para que los temas sigan siendo simples, modulares y escalables, incluso cuando aumenta su complejidad.

Orden de Búsqueda del Tema
Para cada sistema, ES-X busca el archivo theme.xml siguiendo esta prioridad:

Ruta del sistema (tema por sistema)

[RUTA_DEL_SISTEMA]/theme.xml

Set de temas del usuario [HOME]/.emulationstation/themes/[SET_DE_TEMAS_ACTUAL]/[TEMA_DEL_SISTEMA]/theme.xml

Set de temas del sistema /etc/emulationstation/themes/[SET_DE_TEMAS_ACTUAL]/[TEMA_DEL_SISTEMA]/theme.xml

Si existen varios temas, prevalece la primera coincidencia.

Orden de prioridad: Ruta del sistema → Set de temas Home → Set de temas /etc

Esta arquitectura permite:

Sobrescritura por sistema.

Personalización del usuario.

Separación limpia entre temas globales y locales.

Identificadores de Tema
SYSTEM_THEME
Este valor proviene de la etiqueta <theme> en es_systems.cfg.

Ejemplo: <theme>snes</theme>

Si <theme> no está definido, ES-X utiliza el <name> (nombre) del sistema.

CURRENT_THEME_SET
Se selecciona en:

Configuración de UI → Set de Temas

Si no está definido, ES-X carga el primer set de temas disponible.

Estructura de Carpetas del Set de Temas
Un set de temas es simplemente una carpeta estructurada:

Plaintext
themes/
  mi_set_de_temas/
    snes/
      theme.xml
    nes/
      theme.xml
    _inc/
      backgrounds/
      logos/
      audio/
    theme.xml
El theme.xml de la raíz actúa como respaldo (fallback) si falta la carpeta de un sistema. Esto mantiene los temas modulares, reutilizables y fáciles de mantener.

Conceptos Críticos
Vistas (Views)
Una vista representa un tipo de pantalla.

Ejemplo:

XML
<view name="detailed">
  ...
</view>
Varias vistas pueden compartir el mismo diseño:

XML
<view name="system, basic, detailed, grid">
  ...
</view>
Elementos (Elements)
Son los componentes de la interfaz usados dentro de las vistas.

Elementos comunes: image, text, video, carousel, textlist, imagegrid, rating, datetime, helpsystem.

Puedes modificar elementos existentes o crear nuevos:

Modificar un elemento existente:

XML
<text name="md_description">
  <color>FFFFFF</color>
</text>
Crear un nuevo elemento:

XML
<image name="e_overlay" extra="true">
  <path>./overlay.png</path>
</image>
Los elementos "extra" se renderizan en orden de declaración. Los elementos de fondo (background) generalmente deben definirse primero. Los temas solo necesitan definir lo que desean cambiar.

Propiedades (Properties)
Controlan el comportamiento del elemento. No es necesario definir todas las propiedades, solo sobrescribir las importantes.

XML
<pos>0.1 0.2</pos>
<color>FFFFFFFF</color>
Etiquetas Requeridas y Avanzadas
formatVersion: Requerido en cada tema.

<formatVersion>4</formatVersion>

resolution: Posicionamiento opcional basado en píxeles.

<resolution>1920 1080</resolution>

ES-X normaliza las coordenadas automáticamente. ⚠ El "parenting" (jerarquía padre-hijo) no es compatible cuando la resolución es distinta a 1 1.

include: Permite archivos de tema modulares.

<include>./shared.xml</include>

Orden de Renderizado zIndex
zIndex controla las capas de renderizado. Los valores más bajos se renderizan primero; los más altos aparecen encima.

Vista de Sistema Típica:
| Elemento | zIndex |
| :--- | :--- |
| Elementos extra | 10 |
| systemcarousel | 40 |
| systemInfo | 50 |

Vista de Gamelist Típica:
| Elemento | zIndex |
| :--- | :--- |
| background | 0 |
| Extra | 10 |
| list / grid | 20 |
| media | 30 |
| metadata | 40 |
| logo | 50 |

Variables
Las variables simplifican la reutilización y el escalado.

Variables de Sistema (Automáticas)
${system.name}, ${system.fullName}, ${system.theme}, ${system.gameCount}, ${system.mostPlayedImage}.

Ejemplo: <path>./_inc/consoles/${system.theme}.png</path>

Variables de Tema
Definidas dentro del tema:

XML
<variables>
  <themeColor>8b0000</themeColor>
</variables>
Uso: <color>${themeColor}c0</color>

Extensiones de ES-X
Carrusel (Vista de Sistema)
XML
<carousel name="systemcarousel">
  <logoScale>1.32</logoScale>
  <minLogoOpacity>0.94</minLogoOpacity>
  <scaledLogoSpacing>1</scaledLogoSpacing>
</carousel>
Propiedades importantes: logoScale, minLogoOpacity, scaledLogoSpacing.

La profundidad y el espaciado se pueden ajustar totalmente sin modificar el código del motor.

Sonidos de Navegación
Los temas pueden definir audio de navegación (solo archivos .wav).

XML
<feature supported="navigationsounds">
  <view name="all">
    <sound name="scroll">
      <path>./audio/scroll.wav</path>
    </sound>
  </view>
</feature>
Opciones de Tema (ES-X)
Los temas pueden exponer opciones configurables en la interfaz:

Selección de diseño (ps4 / ps3 / lite).

Selección de fuente de arte (artwork).

Interruptores de rendimiento.

Modos de densidad.

theme.ini (ES-X)
Es opcional pero potente. Permite centralizar configuraciones y reducir la duplicación de XML.

Uso de valores INI en theme.xml: Las llaves INI se convierten en variables.

XML
<pos>${systemNamePos}</pos>
<maxSize>${mostPlayedMaxSize}</maxSize>

Elemento Reloj (ES-X)
ES-X incluye un reloj del sistema integrado que los temas pueden estilizar.

Se define como un elemento de texto dentro de la vista de pantalla (screen).

XML
<view name="screen">
  <text name="clock">
    <pos>0.984 0.03</pos>
    <origin>1 0.5</origin>
    <color>FFFFFFFF</color>
    <fontPath>./_inc/fonts/Exo2-Medium.ttf</fontPath>
    <fontSize>0.018</fontSize>
  </text>
</view>
Formato: El formato de hora (24H/12H) es controlado por el usuario, no por el tema.

Renderizado: Se actualiza suavemente una vez por segundo con un uso mínimo de CPU.

Filosofía de Diseño
ES-X separa las responsabilidades:

theme.xml → Estructura

theme.ini → Configuración

Opciones de Tema → Experiencia de usuario

Esto hace que los temas sean más fáciles de mantener, expandir y adaptar a diferentes hardwares. Los temas modernos no se tratan de complejidad, sino de flexibilidad sin caos.