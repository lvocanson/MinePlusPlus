#pragma once
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace Resources
{

inline const std::filesystem::path RESOURCES_DIR = "res";

namespace Fonts
{

constexpr auto ROBOTO_FILE = "roboto-v48-latin-regular.ttf";

inline const sf::Font roboto{RESOURCES_DIR / ROBOTO_FILE};

} // namespace Fonts

namespace Textures
{

constexpr auto UNOPENED_CELL_FILE = "unopened-cell.png";
constexpr auto UNOPENED_SELECTED_CELL_FILE = "unopened-selected-cell.png";
constexpr auto UNOPENED_FLAGGED_CELL_FILE = "unopened-flagged-cell.png";
constexpr auto OPENED_CELL_0_FILE = "opened-cell-0.png";
constexpr auto OPENED_CELL_1_FILE = "opened-cell-1.png";
constexpr auto OPENED_CELL_2_FILE = "opened-cell-2.png";
constexpr auto OPENED_CELL_3_FILE = "opened-cell-3.png";
constexpr auto OPENED_CELL_4_FILE = "opened-cell-4.png";
constexpr auto OPENED_CELL_5_FILE = "opened-cell-5.png";
constexpr auto OPENED_CELL_6_FILE = "opened-cell-6.png";
constexpr auto OPENED_CELL_7_FILE = "opened-cell-7.png";
constexpr auto OPENED_CELL_8_FILE = "opened-cell-8.png";
constexpr auto OPENED_CELL_CLICKED_MINE_FILE = "opened-cell-clicked-mine.png";
constexpr auto OPENED_CELL_RUNNING_MINE_FILE = "opened-cell-running-mine.png";
constexpr auto OPENED_CELL_NO_MINE_FILE = "opened-cell-no-mine.png";
constexpr auto OPENED_CELL_MINE_FILE = "opened-cell-mine.png";

inline const sf::Texture unopenedCell{RESOURCES_DIR / UNOPENED_CELL_FILE};
inline const sf::Texture unopenedSelectedCell{RESOURCES_DIR / UNOPENED_SELECTED_CELL_FILE};
inline const sf::Texture unopenedFlaggedCell{RESOURCES_DIR / UNOPENED_FLAGGED_CELL_FILE};
inline const sf::Texture openedCell0{RESOURCES_DIR / OPENED_CELL_0_FILE};
inline const sf::Texture openedCell1{RESOURCES_DIR / OPENED_CELL_1_FILE};
inline const sf::Texture openedCell2{RESOURCES_DIR / OPENED_CELL_2_FILE};
inline const sf::Texture openedCell3{RESOURCES_DIR / OPENED_CELL_3_FILE};
inline const sf::Texture openedCell4{RESOURCES_DIR / OPENED_CELL_4_FILE};
inline const sf::Texture openedCell5{RESOURCES_DIR / OPENED_CELL_5_FILE};
inline const sf::Texture openedCell6{RESOURCES_DIR / OPENED_CELL_6_FILE};
inline const sf::Texture openedCell7{RESOURCES_DIR / OPENED_CELL_7_FILE};
inline const sf::Texture openedCell8{RESOURCES_DIR / OPENED_CELL_8_FILE};
inline const sf::Texture openedCellClickedMine{RESOURCES_DIR / OPENED_CELL_CLICKED_MINE_FILE};
inline const sf::Texture openedCellRunningMine{RESOURCES_DIR / OPENED_CELL_RUNNING_MINE_FILE};
inline const sf::Texture openedCellNoMine{RESOURCES_DIR / OPENED_CELL_NO_MINE_FILE};
inline const sf::Texture openedCellMine{RESOURCES_DIR / OPENED_CELL_MINE_FILE};

} // namespace Textures

} // namespace Resources
