#ifndef PTUI_TASTERMINALTILEMAP_HPP
#   define PTUI_TASTERMINALTILEMAP_HPP

#   include "Pokitto.h"

#   include "ptui/TerminalTileMap.hpp"


namespace ptui
{
    constexpr unsigned ttmColumns = 37;
    constexpr unsigned ttmRows = 30;
    constexpr unsigned ttmTileWidth = 6;
    constexpr unsigned ttmTileHeight = 6;
    constexpr unsigned lcdWidth = PROJ_LCDWIDTH;
    constexpr unsigned lcdHeight = PROJ_LCDHEIGHT;
    
    using TASUITileMap = TerminalTileMap<ttmColumns, ttmRows, ttmTileWidth, ttmTileHeight, lcdWidth>;
    
    inline TASUITileMap tasUITileMap;
}


#endif // PTUI_TASTERMINALTILEMAP_HPP