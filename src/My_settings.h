#include "ptui/TASLineFiller.hpp"

#define PROJ_SCREENMODE TASMODE
#define PROJ_TILE_H 16
#define PROJ_TILE_W 16
#define MAX_TILE_COUNT 256
#define PROJ_FPS 250
#define PROJ_USE_FPS_COUNTER

#define PROJ_LINE_FILLERS TAS::BGTileFiller, TAS::SpriteFiller, ptui::TerminalTMFiller
