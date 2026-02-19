#include "guis/GuiVideoScreensaverOptions.h"

#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "components/TextComponent.h"

#include "guis/GuiMsgBox.h"

#include "Settings.h"
#include "PowerSaver.h"
#include "LocaleES.h"
#include "resources/Font.h"

#include <vector>
#include <string>

GuiVideoScreensaverOptions::GuiVideoScreensaverOptions(Window* window, const char* title)
	: GuiScreensaverOptions(window, title)
{
	// Helper local seguro (usable en lambdas)
	auto T = [](const std::string& key) -> std::string {
		return LocaleES::getInstance().translate(key);
	};

	// ------------------------------------------------------------
	// SWAP VIDEO AFTER
	// ------------------------------------------------------------
	auto swap = std::make_shared<SliderComponent>(mWindow, 10.f, 1000.f, 1.f, "s");
	swap->setValue((float)(Settings::getInstance()->getInt("ScreenSaverSwapVideoTimeout") / 1000));
	addWithLabel(T("SWAP VIDEO AFTER (SECS)"), swap);

	addSaveFunc([swap] {
		int playNextTimeout = (int)Math::round(swap->getValue()) * 1000;
		Settings::getInstance()->setInt("ScreenSaverSwapVideoTimeout", playNextTimeout);
		PowerSaver::updateTimeouts();
	});

	// ------------------------------------------------------------
	// STRETCH VIDEO
	// ------------------------------------------------------------
	auto stretch_screensaver = std::make_shared<SwitchComponent>(mWindow);
	stretch_screensaver->setState(Settings::getInstance()->getBool("StretchVideoOnScreenSaver"));
	addWithLabel(T("STRETCH VIDEO ON SCREENSAVER"), stretch_screensaver);

	addSaveFunc([stretch_screensaver] {
		Settings::getInstance()->setBool("StretchVideoOnScreenSaver", stretch_screensaver->getState());
	});

#ifdef _OMX_
	// ------------------------------------------------------------
	// OMX PLAYER
	// ------------------------------------------------------------
	auto ss_omx = std::make_shared<SwitchComponent>(mWindow);
	ss_omx->setState(Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
	addWithLabel(T("USE OMX PLAYER FOR SCREENSAVER"), ss_omx);

	addSaveFunc([ss_omx] {
		Settings::getInstance()->setBool("ScreenSaverOmxPlayer", ss_omx->getState());
	});
#endif

	// ------------------------------------------------------------
	// SHOW GAME INFO (subtitles)
	//  - IMPORTANTE: guardamos el valor interno en inglés
	// ------------------------------------------------------------
	auto ss_info = std::make_shared<OptionListComponent<std::string>>(mWindow, T("SHOW GAME INFO"), false);

	struct InfoItem { std::string value; std::string labelKey; };
	const std::vector<InfoItem> info_type = {
		{ "always",      "SS_INFO_ALWAYS" },
		{ "start & end", "SS_INFO_START_END" },
		{ "never",       "SS_INFO_NEVER" }
	};

	const std::string curInfo = Settings::getInstance()->getString("ScreenSaverGameInfo");

	for (const auto& it : info_type)
		ss_info->add(T(it.labelKey), it.value, curInfo == it.value);

	addWithLabel(T("SHOW GAME INFO ON SCREENSAVER"), ss_info);

	addSaveFunc([ss_info] {
		Settings::getInstance()->setString("ScreenSaverGameInfo", ss_info->getSelected());
	});

	// ------------------------------------------------------------
	// MUTE AUDIO
	// ------------------------------------------------------------
	auto ss_video_mute = std::make_shared<SwitchComponent>(mWindow);
	ss_video_mute->setState(Settings::getInstance()->getBool("ScreenSaverVideoMute"));
	addWithLabel(T("MUTE SCREENSAVER AUDIO"), ss_video_mute);

	addSaveFunc([ss_video_mute] {
		Settings::getInstance()->setBool("ScreenSaverVideoMute", ss_video_mute->getState());
	});

	// ------------------------------------------------------------
	// VLC RESOLUTION
	//  - IMPORTANTE: guardamos el valor interno en inglés
	// ------------------------------------------------------------
	auto ss_vlc_resolution = std::make_shared<OptionListComponent<std::string>>(mWindow, T("VIDEO RESOLUTION"), false);

	struct ResItem { std::string value; std::string labelKey; };
	const std::vector<ResItem> vlc_res = {
		{ "original", "VLC_RES_ORIGINAL" },
		{ "low",      "VLC_RES_LOW" },
		{ "medium",   "VLC_RES_MEDIUM" },
		{ "high",     "VLC_RES_HIGH" },
		{ "max",      "VLC_RES_MAX" }
	};

	const std::string curRes = Settings::getInstance()->getString("VlcScreenSaverResolution");

	for (const auto& it : vlc_res)
		ss_vlc_resolution->add(T(it.labelKey), it.value, curRes == it.value);

	addWithLabel(T("VLC: SCREENSAVER VIDEO RESOLUTION"), ss_vlc_resolution);

	addSaveFunc([ss_vlc_resolution] {
		Settings::getInstance()->setString("VlcScreenSaverResolution", ss_vlc_resolution->getSelected());
	});

#ifdef _OMX_
	ComponentListRow row;

	// ------------------------------------------------------------
	// OMX SUBTITLE ALIGNMENT
	//  - guardamos left/center (interno)
	// ------------------------------------------------------------
	auto ss_omx_subs_align = std::make_shared<OptionListComponent<std::string>>(mWindow, T("GAME INFO ALIGNMENT"), false);

	struct AlignItem { std::string value; std::string labelKey; };
	const std::vector<AlignItem> align_mode = {
		{ "left",   "ALIGN_LEFT" },
		{ "center", "ALIGN_CENTER" }
	};

	const std::string curAlign = Settings::getInstance()->getString("SubtitleAlignment");

	for (const auto& it : align_mode)
		ss_omx_subs_align->add(T(it.labelKey), it.value, curAlign == it.value);

	addWithLabel(T("OMX: GAME INFO ALIGNMENT"), ss_omx_subs_align);

	addSaveFunc([ss_omx_subs_align] {
		Settings::getInstance()->setString("SubtitleAlignment", ss_omx_subs_align->getSelected());
	});

	// ------------------------------------------------------------
	// OMX SUBTITLE FONT SIZE
	// ------------------------------------------------------------
	auto ss_omx_font_size = std::make_shared<SliderComponent>(mWindow, 1.f, 64.f, 1.f, "h");
	ss_omx_font_size->setValue((float)(Settings::getInstance()->getInt("SubtitleSize")));
	addWithLabel(T("OMX: GAME INFO FONT SIZE"), ss_omx_font_size);

	addSaveFunc([ss_omx_font_size] {
		int subSize = (int)Math::round(ss_omx_font_size->getValue());
		Settings::getInstance()->setInt("SubtitleSize", subSize);
	});

	// ------------------------------------------------------------
	// OMX PATHS
	// ------------------------------------------------------------
	auto ss_omx_font_file = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF);
	addEditableTextComponent(row, T("OMX: PATH TO FONT FILE"), ss_omx_font_file, Settings::getInstance()->getString("SubtitleFont"));

	addSaveFunc([ss_omx_font_file] {
		Settings::getInstance()->setString("SubtitleFont", ss_omx_font_file->getValue());
	});

	auto ss_omx_italic_font_file = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF);
	addEditableTextComponent(row, T("OMX: PATH TO ITALIC FONT FILE"), ss_omx_italic_font_file, Settings::getInstance()->getString("SubtitleItalicFont"));

	addSaveFunc([ss_omx_italic_font_file] {
		Settings::getInstance()->setString("SubtitleItalicFont", ss_omx_italic_font_file->getValue());
	});
#endif
}

GuiVideoScreensaverOptions::~GuiVideoScreensaverOptions()
{
}

void GuiVideoScreensaverOptions::save()
{
#ifdef _OMX_
	bool startingStatusNotRisky =
		(Settings::getInstance()->getString("ScreenSaverGameInfo") == "never"
			|| !Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
#endif

	GuiScreensaverOptions::save();

#ifdef _OMX_
	bool endStatusRisky =
		(Settings::getInstance()->getString("ScreenSaverGameInfo") != "never"
			&& Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));

	if (startingStatusNotRisky && endStatusRisky)
	{
		auto T = [](const std::string& key) -> std::string {
			return LocaleES::getInstance().translate(key);
		};

		std::string msg =
			T("OMX_SS_WARNING_1") + "\n\n" +
			T("OMX_SS_WARNING_2") + "\n" +
			T("OMX_SS_WARNING_3") + "\n" +
			T("OMX_SS_WARNING_4") + "\n" +
			T("OMX_SS_WARNING_5");

		mWindow->pushGui(new GuiMsgBox(mWindow, msg, T("GOT IT!"), [] { return; }));
	}
#endif
}