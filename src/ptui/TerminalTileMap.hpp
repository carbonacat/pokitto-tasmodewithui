#ifndef PTUI_TERMINALTILEMAP_HPP
#   define PTUI_TERMINALTILEMAP_HPP


namespace ptui
{
    // A TileMap that can be used as a terminal, with printing capabilities, scrolling, etc.
    template<unsigned columnsP, unsigned rowsP,
             unsigned tileWidthP, unsigned tileHeightP,
             unsigned lineWidthP>
    class TerminalTileMap
    {
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
                    
                    *pixelP += tileX + _tileY;
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
        static constexpr short _safeIndexEnd(short index) noexcept
        {
            return std::min(lineWidthP, index + columnsP * tileWidthP);
        }
    
        // Changes only when an offset is introduced.
        short _indexStart = 0;
        short _indexEnd = _safeIndexEnd(0);
        
        // The y coordinate within the current tile.
        short _tileSubY = 0;
        short _tileY = 0;
    };
}


#endif // PTUI_TERMINALTILEMAP_HPP