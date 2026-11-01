#include "SettingsMenu.h"
#include "MainMenu.h"
#include "Core/App.h"
#include "UI/UITarget.h"
#include <algorithm>
#include <cmath>

namespace
{

constexpr Vec2i FIELD_SIZE = {150, 28};
constexpr Vec2i RESET_BUTTON_SIZE = {170, 28};
constexpr int ROW_WIDTH = 360;
constexpr int ROW_STEP = 40;
constexpr int SECTION_GAP = 70;
constexpr int MENU_HEIGHT = SECTION_GAP * 2 + ROW_STEP * 3;

constexpr std::size_t MAX_VOLUME = std::size_t(Audio::MAX_VOLUME);
constexpr std::size_t MAX_SENSITIVITY_PERCENT = std::size_t(Preferences::MAX_SENSITIVITY * 100.f);

constexpr std::string_view RESET_PREFERENCES_TEXT = "Reset preferences";
constexpr std::string_view RESET_SCORES_TEXT = "Reset scores";
constexpr std::string_view CONFIRM_RESET_TEXT = "Confirm reset?";
constexpr std::string_view PREFERENCES_RESET_TEXT = "Preferences reset";
constexpr std::string_view SCORES_RESET_TEXT = "Scores reset";
constexpr std::string_view RESET_FAILED_TEXT = "Reset failed";

std::size_t toPercent(float sensitivity)
{
	return std::size_t(std::lround(sensitivity * 100.f));
}

float toSensitivity(std::size_t percent)
{
	return std::max(float(percent) / 100.f, Preferences::MIN_SENSITIVITY);
}

}

SettingsMenu::SettingsMenu(Preferences& preferences)
	: preferences_(preferences)
	, modified_(false)
	, confirming_(Reset::None)
	, title_{.origin = Text::Middle, .string = "Settings"}
	, volumeText_{.origin = Text::Left, .string = "Volume (%)"}
	, panText_{.origin = Text::Left, .string = "Pan sensitivity (%)"}
	, zoomText_{.origin = Text::Left, .string = "Zoom sensitivity (%)"}
	, volumeField_{.rect = {{}, FIELD_SIZE}}
	, panField_{.rect = {{}, FIELD_SIZE}}
	, zoomField_{.rect = {{}, FIELD_SIZE}}
	, resetPreferencesBtn_{.rect = {{}, RESET_BUTTON_SIZE}, .text = RESET_PREFERENCES_TEXT}
	, resetScoresBtn_{.rect = {{}, RESET_BUTTON_SIZE}, .text = RESET_SCORES_TEXT}
	, backBtn_{.rect = {{}, FIELD_SIZE}, .text = "Back"}
{
	loadFields();
}

SettingsMenu::~SettingsMenu()
{
	if (modified_)
		Save::savePreferences(preferences_);
}

UIEvent::Result SettingsMenu::operator()(const UIEvent::Resized& event)
{
	Vec2i centerView = event.newSize / 2;
	int left = centerView.x - ROW_WIDTH / 2;
	int right = centerView.x + ROW_WIDTH / 2;
	int y = centerView.y - MENU_HEIGHT / 2;

	title_.position = {centerView.x, y};

	y += SECTION_GAP;
	for (auto [text, field] : {
		     std::pair{&volumeText_, &volumeField_},
		     std::pair{&panText_, &panField_},
		     std::pair{&zoomText_, &zoomField_}
	     })
	{
		text->position = {left, y};
		field->rect.position = {right - field->rect.size.x, y - field->rect.size.y / 2};
		y += ROW_STEP;
	}

	y += SECTION_GAP - ROW_STEP;
	resetPreferencesBtn_.rect.position = {left, y - RESET_BUTTON_SIZE.y / 2};
	resetScoresBtn_.rect.position = {right - RESET_BUTTON_SIZE.x, y - RESET_BUTTON_SIZE.y / 2};

	y += ROW_STEP;
	backBtn_.rect.position = Vec2i{centerView.x, y} - backBtn_.rect.size / 2;
	return UIEvent::Consumed;
}

UIEvent::Result SettingsMenu::operator()(const UIEvent::Pressed& event)
{
	cursor_.field = nullptr;
	loadFields();
	if (confirming_ == Reset::None || !getResetButton(confirming_).rect.contains(event.position))
		cancelReset();

	if (tracker_.registerPress(event.position, {resetPreferencesBtn_, resetScoresBtn_, backBtn_}))
		return UIEvent::Consumed;

	for (NumberField* field : {&volumeField_, &panField_, &zoomField_})
	{
		if (field->rect.contains(event.position))
		{
			cursor_.field = field;
			return UIEvent::Consumed;
		}
	}
	return UIEvent::Ignored;
}

UIEvent::Result SettingsMenu::operator()(const UIEvent::Released& event)
{
	if (tracker_.isClicked(resetPreferencesBtn_, event.position))
	{
		requestOrConfirmReset(Reset::Preferences, event.app.getAudio());
	}
	else if (tracker_.isClicked(resetScoresBtn_, event.position))
	{
		requestOrConfirmReset(Reset::Scores, event.app.getAudio());
	}
	else if (tracker_.isClicked(backBtn_, event.position))
	{
		event.app.submitCommand<SwapUI>(SwapUI::DEFAULT<MainMenu>);
	}
	else
		return UIEvent::Ignored;
	return UIEvent::Consumed;
}

UIEvent::Result SettingsMenu::operator()(const UIEvent::Typed& event)
{
	if (!cursor_.field)
		return UIEvent::Ignored;

	cursor_.field->add(event.utf32);
	applyFields(event.app.getAudio());
	modified_ = true;

	return UIEvent::Consumed;
}

void SettingsMenu::render(UITarget& target) const
{
	target.draw(title_);
	target.draw(volumeText_);
	target.draw(panText_);
	target.draw(zoomText_);
	target.draw(volumeField_);
	target.draw(panField_);
	target.draw(zoomField_);
	target.draw(resetPreferencesBtn_);
	target.draw(resetScoresBtn_);
	target.draw(backBtn_);
	target.draw(cursor_);
}

void SettingsMenu::loadFields()
{
	volumeField_.value = std::size_t(std::lround(preferences_.volume));
	panField_.value = toPercent(preferences_.panSensitivity);
	zoomField_.value = toPercent(preferences_.zoomSensitivity);
}

void SettingsMenu::applyFields(Audio& audio)
{
	volumeField_.value = std::min(volumeField_.value, MAX_VOLUME);
	panField_.value = std::min(panField_.value, MAX_SENSITIVITY_PERCENT);
	zoomField_.value = std::min(zoomField_.value, MAX_SENSITIVITY_PERCENT);

	preferences_.volume = float(volumeField_.value);
	preferences_.panSensitivity = toSensitivity(panField_.value);
	preferences_.zoomSensitivity = toSensitivity(zoomField_.value);
	audio.setVolume(preferences_.volume);
}

Button& SettingsMenu::getResetButton(Reset target)
{
	return target == Reset::Preferences ? resetPreferencesBtn_ : resetScoresBtn_;
}

void SettingsMenu::requestOrConfirmReset(Reset target, Audio& audio)
{
	Button& button = getResetButton(target);
	if (confirming_ != target)
	{
		confirming_ = target;
		button.text = CONFIRM_RESET_TEXT;
		button.textColor = Colors::Red;
		return;
	}

	confirming_ = Reset::None;
	button.textColor = Colors::White;
	if (target == Reset::Preferences)
	{
		preferences_ = {};
		audio.setVolume(preferences_.volume);
		loadFields();
		modified_ = false;
		button.text = Save::savePreferences(preferences_) ? PREFERENCES_RESET_TEXT : RESET_FAILED_TEXT;
	}
	else
	{
		button.text = Save::clearScores() ? SCORES_RESET_TEXT : RESET_FAILED_TEXT;
	}
}

void SettingsMenu::cancelReset()
{
	resetPreferencesBtn_.text = RESET_PREFERENCES_TEXT;
	resetScoresBtn_.text = RESET_SCORES_TEXT;
	resetPreferencesBtn_.textColor = resetScoresBtn_.textColor = Colors::White;
	confirming_ = Reset::None;
}
