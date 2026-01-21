#include "components/ScrollableTextComponent.h"
#include "ThemeData.h"
#include "renderers/Renderer.h"

ScrollableTextComponent::ScrollableTextComponent(Window* window)
	: GuiComponent(window),
	  mScroll(window, 0),
	  mText(window),
	  mAutoScroll(false),
	  mDelayMs(1200)
{
	addChild(&mScroll);
	mScroll.addChild(&mText);

	// Texto interno pegado al contenedor
	mText.setOrigin(0.0f, 0.0f);
	mText.setPosition(0.0f, 0.0f, 0.0f);

	mScroll.setOrigin(0.0f, 0.0f);
	mScroll.setPosition(0.0f, 0.0f, 0.0f);
}

void ScrollableTextComponent::setAutoScroll(bool enabled)
{
	mAutoScroll = enabled;
	mScroll.setAutoScroll(mAutoScroll, mDelayMs);
}

void ScrollableTextComponent::setAutoScrollDelay(float secondsOrMs)
{
	if (secondsOrMs < 0.0f)
		secondsOrMs = 0.0f;

	// Si llega algo grande (1200), interpretamos como ms.
	// Si llega algo chico (1.2), interpretamos como segundos.
	if (secondsOrMs > 50.0f)
		mDelayMs = (int)secondsOrMs;
	else
		mDelayMs = (int)(secondsOrMs * 1000.0f);

	mScroll.setAutoScroll(mAutoScroll, mDelayMs);
}

std::string ScrollableTextComponent::getValue() const
{
	return mText.getValue();
}

void ScrollableTextComponent::setValue(const std::string& value)
{
	mText.setValue(value);
	syncLayout();
}

void ScrollableTextComponent::onSizeChanged()
{
	syncLayout();
}

void ScrollableTextComponent::syncLayout()
{
	// Ventana visible (clip)
	mScroll.setSize(mSize);

	// Wrap: ancho fijo, alto 0 => alto automático
	if (mSize.x() > 0.0f)
		mText.setSize(mSize.x(), 0.0f);
	else
		mText.setSize(0.0f, 0.0f);

	mScroll.reset();
	mScroll.setAutoScroll(mAutoScroll, mDelayMs);
}

void ScrollableTextComponent::update(int deltaTime)
{
	GuiComponent::update(deltaTime);
}

void ScrollableTextComponent::render(const Transform4x4f& parentTrans)
{
	GuiComponent::render(parentTrans);
}

void ScrollableTextComponent::applyTheme(const std::shared_ptr<ThemeData>& theme,
                                        const std::string& view,
                                        const std::string& element,
                                        unsigned int properties)
{
	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "text");
	if (!elem)
	{
		mText.applyTheme(theme, view, element, properties);
		syncLayout();
		return;
	}

	// Escala de pantalla (px)
	const float sw = (float)Renderer::getScreenWidth();
	const float sh = (float)Renderer::getScreenHeight();

	// TRANSFORM DEL WRAPPER (no del TextComponent interno)
	if (elem->has("pos"))
	{
		Vector2f p = elem->get<Vector2f>("pos");
		setPosition(p.x() * sw, p.y() * sh, 0.0f);
	}

	if (elem->has("size"))
	{
		Vector2f s = elem->get<Vector2f>("size");
		setSize(s.x() * sw, s.y() * sh);
	}

	if (elem->has("origin"))
	{
		Vector2f o = elem->get<Vector2f>("origin");
		setOrigin(o);
	}

	if (elem->has("rotation"))
		setRotationDegrees(elem->get<float>("rotation"));

	if (elem->has("rotationOrigin"))
		setRotationOrigin(elem->get<Vector2f>("rotationOrigin"));

	if (elem->has("zIndex"))
		setZIndex(elem->get<float>("zIndex"));

	if (elem->has("visible"))
		setVisible(elem->get<bool>("visible"));

	// Estilo tipográfico al TextComponent interno
	mText.applyTheme(theme, view, element, properties);

	// Texto interno siempre pegado arriba-izq dentro del contenedor
	mText.setOrigin(0.0f, 0.0f);
	mText.setPosition(0.0f, 0.0f, 0.0f);

	syncLayout();
}
