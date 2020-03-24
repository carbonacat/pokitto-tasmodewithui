#ifndef PTUI_UITILEMAP_HPP
#   define PTUI_UITILEMAP_HPP

#   include "ptui/BaseTileMap.hpp"


namespace ptui
{
    // An extension of BaseTileMap revolving around a UI TileSet.
    // Provides the following services:
    // - Text printing with bounds and bounded autoscroll. [Terminal]
    // - Box, checkbox, gauge drawing. [Widgets]
    template<unsigned columnsP, unsigned rowsP,
             unsigned tileWidthP, unsigned tileHeightP,
             unsigned lineWidthP>
    class UITileMap:
        BaseTileMap
    {
        
    };
}


#endif // PTUI_UITILEMAP_HPP