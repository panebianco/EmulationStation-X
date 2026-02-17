#include "guis/GuiGeneralScreensaverOptions.h"

#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "components/TextComponent.h"

#include "guis/GuiMsgBox.h"
#include "guis/GuiSlideshowScreensaverOptions.h"
#include "guis/GuiVideoScreensaverOptions.h"

#include "Settings.h"
#include "PowerSaver.h"
#include "resources/Font.h"
#include "LocaleES.h"

#include <vector>
#include <string>

GuiGeneralScreensaverOptions::GuiGeneralScreensaverOptions(Window* window, const char* title)
    : GuiScreensaverOptions(window, title)
{
    // Helper local seguro (usable dentro de lambdas)
    auto T = [](const std::string& key) -> std::string {
        return LocaleES::getInstance().translate(key);
    };

    // ------------------------------------------------------------
    // SCREENSAVER AFTER
    // ------------------------------------------------------------
    auto screensaver_time = std::make_shared<SliderComponent>(mWindow, 0.f, 30.f, 1.f, "m");
    screensaver_time->setValue(
        (float)(Settings::getInstance()->getInt("ScreenSaverTime") / Settings::ONE_MINUTE_IN_MS)
    );

    addWithLabel(T("SCREENSAVER AFTER"), screensaver_time);

    addSaveFunc([screensaver_time] {
        Settings::getInstance()->setInt(
            "ScreenSaverTime",
            (int)Math::round(screensaver_time->getValue()) * Settings::ONE_MINUTE_IN_MS
        );
        PowerSaver::updateTimeouts();
    });

    // ------------------------------------------------------------
    // SCREENSAVER CONTROLS
    // ------------------------------------------------------------
    auto ss_controls = std::make_shared<SwitchComponent>(mWindow);
    ss_controls->setState(Settings::getInstance()->getBool("ScreenSaverControls"));

    addWithLabel(T("SCREENSAVER CONTROLS"), ss_controls);

    addSaveFunc([ss_controls] {
        Settings::getInstance()->setBool("ScreenSaverControls", ss_controls->getState());
    });

    // ------------------------------------------------------------
    // SCREENSAVER BEHAVIOR
    // ------------------------------------------------------------
    auto screensaver_behavior =
        std::make_shared<OptionListComponent<std::string>>(
            mWindow,
            T("SCREENSAVER BEHAVIOR"),
            false
        );

    struct BehaviorItem
    {
        std::string value;     // valor interno (Settings)
        std::string labelKey;  // clave traducible
    };

    const std::vector<BehaviorItem> behaviors = {
        { "dim",          "SS_BEHAVIOR_DIM" },
        { "black",        "SS_BEHAVIOR_BLACK" },
        { "random video", "SS_BEHAVIOR_RANDOM_VIDEO" },
        { "slideshow",    "SS_BEHAVIOR_SLIDESHOW" }
    };

    const std::string currentBehavior =
        Settings::getInstance()->getString("ScreenSaverBehavior");

    for (const auto& b : behaviors)
        screensaver_behavior->add(
            T(b.labelKey),
            b.value,
            (currentBehavior == b.value)
        );

    addWithLabel(T("SCREENSAVER BEHAVIOR"), screensaver_behavior);

    addSaveFunc([this, screensaver_behavior] {
        auto T2 = [](const std::string& key) -> std::string {
            return LocaleES::getInstance().translate(key);
        };

        const std::string prev =
            Settings::getInstance()->getString("ScreenSaverBehavior");

        const std::string next =
            screensaver_behavior->getSelected();

        // Advertencia solo al activar random video
        if (prev != "random video" && next == "random video")
        {
            std::string msg =
                T2("SS_RANDOM_VIDEO_WARNING_1") + "\n\n" +
                T2("SS_RANDOM_VIDEO_WARNING_2") + "\n\n" +
                T2("SS_RANDOM_VIDEO_WARNING_3");

            mWindow->pushGui(
                new GuiMsgBox(mWindow, msg, T2("OK"), [] { return; })
            );
        }

        Settings::getInstance()->setString("ScreenSaverBehavior", next);
        PowerSaver::updateTimeouts();
    });

    // ------------------------------------------------------------
    // SUBMENÚ VIDEO
    // ------------------------------------------------------------
    {
        ComponentListRow row;
        row.elements.clear();

        row.addElement(
            std::make_shared<TextComponent>(
                mWindow,
                T("VIDEO SCREENSAVER SETTINGS"),
                Font::get(FONT_SIZE_MEDIUM),
                0x777777FF
            ),
            true
        );

        row.addElement(makeArrow(mWindow), false);

        row.makeAcceptInputHandler(
            std::bind(&GuiGeneralScreensaverOptions::openVideoScreensaverOptions, this)
        );

        addRow(row);
    }

    // ------------------------------------------------------------
    // SUBMENÚ SLIDESHOW
    // ------------------------------------------------------------
    {
        ComponentListRow row;
        row.elements.clear();

        row.addElement(
            std::make_shared<TextComponent>(
                mWindow,
                T("SLIDESHOW SCREENSAVER SETTINGS"),
                Font::get(FONT_SIZE_MEDIUM),
                0x777777FF
            ),
            true
        );

        row.addElement(makeArrow(mWindow), false);

        row.makeAcceptInputHandler(
            std::bind(&GuiGeneralScreensaverOptions::openSlideshowScreensaverOptions, this)
        );

        addRow(row);
    }

    // ------------------------------------------------------------
    // SYSTEM SLEEP AFTER
    // ------------------------------------------------------------
    const float stepw = 5.f;
    const float max   = 120.f;

    auto system_sleep_time =
        std::make_shared<SliderComponent>(mWindow, 0.f, max, stepw, "m");

    system_sleep_time->setValue(
        (float)(Settings::getInstance()->getInt("SystemSleepTime") / Settings::ONE_MINUTE_IN_MS)
    );

    addWithLabel(T("SYSTEM SLEEP AFTER"), system_sleep_time);

    addSaveFunc([this, system_sleep_time, screensaver_time, max, stepw] {

        auto T2 = [](const std::string& key) -> std::string {
            return LocaleES::getInstance().translate(key);
        };

        if (screensaver_time->getValue() > system_sleep_time->getValue()
            && system_sleep_time->getValue() > 0)
        {
            int steps = Math::min(
                1 + (int)(screensaver_time->getValue() / stepw),
                (int)(max / stepw)
            );

            int adj_system_sleep_time = steps * (int)stepw;
            system_sleep_time->setValue((float)adj_system_sleep_time);

            std::string msg;

            if (!Settings::getInstance()->getBool("SystemSleepTimeHintDisplayed"))
            {
                msg += T2("SS_SLEEP_NOTE_1") + "\n";
                msg += T2("SS_SLEEP_NOTE_2");
                Settings::getInstance()->setBool("SystemSleepTimeHintDisplayed", true);
            }

            if (!msg.empty())
                msg += "\n\n";

            msg += T2("SS_SLEEP_ADJUST_1");
            msg += "\n\n";
            msg += T2("SS_SLEEP_ADJUST_2") + " "
                 + std::to_string(adj_system_sleep_time) + " "
                 + T2("SS_SLEEP_ADJUST_3");

            mWindow->pushGui(
                new GuiMsgBox(mWindow, msg, T2("OK"), [] { return; })
            );
        }

        Settings::getInstance()->setInt(
            "SystemSleepTime",
            (int)Math::round(system_sleep_time->getValue()) * Settings::ONE_MINUTE_IN_MS
        );
    });
}

GuiGeneralScreensaverOptions::~GuiGeneralScreensaverOptions()
{
}

// ============================================================
// SUBMENÚ VIDEO
// ============================================================
void GuiGeneralScreensaverOptions::openVideoScreensaverOptions()
{
    const std::string title =
        LocaleES::getInstance().translate("VIDEO SCREENSAVER");

    mWindow->pushGui(
        new GuiVideoScreensaverOptions(mWindow, title.c_str())
    );
}

// ============================================================
// SUBMENÚ SLIDESHOW
// ============================================================
void GuiGeneralScreensaverOptions::openSlideshowScreensaverOptions()
{
    const std::string title =
        LocaleES::getInstance().translate("SLIDESHOW SCREENSAVER");

    mWindow->pushGui(
        new GuiSlideshowScreensaverOptions(mWindow, title.c_str())
    );
}
