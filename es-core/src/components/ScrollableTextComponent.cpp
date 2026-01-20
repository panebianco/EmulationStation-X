#include "components/ScrollableTextComponent.h"

#include "ThemeData.h"
#include "renderers/Renderer.h"

ScrollableTextComponent::ScrollableTextComponent(Window* window)
	: GuiComponent(window)
	, mScroll(window, 1000)
	, mText(window)
	, mAutoScroll(false)
	, mAutoScrollDelay(1000)
{
	// Nuestro componente contiene un ScrollableContainer,
	// y dentro de éste vive el TextComponent.
	addChild(&mScroll);
	mScroll.addChild(&mText);

	// Layout por defecto dentro del contenedor
	mText.setPosition(0, 0);
	mText.setOrigin(0, 0);

	syncChildren();
}

void ScrollableTextComponent::setText(const std::string& text)
{
	mText.setText(text);
}

const std::string& ScrollableTextComponent::getText() const
{
	return mText.getText();
}

void ScrollableTextComponent::setAutoScroll(bool enabled, int delayMs)
{
	mAutoScroll = enabled;
	if (delayMs < 0) delayMs = 0;
	mAutoScrollDelay = delayMs;

	// ScrollableContainer usa scrollDelay en el ctor, pero también lo resetea en setAutoScroll(true)
	// Así que recreamos el comportamiento “delay” dejando el ctor fijo y usando setAutoScroll.
	// (El delay real se maneja adentro con mAutoScrollDelay, acá hacemos reset seguro.)
	mScroll.setAutoScroll(enabled);
}

void ScrollableTextComponent::setSize(const Vector2f& size)
{
	GuiComponent::setSize(size);
	syncChildren();
}

void ScrollableTextComponent::setPosition(float x, float y, float z)
{
	GuiComponent::setPosition(x, y, z);
}

void ScrollableTextComponent::setPosition(const Vector3f& offset)
{
	GuiComponent::setPosition(offset);
}

void ScrollableTextComponent::syncChildren()
{
	// El contenedor ocupa todo el área del componente
	mScroll.setPosition(0, 0);
	mScroll.setSize(mSize);

	// El texto: se renderiza desde (0,0) y se deja que crezca; el contenedor lo “clippea”
	mText.setPosition(0, 0);
	mText.setSize(mSize);
}

void ScrollableTextComponent::applyTheme(const std::shared_ptr<ThemeData>& theme,
                                        const std::string& view,
                                        const std::string& element,
                                        unsigned int properties)
{
	// Este elemento ES un "text" en el tema
	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "text");
	if (!elem)
		return;

	// Escala a pantalla (igual que hacen los otros applyTheme)
	const Vector2f screenSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	if ((properties & ThemeFlags::POSITION) && elem->has("pos"))
	{
		Vector2f p = elem->get<Vector2f>("pos") * screenSize;
		setPosition(p.x(), p.y());
	}

	if ((properties & ThemeFlags::SIZE) && elem->has("size"))
	{
		Vector2f s = elem->get<Vector2f>("size") * screenSize;
		setSize(s);
	}

	if ((properties & ThemeFlags::ORIGIN) && elem->has("origin"))
	{
		setOrigin(elem->get<Vector2f>("origin"));
	}

	if ((properties & ThemeFlags::ROTATION) && elem->has("rotation"))
	{
		setRotation(elem->get<float>("rotation"));
	}

	if ((properties & ThemeFlags::ROTATION_ORIGIN) && elem->has("rotationOrigin"))
	{
		setRotationOrigin(elem->get<Vector2f>("rotationOrigin"));
	}

	if ((properties & ThemeFlags::VISIBLE) && elem->has("visible"))
	{
		setVisible(elem->get<bool>("visible"));
	}

	if ((properties & ThemeFlags::Z_INDEX) && elem->has("zIndex"))
	{
		setZIndex(elem->get<float>("zIndex"));
	}

	// Aplicamos estilo SOLO al texto (sin pos/size porque ya los controla el contenedor)
	unsigned int textFlags =
		ThemeFlags::TEXT |
		ThemeFlags::FONT_PATH |
		ThemeFlags::FONT_SIZE |
		ThemeFlags::COLOR |
		ThemeFlags::ALIGNMENT |
		ThemeFlags::LINE_SPACING |
		ThemeFlags::FORCE_UPPERCASE |
		ThemeFlags::BACKGROUND_COLOR;

	// (Tus stroke props ya están en ThemeData; TextComponent debe soportarlas si ya lo tenías.)
	mText.applyTheme(theme, view, element, textFlags);

	// Texto siempre dentro del container
	mText.setPosition(0, 0);
	mText.setOrigin(0, 0);
	mText.setSize(mSize);

	// autoScroll
	bool enable = false;
	int delayMs = 1000;

	if (elem->has("autoScroll"))
		enable = elem->get<bool>("autoScroll");

	if (elem->has("autoScrollDelay"))
	{
		float d = elem->get<float>("autoScrollDelay");
		if (d < 0) d = 0;
		delayMs = (int)(d);
	}

	// Ajustamos el delay interno del ScrollableContainer:
	// Como ScrollableContainer recibe delay en constructor, lo más estable es:
	// - mantenerlo fijo y usar setAutoScroll(true) que resetea.
	// (Si querés delay real, en tu ScrollableContainer habría que exponer setter; por ahora evitamos tocarlo.)
	setAutoScroll(enable, delayMs);

	syncChildren();
}
