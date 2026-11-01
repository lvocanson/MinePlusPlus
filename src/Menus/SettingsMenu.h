#pragma once
#include "Core/AppEvent.h"
#include "UI/Button.h"
#include "UI/NumberField.h"
#include "UI/Text.h"
#include "UI/ClickTracker.h"
#include "UI/Cursor.h"
#include <cstdint>

struct Preferences;
class Audio;

class SettingsMenu
{
public:

	SettingsMenu(Preferences&);
	~SettingsMenu();
	UIEvent::Result operator()(const UIEvent::Resized& event);
	UIEvent::Result operator()(const UIEvent::Pressed& event);
	UIEvent::Result operator()(const UIEvent::Released& event);
	UIEvent::Result operator()(const UIEvent::Typed& event);
	void render(class UITarget& target) const;

private:

	enum class Reset : std::uint8_t { None, Preferences, Scores };

	void loadFields();
	void applyFields(Audio& audio);
	Button& getResetButton(Reset target);
	void requestOrConfirmReset(Reset target, Audio& audio);
	void cancelReset();

	Preferences& preferences_;
	bool modified_;
	Reset confirming_;
	Text title_, volumeText_, panText_, zoomText_;
	NumberField volumeField_, panField_, zoomField_;
	Button resetPreferencesBtn_, resetScoresBtn_, backBtn_;
	Cursor cursor_;
	ClickTracker tracker_;
};
