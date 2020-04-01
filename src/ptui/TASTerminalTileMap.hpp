#ifndef PTUI_TASTERMINALTILEMAP_HPP
#   define PTUI_TASTERMINALTILEMAP_HPP

#   include "Pokitto.h"

#   include "ptui/UITileMap.hpp"


namespace ptui
{
    constexpr unsigned lcdWidth = PROJ_LCDWIDTH;
    constexpr unsigned lcdHeight = PROJ_LCDHEIGHT;
    constexpr unsigned ttmTileWidth = 6;
    constexpr unsigned ttmTileHeight = 6;
    // Calculates the right number of tiles depending on the screen size.
    constexpr unsigned ttmFullDisplayColumns = (lcdWidth + ttmTileWidth - 1) / ttmTileWidth;
    constexpr unsigned ttmFullDisplayRows = (lcdHeight + ttmTileHeight - 1) / ttmTileHeight;
    
    constexpr unsigned ttmColumns = ttmFullDisplayColumns;
    constexpr unsigned ttmRows = ttmFullDisplayRows;
    
    using TASUITileMap = UITileMap<ttmColumns, ttmRows, ttmTileWidth, ttmTileHeight, lcdWidth, true, 256>;
    
    extern TASUITileMap tasUITileMap;
}


#endif // PTUI_TASTERMINALTILEMAP_HPP