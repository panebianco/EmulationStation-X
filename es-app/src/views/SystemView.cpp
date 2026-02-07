// es-app/src/views/SystemView.cpp
#include "views/SystemView.h"

#include "animations/LambdaAnimation.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiInfoTextViewer.h"
#include "views/UIModeController.h"
#include "views/ViewController.h"
#include "Log.h"
#include "Scripting.h"
#include "Settings.h"
#include "SystemData.h"
#include "Window.h"
#include "LocaleES.h"
#include "Sound.h"
#include "NavigationSounds.h"

#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "renderers/Renderer.h"
#include "resources/ResourceManager.h"
#include "utils/StringUtil.h"
#include "math/Misc.h"

#include <algorithm>
#include <sstream>
#include <cmath>

// buffer values for scrolling velocity (left, stopped, right)
const int logoBuffersLeft[]  = { -5, -2, -1 };
const int logoBuffersRight[] = {  1,  2,  5 };

// ─────────────────────────────────────────────
// Helpers: descripción resuelta desde ThemeData (system-description)
// ─────────────────────────────────────────────
static inline bool looksLikeUnresolvedPlaceholder(const std::string& s)
{
	// Si todavía hay ${...} es que faltó variable y no queremos abrir visor con eso
	return (s.find("${") != std::string::npos);
}

static inline std::string getResolvedThemeText(const std::shared_ptr<ThemeData>& theme,
                                               const std::string& view,
                                               const std::string& key)
{
	if (!theme) return "";

	const ThemeData::ThemeElement* elem = theme->getElement(view, key, "text");
	if (!elem) return "";

	if (!elem->has("text")) return "";

	std::string out = elem->get<std::string>("text");
	out = Utils::String::trim(out);

	if (out.empty()) return "";
	if (looksLikeUnresolvedPlaceholder(out)) return "";

	return out;
}

// Crea un componente desde el tema para un key (image/text).
// Devuelve nullptr si no existe o si el tipo no es image/text.
static inline GuiComponent* makeThemedGuiComponent(Window* window,
                                                   const std::shared_ptr<ThemeData>& theme,
                                                   const std::string& view,
                                                   const std::string& key)
{
	if (!theme) return nullptr;

	const ThemeData::ThemeElement* elem = theme->getElement(view, key, "");
	if (!elem) return nullptr;

	GuiComponent* comp = nullptr;

	if (elem->type == "image")
	{
		comp = new ImageComponent(window);
	}
	else if (elem->type == "text")
	{
		comp = new TextComponent(window);
	}
	else
	{
		return nullptr;
	}

	comp->setDefaultZIndex(10);
	comp->applyTheme(theme, view, key, ThemeFlags::ALL);
	return comp;
}

SystemView::SystemView(Window* window) :
	IList<SystemViewData, SystemData*>(window, LIST_SCROLL_STYLE_SLOW, LIST_ALWAYS_LOOP),
	mViewNeedsReload(true),
	mSystemInfo(window, "SYSTEM INFO", Font::get(FONT_SIZE_SMALL), 0x33333300, ALIGN_CENTER),
	mInfoFocus(false)
{
	auto& loc = LocaleES::getInstance();
	loc.loadFromSettings();

	mSystemInfo.setText(loc.translate("SYSTEM INFO"));

	mCamOffset = 0;
	mExtrasCamOffset = 0;
	mExtrasFadeOpacity = 0.0f;
	mScrollSnd.reset();

	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
	populate();
}

void SystemView::populate()
{
	mEntries.clear();

	for(auto it = SystemData::sSystemVector.cbegin(); it != SystemData::sSystemVector.cend(); it++)
	{
		const std::shared_ptr<ThemeData>& theme = (*it)->getTheme();

		if(mViewNeedsReload)
			getViewElements(theme);

		if((*it)->isVisible())
		{
			Entry e;
			e.name   = (*it)->getName();
			e.object = *it;

			// init data
			e.data.infoButton = nullptr;
			e.data.infoButtonSelected = nullptr;
			e.data.hasDescription = false;
			e.data.descriptionText.clear();

			// make logo
			const ThemeData::ThemeElement* logoElem = theme->getElement("system", "logo", "image");
			if(logoElem)
			{
				std::string path        = logoElem->get<std::string>("path");
				std::string defaultPath = logoElem->has("default") ? logoElem->get<std::string>("default") : "";

				if((!path.empty() && ResourceManager::getInstance()->fileExists(path)) ||
				   (!defaultPath.empty() && ResourceManager::getInstance()->fileExists(defaultPath)))
				{
					ImageComponent* logo = new ImageComponent(mWindow, false, false);
					logo->setMaxSize(mCarousel.logoSize * mCarousel.logoScale);
					logo->applyTheme(theme, "system", "logo", ThemeFlags::PATH | ThemeFlags::COLOR);
					logo->setRotateByTargetSize(true);
					e.data.logo = std::shared_ptr<GuiComponent>(logo);
				}
			}

			if(!e.data.logo)
			{
				// no logo in theme; use text
				TextComponent* text = new TextComponent(
					mWindow,
					(*it)->getName(),
					Font::get(FONT_SIZE_LARGE),
					0x000000FF,
					ALIGN_CENTER);

				text->setSize(mCarousel.logoSize * mCarousel.logoScale);
				text->applyTheme(
					(*it)->getTheme(),
					"system",
					"logoText",
					ThemeFlags::FONT_PATH
						| ThemeFlags::FONT_SIZE
						| ThemeFlags::COLOR
						| ThemeFlags::FORCE_UPPERCASE
						| ThemeFlags::LINE_SPACING
						| ThemeFlags::TEXT);

				e.data.logo = std::shared_ptr<GuiComponent>(text);

				if(mCarousel.type == VERTICAL || mCarousel.type == VERTICAL_WHEEL)
				{
					text->setHorizontalAlignment(mCarousel.logoAlignment);
					text->setVerticalAlignment(ALIGN_CENTER);
				}
				else
				{
					text->setHorizontalAlignment(ALIGN_CENTER);
					text->setVerticalAlignment(mCarousel.logoAlignment);
				}
			}

			if(mCarousel.type == VERTICAL || mCarousel.type == VERTICAL_WHEEL)
			{
				if(mCarousel.logoAlignment == ALIGN_LEFT)
					e.data.logo->setOrigin(0, 0.5);
				else if(mCarousel.logoAlignment == ALIGN_RIGHT)
					e.data.logo->setOrigin(1.0, 0.5);
				else
					e.data.logo->setOrigin(0.5, 0.5);
			}
			else
			{
				if(mCarousel.logoAlignment == ALIGN_TOP)
					e.data.logo->setOrigin(0.5, 0);
				else if(mCarousel.logoAlignment == ALIGN_BOTTOM)
					e.data.logo->setOrigin(0.5, 1);
				else
					e.data.logo->setOrigin(0.5, 0.5);
			}

			Vector2f denormalized = mCarousel.logoSize * e.data.logo->getOrigin();
			e.data.logo->setPosition(denormalized.x(), denormalized.y(), 0.0);

			// delete any existing extras
			for(auto extra : e.data.backgroundExtras)
				delete extra;
			e.data.backgroundExtras.clear();

			// make background extras
			e.data.backgroundExtras = ThemeData::makeExtras((*it)->getTheme(), "system", mWindow);

			// ─────────────────────────────────────────────
			// Descripción: SOLO como dato desde theme key "system-description"
			// ─────────────────────────────────────────────
			e.data.descriptionText = getResolvedThemeText(theme, "system", "system-description");
			e.data.hasDescription  = !e.data.descriptionText.empty();

			// ─────────────────────────────────────────────
			// infoButton (idle/focused) totalmente temable:
			// - infoButton
			// - infoButtonSelected
			// Sólo se muestran si hay descripción válida
			// ─────────────────────────────────────────────
			if (e.data.hasDescription)
			{
				GuiComponent* idle = makeThemedGuiComponent(mWindow, theme, "system", "infoButton");
				GuiComponent* sel  = makeThemedGuiComponent(mWindow, theme, "system", "infoButtonSelected");

				// Si solo existe uno, igual funciona (sin crash)
				if (idle)
				{
					e.data.infoButton = idle;
					e.data.backgroundExtras.push_back(idle);
				}
				if (sel)
				{
					e.data.infoButtonSelected = sel;
					e.data.infoButtonSelected->setVisible(false); // empieza apagado
					e.data.backgroundExtras.push_back(sel);
				}
			}

			// sort the extras by z-index
			std::stable_sort(
				e.data.backgroundExtras.begin(),
				e.data.backgroundExtras.end(),
				[](GuiComponent* a, GuiComponent* b)
				{
					return b->getZIndex() > a->getZIndex();
				});

			this->add(e);
		}
	}

	if(mEntries.size() == 0)
	{
		// Something is wrong, there is not a single system to show, check if UI mode is not full
		if(!UIModeController::getInstance()->isUIModeFull())
		{
			Settings::getInstance()->setString("UIMode", "Full");
			mWindow->pushGui(new GuiMsgBox(
				mWindow,
				"The selected UI mode has nothing to show,\n returning to UI mode: FULL",
				"OK",
				nullptr));
		}
	}

	// Reset foco al reconstruir
	mInfoFocus = false;
}

void SystemView::onShow()
{
	mShowing = true;
}

void SystemView::onHide()
{
	mShowing = false;
}

void SystemView::goToSystem(SystemData* system, bool animate)
{
	setCursor(system);

	if(!animate)
		finishAnimation(0);
}

// helper local para reproducir scroll de sistema
inline void playSystemScrollSound(SystemData* sys,
                                  const std::shared_ptr<Sound>& scrollFromCarousel)
{
	// 1) prioridad: sonido definido en <carousel scrollSound="...">
	if (scrollFromCarousel)
	{
		scrollFromCarousel->play();
		return;
	}

	if (!sys)
		return;

	const std::shared_ptr<ThemeData>& theme = sys->getTheme();
	if (!theme)
		return;

	// 2) Intentar esquema tipo Batocera: "systembrowse" → scroll de carrusel
	auto snd = NavigationSounds::getFromTheme(theme, "systembrowse");
	if (!snd)
	{
		// 3) fallback genérico "scroll"
		snd = NavigationSounds::getFromTheme(theme, "scroll");
	}

	if (snd)
		snd->play();
}

// ─────────────────────────────────────────────
// InfoButton focus helpers
// ─────────────────────────────────────────────
bool SystemView::selectedHasInfo() const
{
	if (mEntries.empty()) return false;
	const SystemViewData& d = mEntries.at(mCursor).data;

	// Debe existir descripción válida y al menos un botón temado
	if (!d.hasDescription) return false;
	return (d.infoButton != nullptr || d.infoButtonSelected != nullptr);
}

void SystemView::setInfoFocus(bool focus)
{
	mInfoFocus = focus;

	if (mEntries.empty()) return;
	SystemViewData& d = mEntries.at(mCursor).data;

	// Si no hay botones, nada que alternar
	if (!d.infoButton && !d.infoButtonSelected)
		return;

	// Si falta uno de los dos, el otro queda visible según convenga
	if (d.infoButton)
		d.infoButton->setVisible(!mInfoFocus);

	if (d.infoButtonSelected)
		d.infoButtonSelected->setVisible(mInfoFocus);
}

bool SystemView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->getDeviceId() == DEVICE_KEYBOARD && input.value &&
		   input.id == SDLK_r && SDL_GetModState() & KMOD_LCTRL &&
		   Settings::getInstance()->getBool("Debug"))
		{
			LOG(LogInfo) << " Reloading all";
			ViewController::get()->reloadAll();
			return true;
		}

		// ─────────────────────────────────────────────
		// Navegación por foco:
		// - Carrusel horizontal: ↓ entra al botón, ↑ vuelve.
		// - Carrusel vertical:   → entra al botón, ← vuelve.
		// Además: en foco INFO:
		//   - A = abrir visor (mismo botón que entrar al sistema)
		//   - B/Back = salir del foco (mismo botón que salir/volver)
		// ─────────────────────────────────────────────
		const bool carouselIsVertical = (mCarousel.type == VERTICAL || mCarousel.type == VERTICAL_WHEEL);

		if (!mInfoFocus)
		{
			// entrar al botón
			if (selectedHasInfo())
			{
				if (!carouselIsVertical && config->isMappedLike("down", input))
				{
					setInfoFocus(true);
					return true;
				}
				if (carouselIsVertical && config->isMappedLike("right", input))
				{
					setInfoFocus(true);
					return true;
				}
			}
		}
		else
		{
			// (1) salir del foco con el mismo botón de volver/salir
			// En ES normalmente es "b". Si tu build usa "back", lo aceptamos también.
			if (config->isMappedTo("b", input) || config->isMappedTo("back", input))
			{
				setInfoFocus(false);
				return true;
			}

			// (2) volver al carrusel por dirección (además del botón back)
			if (!carouselIsVertical && config->isMappedLike("up", input))
			{
				setInfoFocus(false);
				return true;
			}
			if (carouselIsVertical && config->isMappedLike("left", input))
			{
				setInfoFocus(false);
				return true;
			}

			// (3) A abre visor (mismo botón que entrar al sistema)
			if (config->isMappedTo("a", input))
			{
				if (!mEntries.empty())
				{
					const SystemViewData& d = mEntries.at(mCursor).data;
					if (d.hasDescription && !d.descriptionText.empty())
					{
						std::string title = getSelected() ? getSelected()->getName() : "INFO";
						mWindow->pushGui(new GuiInfoTextViewer(mWindow, title, d.descriptionText));
					}
				}
				return true;
			}

			// (4) Mientras estamos en el botón, no navegar sistemas con d-pad
			if (config->isMappedLike("left", input) ||
				config->isMappedLike("right", input) ||
				config->isMappedLike("up", input) ||
				config->isMappedLike("down", input))
			{
				return true;
			}

			// (5) Evitar acciones globales (ej. random en X) mientras el foco está en INFO
			if (config->isMappedTo("x", input))
				return true;

			return true;
		}

		bool moved = false;

		switch(mCarousel.type)
		{
		case VERTICAL:
		case VERTICAL_WHEEL:
			if(config->isMappedLike("up", input))
			{
				moved = listInput(-1);
				if (moved)
					playSystemScrollSound(getSelected(), mScrollSnd);
				return true;
			}
			if(config->isMappedLike("down", input))
			{
				moved = listInput(1);
				if (moved)
					playSystemScrollSound(getSelected(), mScrollSnd);
				return true;
			}
			break;

		case HORIZONTAL:
		case HORIZONTAL_WHEEL:
		default:
			if(config->isMappedLike("left", input))
			{
				moved = listInput(-1);
				if (moved)
					playSystemScrollSound(getSelected(), mScrollSnd);
				return true;
			}
			if(config->isMappedLike("right", input))
			{
				moved = listInput(1);
				if (moved)
					playSystemScrollSound(getSelected(), mScrollSnd);
				return true;
			}
			break;
		}

		if(config->isMappedTo("a", input))
		{
			// Si el foco está en INFO, este A NO entra al sistema
			if (mInfoFocus)
				return true;

			std::shared_ptr<Sound> selectSnd;

			SystemData* sys = getSelected();
			if(sys != nullptr)
			{
				const std::shared_ptr<ThemeData>& theme = sys->getTheme();
				if (theme)
				{
					selectSnd = NavigationSounds::getFromTheme(theme, "select");

					if (!selectSnd && theme->hasView("system"))
					{
						const ThemeData::ThemeElement* selectElem =
							theme->getElement("system", "selectSound", "sound");

						if(selectElem && selectElem->has("path"))
						{
							std::string path = selectElem->get<std::string>("path");
							if(!path.empty())
								selectSnd = Sound::get(path);
						}
					}
				}
			}

			if(selectSnd)
				selectSnd->play();

			stopScrolling();
			setInfoFocus(false);
			ViewController::get()->goToGameList(getSelected());
			return true;
		}

		// X: en carrusel sigue funcionando como antes (random)
		if(config->isMappedTo("x", input))
		{
			setInfoFocus(false);
			setCursor(SystemData::getRandomSystem());
			return true;
		}
	}
	else
	{
		if(config->isMappedLike("left", input) ||
		   config->isMappedLike("right", input) ||
		   config->isMappedLike("up", input) ||
		   config->isMappedLike("down", input))
			listInput(0);

		Scripting::fireEvent("system-select", this->IList::getSelected()->getName(), "input");

		if(!UIModeController::getInstance()->isUIModeKid() &&
		   config->isMappedTo("select", input) &&
		   Settings::getInstance()->getBool("ScreenSaverControls"))
		{
			mWindow->startScreenSaver();
			mWindow->renderScreenSaver();
			return true;
		}
	}

	return GuiComponent::input(config, input);
}

void SystemView::update(int deltaTime)
{
	listUpdate(deltaTime);
	GuiComponent::update(deltaTime);
}

void SystemView::onCursorChanged(const CursorState& /*state*/)
{
	updateHelpPrompts();

	// Al cambiar sistema, volver el foco al carrusel y refrescar visibilidad del botón
	setInfoFocus(false);

	float startPos = mCamOffset;

	float posMax = (float)mEntries.size();
	float target = (float)mCursor;

	float endPos = target;
	float dist   = abs(endPos - startPos);

	if(abs(target + posMax - startPos) < dist)
		endPos = target + posMax;
	if(abs(target - posMax - startPos) < dist)
		endPos = target - posMax;

	cancelAnimation(1);
	cancelAnimation(2);

	std::string transition_style = Settings::getInstance()->getString("TransitionStyle");
	bool goFast                   = transition_style == "instant";
	const float infoStartOpacity  = mSystemInfo.getOpacity() / 255.f;

	Animation* infoFadeOut = new LambdaAnimation(
		[infoStartOpacity, this](float t)
		{
			mSystemInfo.setOpacity((unsigned char)(Math::lerp(infoStartOpacity, 0.f, t) * 255));
		},
		(int)(infoStartOpacity * (goFast ? 10 : 150)));

	unsigned int gameCount = getSelected()->getDisplayedGameCount();
	LocaleES& loc = LocaleES::getInstance();

	setAnimation(
		infoFadeOut,
		0,
		[this, gameCount, &loc]()
		{
			std::stringstream ss;

			if(!getSelected()->isGameSystem())
			{
				ss << loc.translate("CONFIGURATION");
			}
			else
			{
				ss << gameCount << " "
				   << loc.translate(gameCount == 1 ? "GAME" : "GAMES")
				   << " "
				   << loc.translate("AVAILABLE");
			}

			mSystemInfo.setText(ss.str());
		},
		false,
		1);

	Animation* infoFadeIn = new LambdaAnimation(
		[this](float t)
		{
			mSystemInfo.setOpacity((unsigned char)(Math::lerp(0.f, 1.f, t) * 255));
		},
		goFast ? 10 : 300);

	setAnimation(infoFadeIn, goFast ? 0 : 2000, nullptr, false, 2);

	if(endPos == mCamOffset && endPos == mExtrasCamOffset)
		return;

	Animation* anim;
	bool move_carousel = Settings::getInstance()->getBool("MoveCarousel");

	if(transition_style == "fade")
	{
		float startExtrasFade = mExtrasFadeOpacity;
		anim = new LambdaAnimation(
			[this, startExtrasFade, startPos, endPos, posMax, move_carousel](float t)
			{
				t -= 1;
				float f = Math::lerp(startPos, endPos, t * t * t + 1);
				if(f < 0)
					f += posMax;
				if(f >= posMax)
					f -= posMax;

				this->mCamOffset = move_carousel ? f : endPos;

				t += 1;
				if(t < 0.3f)
					this->mExtrasFadeOpacity = Math::lerp(0.0f, 1.0f, t / 0.3f + startExtrasFade);
				else if(t < 0.7f)
					this->mExtrasFadeOpacity = 1.0f;
				else
					this->mExtrasFadeOpacity = Math::lerp(1.0f, 0.0f, (t - 0.7f) / 0.3f);

				if(t > 0.5f)
					this->mExtrasCamOffset = endPos;
			},
			500);
	}
	else if(transition_style == "slide")
	{
		anim = new LambdaAnimation(
			[this, startPos, endPos, posMax, move_carousel](float t)
			{
				t -= 1;
				float f = Math::lerp(startPos, endPos, t * t * t + 1);
				if(f < 0)
					f += posMax;
				if(f >= posMax)
					f -= posMax;

				this->mCamOffset       = move_carousel ? f : endPos;
				this->mExtrasCamOffset = f;
			},
			500);
	}
	else
	{
		anim = new LambdaAnimation(
			[this, startPos, endPos, posMax, move_carousel](float t)
			{
				t -= 1;
				float f = Math::lerp(startPos, endPos, t * t * t + 1);
				if(f < 0)
					f += posMax;
				if(f >= posMax)
					f -= posMax;

				this->mCamOffset       = move_carousel ? f : endPos;
				this->mExtrasCamOffset = endPos;
			},
			move_carousel ? 500 : 1);
	}

	setAnimation(anim, 0, nullptr, false, 0);
}

void SystemView::render(const Transform4x4f& parentTrans)
{
	if(size() == 0)
		return;

	Transform4x4f trans = getTransform() * parentTrans;

	auto systemInfoZIndex = mSystemInfo.getZIndex();
	auto minMax           = std::minmax(mCarousel.zIndex, systemInfoZIndex);

	renderExtras(trans, INT16_MIN, minMax.first);
	renderFade(trans);

	if(mCarousel.zIndex > mSystemInfo.getZIndex())
		renderInfoBar(trans);
	else
		renderCarousel(trans);

	renderExtras(trans, minMax.first, minMax.second);

	if(mCarousel.zIndex > mSystemInfo.getZIndex())
		renderCarousel(trans);
	else
		renderInfoBar(trans);

	renderExtras(trans, minMax.second, INT16_MAX);
}

std::vector<HelpPrompt> SystemView::getHelpPrompts()
{
	LocaleES& loc = LocaleES::getInstance();

	std::vector<HelpPrompt> prompts;

	const bool carouselIsVertical = (mCarousel.type == VERTICAL || mCarousel.type == VERTICAL_WHEEL);

	if (!mInfoFocus)
	{
		if (carouselIsVertical)
			prompts.push_back(HelpPrompt("up/down", loc.translate("CHOOSE")));
		else
			prompts.push_back(HelpPrompt("left/right", loc.translate("CHOOSE")));

		prompts.push_back(HelpPrompt("a", loc.translate("SELECT")));
		prompts.push_back(HelpPrompt("x", loc.translate("RANDOM")));

		// Si existe infoButton y hay descripción, sugerir entrada al botón
		if (selectedHasInfo())
		{
			if (carouselIsVertical)
				prompts.push_back(HelpPrompt("right", "INFO"));
			else
				prompts.push_back(HelpPrompt("down", "INFO"));
		}
	}
	else
	{
		// En foco de botón: A abre visor (mismo botón de entrar)
		prompts.push_back(HelpPrompt("a", "INFO"));

		// Salir con el mismo botón de volver/salir
		prompts.push_back(HelpPrompt("b", loc.translate("BACK")));

		// Además, volver al carrusel por dirección
		if (carouselIsVertical)
			prompts.push_back(HelpPrompt("left", loc.translate("BACK")));
		else
			prompts.push_back(HelpPrompt("up", loc.translate("BACK")));
	}

	if(!UIModeController::getInstance()->isUIModeKid() &&
	   Settings::getInstance()->getBool("ScreenSaverControls"))
	{
		prompts.push_back(HelpPrompt("select", loc.translate("LAUNCH SCREENSAVER")));
	}

	return prompts;
}

HelpStyle SystemView::getHelpStyle()
{
	HelpStyle style;
	style.applyTheme(mEntries.at(mCursor).object->getTheme(), "system");
	return style;
}

void SystemView::onThemeChanged(const std::shared_ptr<ThemeData>& /*theme*/)
{
	LOG(LogDebug) << "SystemView::onThemeChanged()";
	mViewNeedsReload = true;
	populate();
}

void SystemView::getViewElements(const std::shared_ptr<ThemeData>& theme)
{
	LOG(LogDebug) << "SystemView::getViewElements()";

	getDefaultElements();

	if(!theme->hasView("system"))
		return;

	const ThemeData::ThemeElement* carouselElem = theme->getElement("system", "systemcarousel", "carousel");
	if(carouselElem)
		getCarouselFromTheme(carouselElem);

	const ThemeData::ThemeElement* sysInfoElem = theme->getElement("system", "systemInfo", "text");
	if(sysInfoElem)
		mSystemInfo.applyTheme(theme, "system", "systemInfo", ThemeFlags::ALL);

	mViewNeedsReload = false;
}

void SystemView::renderCarousel(const Transform4x4f& trans)
{
	if(mEntries.empty())
		return;

	Transform4x4f carouselTrans = trans;
	carouselTrans.translate(Vector3f(mCarousel.pos.x(), mCarousel.pos.y(), 0.0));
	carouselTrans.translate(Vector3f(
		mCarousel.origin.x() * mCarousel.size.x() * -1,
		mCarousel.origin.y() * mCarousel.size.y() * -1,
		0.0f));

	Vector2f clipPos(carouselTrans.translation().x(), carouselTrans.translation().y());
	Renderer::pushClipRect(
		Vector2i((int)clipPos.x(), (int)clipPos.y()),
		Vector2i((int)mCarousel.size.x(), (int)mCarousel.size.y()));

	Renderer::setMatrix(carouselTrans);

	Renderer::drawRect(
		0.0f,
		0.0f,
		mCarousel.size.x(),
		mCarousel.size.y(),
		mCarousel.color,
		mCarousel.colorEnd,
		mCarousel.colorGradientHorizontal);

	// draw logos
	Vector2f logoSpacing(0.0f, 0.0f);
	float xOff = 0.0f;
	float yOff = 0.0f;

	switch(mCarousel.type)
	{
	case VERTICAL_WHEEL:
		logoSpacing[1] = mCarousel.logoSize.y();
		yOff = (mCarousel.size.y() - mCarousel.logoSize.y()) / 2.f - (mCamOffset * logoSpacing[1]);

		if(mCarousel.logoAlignment == ALIGN_LEFT)
			xOff = mCarousel.logoSize.x() / 10.f;
		else if(mCarousel.logoAlignment == ALIGN_RIGHT)
			xOff = mCarousel.size.x() - (mCarousel.logoSize.x() * 1.1f);
		else
			xOff = (mCarousel.size.x() - mCarousel.logoSize.x()) / 2.f;
		break;

	case VERTICAL:
		logoSpacing[1] = ((mCarousel.size.y() -
		                   (mCarousel.logoSize.y() * mCarousel.maxLogoCount)) /
		                  (mCarousel.maxLogoCount))
		               + mCarousel.logoSize.y();
		yOff           = (mCarousel.size.y() - mCarousel.logoSize.y()) / 2.f
		               - (mCamOffset * logoSpacing[1]);

		if(mCarousel.logoAlignment == ALIGN_LEFT)
			xOff = mCarousel.logoSize.x() / 10.f;
		else if(mCarousel.logoAlignment == ALIGN_RIGHT)
			xOff = mCarousel.size.x() - (mCarousel.logoSize.x() * 1.1f);
		else
			xOff = (mCarousel.size.x() - mCarousel.logoSize.x()) / 2.f;
		break;

	case HORIZONTAL_WHEEL:
		logoSpacing[0] = mCarousel.logoSize.x();
		xOff = (mCarousel.size.x() - mCarousel.logoSize.x()) / 2.f - (mCamOffset * logoSpacing[0]);

		if(mCarousel.logoAlignment == ALIGN_TOP)
			yOff = mCarousel.logoSize.y() / 10.f;
		else if(mCarousel.logoAlignment == ALIGN_BOTTOM)
			yOff = mCarousel.size.y() - (mCarousel.logoSize.y() * 1.1f);
		else
			yOff = (mCarousel.size.y() - mCarousel.logoSize.y()) / 2.f;
		break;

	case HORIZONTAL:
	default:
		logoSpacing[0] = ((mCarousel.size.x() -
		                   (mCarousel.logoSize.x() * mCarousel.maxLogoCount)) /
		                  (mCarousel.maxLogoCount))
		               + mCarousel.logoSize.x();
		xOff           = (mCarousel.size.x() - mCarousel.logoSize.x()) / 2.f
		               - (mCamOffset * logoSpacing[0]);

		if(mCarousel.logoAlignment == ALIGN_TOP)
			yOff = mCarousel.logoSize.y() / 10.f;
		else if(mCarousel.logoAlignment == ALIGN_BOTTOM)
			yOff = mCarousel.size.y() - (mCarousel.logoSize.y() * 1.1f);
		else
			yOff = (mCarousel.size.y() - mCarousel.logoSize.y()) / 2.f;
		break;
	}

	int center    = (int)(mCamOffset);
	int logoCount = Math::min(mCarousel.maxLogoCount, (int)mEntries.size());

	int bufferIndex = getScrollingVelocity() + 1;
	if(bufferIndex < 0) bufferIndex = 0;
	if(bufferIndex > 2) bufferIndex = 2;

	int bufferLeft  = logoBuffersLeft[bufferIndex];
	int bufferRight = logoBuffersRight[bufferIndex];

	if(logoCount == 1)
	{
		bufferLeft  = 0;
		bufferRight = 0;
	}

	auto renderLogo = [this, &carouselTrans, &logoSpacing, xOff, yOff](int i)
	{
		if(mEntries.empty())
			return;

		int index = i;
		while(index < 0)
			index += (int)mEntries.size();
		while(index >= (int)mEntries.size())
			index -= (int)mEntries.size();

		Transform4x4f logoTrans = carouselTrans;

		if(mCarousel.type == HORIZONTAL &&
		   mCarousel.logoScale != 1.0f &&
		   mCarousel.scaledLogoSpacing != 0.0f)
		{
			float logoDiffX = ((logoSpacing[0] * mCarousel.logoScale) - logoSpacing[0])
				/ 2.0f * mCarousel.scaledLogoSpacing;

			if(index == mCursor)
			{
				logoTrans.translate(Vector3f(
					i * logoSpacing[0] + xOff,
					i * logoSpacing[1] + yOff,
					0.0f));
			}
			else if(i < mCursor || (mCursor == 0 && index > mCarousel.maxLogoCount))
			{
				logoTrans.translate(Vector3f(
					i * logoSpacing[0] + xOff - logoDiffX,
					i * logoSpacing[1] + yOff,
					0.0f));
			}
			else
			{
				logoTrans.translate(Vector3f(
					i * logoSpacing[0] + xOff + logoDiffX,
					i * logoSpacing[1] + yOff,
					0.0f));
			}
		}
		else
		{
			logoTrans.translate(Vector3f(
				i * logoSpacing[0] + xOff,
				i * logoSpacing[1] + yOff,
				0.0f));
		}

		float distance = i - mCamOffset;

		float scale = 1.0f + ((mCarousel.logoScale - 1.0f) * (1.0f - fabs(distance)));
		scale       = Math::min(mCarousel.logoScale, Math::max(1.0f, scale));
		scale      /= mCarousel.logoScale;

		float minOpacity = mCarousel.minLogoOpacity;
		if(minOpacity < 0.0f) minOpacity = 0.0f;
		if(minOpacity > 1.0f) minOpacity = 1.0f;

		int opref   = (int)Math::round(minOpacity * 255.0f);
		int opacity = (int)Math::round(opref + ((0xFF - opref) * (1.0f - fabs(distance))));
		if(opacity < opref)
			opacity = opref;

		const std::shared_ptr<GuiComponent>& comp = mEntries.at(index).data.logo;
		if(!comp)
			return;

		if(mCarousel.type == VERTICAL_WHEEL || mCarousel.type == HORIZONTAL_WHEEL)
		{
			comp->setRotationDegrees(mCarousel.logoRotation * distance);
			comp->setRotationOrigin(mCarousel.logoRotationOrigin);
		}

		comp->setScale(scale);
		comp->setOpacity((unsigned char)opacity);
		comp->render(logoTrans);
	};

	std::vector<int> activePositions;
	for(int i = center - logoCount / 2 + bufferLeft;
	    i <= center + logoCount / 2 + bufferRight;
	    i++)
	{
		int index = i;
		while(index < 0)
			index += (int)mEntries.size();
		while(index >= (int)mEntries.size())
			index -= (int)mEntries.size();

		if(index == mCursor)
			activePositions.push_back(i);
		else
			renderLogo(i);
	}

	for(auto activePos : activePositions)
		renderLogo(activePos);

	Renderer::popClipRect();
}

void SystemView::renderInfoBar(const Transform4x4f& trans)
{
	Renderer::setMatrix(trans);
	mSystemInfo.render(trans);
}

void SystemView::renderExtras(const Transform4x4f& trans, float lower, float upper)
{
	int extrasCenter = (int)mExtrasCamOffset;

	int bufferIndex = getScrollingVelocity() + 1;
	if(bufferIndex < 0) bufferIndex = 0;
	if(bufferIndex > 2) bufferIndex = 2;

	Renderer::pushClipRect(Vector2i::Zero(), Vector2i((int)mSize.x(), (int)mSize.y()));

	for(int i = extrasCenter + logoBuffersLeft[bufferIndex];
	    i <= extrasCenter + logoBuffersRight[bufferIndex];
	    i++)
	{
		int index = i;
		while(index < 0)
			index += (int)mEntries.size();
		while(index >= (int)mEntries.size())
			index -= (int)mEntries.size();

		if(mShowing || index == mCursor)
		{
			Transform4x4f extrasTrans = trans;
			if(mCarousel.type == HORIZONTAL || mCarousel.type == HORIZONTAL_WHEEL)
				extrasTrans.translate(Vector3f((i - mExtrasCamOffset) * mSize.x(), 0, 0));
			else
				extrasTrans.translate(Vector3f(0, (i - mExtrasCamOffset) * mSize.y(), 0));

			Renderer::pushClipRect(
				Vector2i((int)extrasTrans.translation()[0], (int)extrasTrans.translation()[1]),
				Vector2i((int)mSize.x(), (int)mSize.y()));

			SystemViewData data = mEntries.at(index).data;
			for(unsigned int j = 0; j < data.backgroundExtras.size(); j++)
			{
				GuiComponent* extra = data.backgroundExtras[j];
				if(extra->getZIndex() >= lower && extra->getZIndex() < upper)
					extra->render(extrasTrans);
			}
			Renderer::popClipRect();
		}
	}

	Renderer::popClipRect();
}

void SystemView::renderFade(const Transform4x4f& trans)
{
	if(mExtrasFadeOpacity)
	{
		unsigned int fadeColor = 0x00000000 | (unsigned char)(mExtrasFadeOpacity * 255);
		Renderer::setMatrix(trans);
		Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), fadeColor, fadeColor);
	}
}

void SystemView::getDefaultElements(void)
{
	mCarousel.type          = HORIZONTAL;
	mCarousel.logoAlignment = ALIGN_CENTER;
	mCarousel.size.x()      = mSize.x();
	mCarousel.size.y()      = 0.2325f * mSize.y();
	mCarousel.pos.x()       = 0.0f;
	mCarousel.pos.y()       = 0.5f * (mSize.y() - mCarousel.size.y());
	mCarousel.origin.x()    = 0.0f;
	mCarousel.origin.y()    = 0.0f;
	mCarousel.color         = 0xFFFFFFD8;
	mCarousel.colorEnd      = 0xFFFFFFD8;
	mCarousel.colorGradientHorizontal = true;
	mCarousel.logoScale     = 1.2f;
	mCarousel.logoRotation  = 7.5f;
	mCarousel.logoRotationOrigin.x() = -5.0f;
	mCarousel.logoRotationOrigin.y() = 0.5f;
	mCarousel.logoSize.x()  = 0.25f * mSize.x();
	mCarousel.logoSize.y()  = 0.155f * mSize.y();
	mCarousel.maxLogoCount  = 3;
	mCarousel.zIndex        = 40;

	mCarousel.minLogoOpacity    = 0.5f;
	mCarousel.scaledLogoSpacing = 0.0f;

	mScrollSnd.reset();

	mSystemInfo.setSize(mSize.x(), mSystemInfo.getFont()->getLetterHeight() * 2.2f);
	mSystemInfo.setPosition(0, (mCarousel.pos.y() + mCarousel.size.y() - 0.2f));
	mSystemInfo.setBackgroundColor(0xDDDDDDD8);
	mSystemInfo.setRenderBackground(true);
	mSystemInfo.setFont(Font::get((int)(0.035f * mSize.y()), Font::getDefaultPath()));
	mSystemInfo.setColor(0x000000FF);
	mSystemInfo.setZIndex(50);
	mSystemInfo.setDefaultZIndex(50);
}

void SystemView::getCarouselFromTheme(const ThemeData::ThemeElement* elem)
{
	if(elem->has("type"))
	{
		if(!(elem->get<std::string>("type").compare("vertical")))
			mCarousel.type = VERTICAL;
		else if(!(elem->get<std::string>("type").compare("vertical_wheel")))
			mCarousel.type = VERTICAL_WHEEL;
		else if(!(elem->get<std::string>("type").compare("horizontal_wheel")))
			mCarousel.type = HORIZONTAL_WHEEL;
		else
			mCarousel.type = HORIZONTAL;
	}

	if(elem->has("size"))
		mCarousel.size = elem->get<Vector2f>("size") * mSize;

	if(elem->has("pos"))
		mCarousel.pos = elem->get<Vector2f>("pos") * mSize;

	if(elem->has("origin"))
		mCarousel.origin = elem->get<Vector2f>("origin");

	if(elem->has("color"))
	{
		mCarousel.color    = elem->get<unsigned int>("color");
		mCarousel.colorEnd = mCarousel.color;
	}

	if(elem->has("colorEnd"))
		mCarousel.colorEnd = elem->get<unsigned int>("colorEnd");

	if(elem->has("gradientType"))
		mCarousel.colorGradientHorizontal = !(elem->get<std::string>("gradientType").compare("horizontal"));

	if(elem->has("logoScale"))
		mCarousel.logoScale = elem->get<float>("logoScale");

	if(elem->has("logoSize"))
		mCarousel.logoSize = elem->get<Vector2f>("logoSize") * mSize;

	if(elem->has("maxLogoCount"))
		mCarousel.maxLogoCount = (int)Math::round(elem->get<float>("maxLogoCount"));

	if(elem->has("zIndex"))
		mCarousel.zIndex = elem->get<float>("zIndex");

	if(elem->has("logoRotation"))
		mCarousel.logoRotation = elem->get<float>("logoRotation");

	if(elem->has("logoRotationOrigin"))
		mCarousel.logoRotationOrigin = elem->get<Vector2f>("logoRotationOrigin");

	if(elem->has("logoAlignment"))
	{
		if(!(elem->get<std::string>("logoAlignment").compare("left")))
			mCarousel.logoAlignment = ALIGN_LEFT;
		else if(!(elem->get<std::string>("logoAlignment").compare("right")))
			mCarousel.logoAlignment = ALIGN_RIGHT;
		else if(!(elem->get<std::string>("logoAlignment").compare("top")))
			mCarousel.logoAlignment = ALIGN_TOP;
		else if(!(elem->get<std::string>("logoAlignment").compare("bottom")))
			mCarousel.logoAlignment = ALIGN_BOTTOM;
		else
			mCarousel.logoAlignment = ALIGN_CENTER;
	}

	if(elem->has("minLogoOpacity"))
		mCarousel.minLogoOpacity = elem->get<float>("minLogoOpacity");

	if(elem->has("scaledLogoSpacing"))
		mCarousel.scaledLogoSpacing = elem->get<float>("scaledLogoSpacing");

	if(elem->has("scrollSound"))
	{
		std::string path = elem->get<std::string>("scrollSound");
		if(!path.empty())
			mScrollSnd = Sound::get(path);
	}
}
