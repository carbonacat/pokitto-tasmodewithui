#include "ptui/TASLineFiller.hpp"

#include "Pokitto.h"

#include "ptui/TerminalTileMap.hpp"


namespace ptui
{
    constexpr unsigned ttmColumns = 37;
    constexpr unsigned ttmRows = 30;
    constexpr unsigned ttmTileWidth = 6;
    constexpr unsigned ttmTileHeight = 6;
    constexpr unsigned lcdWidth = PROJ_LCDWIDTH;
    constexpr unsigned lcdHeight = PROJ_LCDHEIGHT;
    
    using TheTerminalTileMap = TerminalTileMap<ttmColumns, ttmRows, ttmTileWidth, ttmTileHeight, lcdWidth>;
    
    TheTerminalTileMap theTerminalTileMap;
    
    void TerminalTMFiller(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
    {
        theTerminalTileMap.fillLine(line, y, skip);
    }
}