#pragma once
#include "Menus/MainMenu.h"
#include "Menus/PlayMenu.h"
#include "Menus/CustomMenu.h"
#include "Menus/SettingsMenu.h"
#include "Game/GameUI.h"
#include <variant>

using AppUI = std::variant<
	MainMenu,
	PlayMenu,
	CustomMenu,
	SettingsMenu,
	GameUI>;
