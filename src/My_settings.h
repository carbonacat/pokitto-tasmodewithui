#include "ptui/TASLineFiller.hpp"


void GameFiller(std::uint8_t* line, std::uint32_t y, bool skip) noexcept;

#define PROJ_SCREENMODE TASMODE
#define PROJ_TILE_H 16
#define PROJ_TILE_W 16
#define MAX_TILE_COUNT 256
#define PROJ_FPS 250
#define PROJ_USE_FPS_COUNTER
#define PROJ_BUTTONS_POLLING_ONLY

#define PROJ_LINE_FILLERS GameFiller
// #define PROJ_LINE_FILLERS TAS::BGTileFiller, TAS::SpriteFiller, ptui::TerminalTMFiller
