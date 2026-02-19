#include "guis/GuiSlideshowScreensaverOptions.h"

#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "components/TextComponent.h"

#include "guis/GuiTextEditPopup.h"

#include "utils/StringUtil.h"
#include "Settings.h"
#include "Window.h"
#include "LocaleES.h"
#include "resources/Font.h"
#include "PowerSaver.h"

GuiSlideshowScreensaverOptions::GuiSlideshowScreensaverOptions(Window* window, const char* title)
	: GuiScreensaverOptions(window, title)
{
	// Helper local seguro
	auto T = [](const std::string& key) -> std::string {
		return LocaleES::getInstance().translate(key);
	};

	ComponentListRow row;

	// ------------------------------------------------------------
	// MEDIA DURATION
	// ------------------------------------------------------------
	auto sss_media_sec = std::make_shared<SliderComponent>(mWindow, 1.f, 60.f, 1.f, "s");
	sss_media_sec->setValue((float)(Settings::getInstance()->getInt("ScreenSaverSwapMediaTimeout") / 1000));

	addWithLabel(row, T("SWAP MEDIA AFTER (SECS)"), sss_media_sec);

	addSaveFunc([sss_media_sec] {
		int playNextTimeout = (int)Math::round(sss_media_sec->getValue()) * 1000;
		Settings::getInstance()->setInt("ScreenSaverSwapMediaTimeout", playNextTimeout);
		PowerSaver::updateTimeouts();
	});

	// ------------------------------------------------------------
	// STRETCH
	// ------------------------------------------------------------
	auto sss_stretch = std::make_shared<SwitchComponent>(mWindow);
	sss_stretch->setState(Settings::getInstance()->getBool("SlideshowScreenSaverStretch"));

	addWithLabel(row, T("STRETCH MEDIA"), sss_stretch);

	addSaveFunc([sss_stretch] {
		Settings::getInstance()->setBool("SlideshowScreenSaverStretch", sss_stretch->getState());
	});

	// ------------------------------------------------------------
	// BACKGROUND AUDIO
	// ------------------------------------------------------------
	auto sss_bg_audio_file = std::make_shared<TextComponent>(
		mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF
	);

	addEditableTextComponent(
		row,
		T("BACKGROUND AUDIO"),
		sss_bg_audio_file,
		Settings::getInstance()->getString("SlideshowScreenSaverBackgroundAudioFile")
	);

	addSaveFunc([sss_bg_audio_file] {
		Settings::getInstance()->setString(
			"SlideshowScreenSaverBackgroundAudioFile",
			sss_bg_audio_file->getValue()
		);
	});

	// ------------------------------------------------------------
	// USE CUSTOM MEDIA
	// ------------------------------------------------------------
	auto sss_custom_source = std::make_shared<SwitchComponent>(mWindow);
	sss_custom_source->setState(
		Settings::getInstance()->getBool("SlideshowScreenSaverCustomMediaSource")
	);

	addWithLabel(row, T("USE CUSTOM MEDIA"), sss_custom_source);

	addSaveFunc([sss_custom_source] {
		Settings::getInstance()->setBool(
			"SlideshowScreenSaverCustomMediaSource",
			sss_custom_source->getState()
		);
	});

	// ------------------------------------------------------------
	// CUSTOM MEDIA DIR
	// ------------------------------------------------------------
	auto sss_media_dir = std::make_shared<TextComponent>(
		mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF
	);

	addEditableTextComponent(
		row,
		T("CUSTOM MEDIA DIR"),
		sss_media_dir,
		Settings::getInstance()->getString("SlideshowScreenSaverMediaDir")
	);

	addSaveFunc([sss_media_dir] {
		Settings::getInstance()->setString(
			"SlideshowScreenSaverMediaDir",
			sss_media_dir->getValue()
		);
	});

	// ------------------------------------------------------------
	// RECURSIVE
	// ------------------------------------------------------------
	auto sss_recurse = std::make_shared<SwitchComponent>(mWindow);
	sss_recurse->setState(
		Settings::getInstance()->getBool("SlideshowScreenSaverRecurse")
	);

	addWithLabel(row, T("CUSTOM MEDIA DIR RECURSIVE"), sss_recurse);

	addSaveFunc([sss_recurse] {
		Settings::getInstance()->setBool(
			"SlideshowScreenSaverRecurse",
			sss_recurse->getState()
		);
	});

	// ------------------------------------------------------------
	// CUSTOM IMAGE FILTER
	// ------------------------------------------------------------
	auto sss_image_filter = std::make_shared<TextComponent>(
		mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF
	);

	addEditableTextComponent(
		row,
		T("CUSTOM IMAGE FILTER"),
		sss_image_filter,
		Settings::getInstance()->getString("SlideshowScreenSaverImageFilter")
	);

	addSaveFunc([sss_image_filter] {
		Settings::getInstance()->setString(
			"SlideshowScreenSaverImageFilter",
			sss_image_filter->getValue()
		);
	});

	// ------------------------------------------------------------
	// CUSTOM VIDEO FILTER
	// ------------------------------------------------------------
	auto sss_video_filter = std::make_shared<TextComponent>(
		mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF
	);

	sss_video_filter->setSize(
		Vector2f(0, Font::get(FONT_SIZE_SMALL)->getLetterHeight())
	);

	addEditableTextComponent(
		row,
		T("CUSTOM VIDEO FILTER"),
		sss_video_filter,
		Settings::getInstance()->getString("SlideshowScreenSaverVideoFilter")
	);

	addSaveFunc([sss_video_filter] {
		Settings::getInstance()->setString(
			"SlideshowScreenSaverVideoFilter",
			sss_video_filter->getValue()
		);
	});
}

GuiSlideshowScreensaverOptions::~GuiSlideshowScreensaverOptions()
{
}

void GuiSlideshowScreensaverOptions::addWithLabel(
	ComponentListRow row,
	const std::string label,
	std::shared_ptr<GuiComponent> component)
{
	row.elements.clear();

	auto lbl = std::make_shared<TextComponent>(
		mWindow,
		Utils::String::toUpper(label),
		Font::get(FONT_SIZE_MEDIUM),
		0x777777FF
	);

	row.addElement(lbl, true);
	row.addElement(component, false, true);

	addRow(row);
}