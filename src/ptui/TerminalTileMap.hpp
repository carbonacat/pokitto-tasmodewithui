#ifndef PTUI_TERMINALTILEMAP_HPP
#   define PTUI_TERMINALTILEMAP_HPP

#   include <array>


namespace ptui
{
    // A TileMap that can be used as a terminal, with printing capabilities, scrolling, etc.
    template<unsigned columnsP, unsigned rowsP,
             unsigned tileWidthP, unsigned tileHeightP,
             unsigned lineWidthP>
    class TerminalTileMap
    {
    public: // Types & Constants.
        using Tile = std::uint8_t;
        
        static constexpr auto columns = columnsP;
        static constexpr auto rows = rowsP;
        static constexpr auto tileWidth = tileWidthP;
        static constexpr auto tileHeight = tileHeightP;
        static constexpr auto lineWidth = lineWidthP;
        
        
    public: // Drawing & Printing.
        // Changes a given tile in this map.
        // Safe - x & y are checked.
        void setTile(int x, int y, Tile tile) noexcept
        {
            if ((x < 0) || (x >= columnsP) || (y < 0) || (y >= rowsP))
                return;
            _tiles[_tileIndex(x, y)] = tile;
        }
        
        
    public: // Rendering.
        // Renders a single line.
        void fillLine(std::uint8_t* lineBuffer, int y, bool skip) noexcept
        {
            if (y == 0)
            {
                _tileSubY = 0;
                _tileY = 0;
            }
            else
            {
                if (_tileSubY == tileHeightP - 1)
                {
                    _tileSubY = 0;
                    _tileY++;
                }
                else
                    _tileSubY++;
            }
            if (!skip)
            {
                int tileSubX = 0;
                int tileX = 0;
                
                for (int index = _indexStart; index < _indexEnd; index++)
                {
                    auto pixelP = lineBuffer + index;
                    
                    // TODO: Can be ++'d instead of computed again.
                    *pixelP += _tiles[_tileIndex(tileX, _tileY)];
                    if (tileSubX == tileWidthP - 1)
                    {
                        tileSubX = 0;
                        tileX++;
                    }
                    else
                        tileSubX++;
                }
            }
        }
        
    private:
        using Tiles = std::array<Tile, columnsP * rowsP>;
    
        static constexpr short _safeIndexEnd(short index) noexcept
        {
            return std::min(lineWidthP, index + columnsP * tileWidthP);
        }
        static constexpr auto _tileIndex(short tileX, short tileY) noexcept
        {
            return tileX + tileY * columnsP;
        }
    
        // Changes only when an offset is introduced.
        short _indexStart = 0;
        short _indexEnd = _safeIndexEnd(0);
        
        // The y coordinate within the current tile.
        short _tileSubY = 0;
        short _tileY = 0;

        Tiles _tiles;
    };
}


#endif // PTUI_TERMINALTILEMAP_HPP