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
        static constexpr auto tileSize = tileWidth * tileHeightP * 1; // 8bpp
        static constexpr auto width = columns * tileWidthP;
        static constexpr auto height = rows * tileHeightP;
        static constexpr auto lineWidth = lineWidthP;
        
        
    public: // Configurations.
        // Changes the origin to the top-left of the display.
        void setOffset(int offsetX, int offsetY) noexcept
        {
            _offsetX = offsetX;
            _offsetY = offsetY;
            if (offsetX >= 0)
            {
                _indexStart = offsetX;
                _indexEnd = std::min<int>(lineWidth, _indexStart + width);
                _tileXStart = 0;
                _tileSubXStart = 0;
            }
            else
            {
                _indexStart = 0;
                _indexEnd = std::min<int>(lineWidth, _indexStart + offsetX + width);
                _tileXStart = -offsetX / tileWidth;
                _tileSubXStart = -offsetX - _tileXStart * tileWidth;
            }
            if (offsetY >= 0)
            {
                _tileYStart = -offsetY / static_cast<int>(tileWidth);
                _tileSubYStart = -offsetY - _tileYStart * tileWidth;
            }
            else
            {
                _tileYStart = -offsetY / tileWidth;
                _tileSubYStart = -offsetY - _tileYStart * tileWidth;
            }
        }
        auto offsetX() const noexcept
        {
            return _offsetX;
        }
        auto offsetY() const noexcept
        {
            return _offsetY;
        }
        
        // Must be called before use!
        // Is a 8BPP tileset where each tile is 6x6 pixels.
        void setTileset(const unsigned char* tilesetData) noexcept
        {
            _tilesetData = tilesetData;
        }
        
        
    public: // Drawing & Printing.
        // Changes a given tile in this map.
        // Safe - x & y are checked.
        void setTile(int x, int y, Tile tile) noexcept
        {
            if ((x < 0) || (x >= columns) || (y < 0) || (y >= rows))
                return;
            _tiles[_tileIndex(x, y)] = tile;
        }
        
        
    public: // Rendering.
        // Renders a single line.
        void fillLine(std::uint8_t* lineBuffer, int y, bool skip) noexcept
        {
            if (y == 0)
            {
                _tileY = _tileYStart;
                _tileSubY = _tileSubYStart;
            }
            else
            {
                if (_tileSubY == tileHeight - 1)
                {
                    _tileSubY = 0;
                    _tileY++;
                }
                else
                    _tileSubY++;
            }
            if ((y < _offsetY) || (_tileY >= static_cast<int>(rows)))
                return ;
            if (!skip)
            {
                int tileSubX = _tileSubXStart;
                int tileX = _tileXStart;
                
                for (int index = _indexStart; index < _indexEnd; index++)
                {
                    auto pixelP = lineBuffer + index;
                    
                    // TODO: Can be ++'d instead of computed again.
                    // TODO: Can also be skipped completely if empty.
                    auto tile = _tiles[_tileIndex(tileX, _tileY)];
                    
                    if (tile != 0)
                    {
                        auto tileIndex = tile * tileSize;
                        auto tileData = _tilesetData + tileIndex;
                        auto tilePixel = tileData[_tileSubY * tileWidth + tileSubX];
                        
                        if (tilePixel != 0)
                            *pixelP = tilePixel;
                    }
                    if (tileSubX == tileWidth - 1)
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
        using Tiles = std::array<Tile, columns * rows>;
    
        static constexpr auto _tileIndex(short tileX, short tileY) noexcept
        {
            return tileX + tileY * columns;
        }
    
        short _offsetX = 0;
        short _offsetY = 0;
    
        // Changes only when an offset is introduced.
        short _indexStart = 0;
        short _indexEnd = std::min(lineWidth, 0 + width);
        
        // The y coordinate within the current tile.
        short _tileSubY = 0;
        short _tileY = 0;
        
        short _tileXStart = 0;
        short _tileSubXStart = 0;
        short _tileYStart = 0;
        short _tileSubYStart = 0;

        const unsigned char* _tilesetData = nullptr;
        Tiles _tiles;
    };
}


#endif // PTUI_TERMINALTILEMAP_HPP