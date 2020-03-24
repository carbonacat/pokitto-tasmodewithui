#ifndef PTUI_BASETILEMAP_HPP
#   define PTUI_BASETILEMAP_HPP

#   include <array>


namespace ptui
{
    // A 8BPP TileMap implementation.
    // Provides the following services:
    // - Set/Get an arbitrary tile. [Tiles Access]
    // - Clear. [Mass Tiles Access]
    // - Shift. [Mass Tiles Access]
    // - Change the Tileset. [Configurations]
    // - Set/Get the offset to the Screen. [Configurations]
    // - Render a scanline using the provided Tileset and offset. [Rendering]
    //
    // - column and row are used for the grid coordinates' names.
    // - x and y are used as the pixel/display coordinates' name.
    // For a more interesting implementation, see UITileMap.
    template<unsigned columnsP, unsigned rowsP,
             unsigned tileWidthP, unsigned tileHeightP,
             unsigned lineWidthP>
    class BaseTileMap
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
        // - 0, 0 means the top-left pixel of the map will be the same than the top-left pixel of the display.
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
        
        
    public: // Tiles Access.
        // Changes a given tile in this map.
        // - Safe - If column or row are outside the map, nothing will happen.
        void set(int column, int row, Tile tile) noexcept
        {
            if (areCoordsValid(column, row))
                _tiles[_tileIndex(column, row)] = tile;
        }
        
        // Returns a given tile in this map.
        // - Safe - If column or row are outside the map, `outsideTile` is returned.
        Tile get(int column, int row, Tile outsideTile = 0) const noexcept
        {
            if (areCoordsValid(column, row))
                return _tiles[_tileIndex(column, row)];
            return outsideTile;
        }
        
    
    public: // Mass Tiles Access.
        // Sets the whole map to the given Tile (Default Tile will make it blank).
        void clear(Tile tile = 0) noexcept
        {
            std::fill(_tiles.begin(), _tiles.end(), tile);
        }
        
        // Sets a given area of the map to the given Tile (0 by default).
        // - `firstColumn`, `firstRow`, `lastColumn` and `lastRow` will be clamped.
        // - `lastColumn` and `lastRow` are included.
        // - Negative and reversed boxes are considered as empty ones.
        void clear(int firstColumn, int firstRow, int lastColumn, int lastRow, Tile tile = 0) noexcept
        {
            firstColumn = clampColumn(firstColumn);
            firstRow = clampRow(firstRow);
            lastColumn = clampColumn(lastColumn);
            lastRow = clampRow(lastRow);
            // TODO: Optimizable with cached indexes calculation. (but is it worth it? :P)
            for (int row = firstRow; row <= lastRow; row++)
                for (int column = firstColumn; column <= lastColumn; column++)
                    set(column, row, tile);
        }
        
        // Shift the whole map by `shiftedColumns` columns and `shiftedRows` rows.
        // - "Introduced" Tiles will be *left as is*.
        void shift(int shiftedColumns, int shiftedRows) noexcept
        {
            int firstColumn;
            int endColumn;
            int columnIncrement;
            int firstRow;
            int endRow;
            int rowIncrement;
            
            if (shiftedColumns < 0)
            {
                // We start from the last column.
                firstColumn = columns - 1;
                endColumn = -shiftedColumns - 1;
                columnIncrement = -1;
            }
            else
            {
                // We start from the first column.
                firstColumn = 0;
                endColumn = columns - shiftedColumns;
                columnIncrement = 1;
            }
            if (shiftedRows < 0)
            {
                // We start from the last column.
                firstRow = rows - 1;
                endRow = -shiftedRows - 1;
                rowIncrement = -1;
            }
            else
            {
                // We start from the first column.
                firstRow = 0;
                endRow = rows - shiftedRows;
                rowIncrement = 1;
            }
            for (int row = firstRow; row != endRow; row += rowIncrement)
                for (int column = firstColumn; column != endColumn; column += columnIncrement)
                    set(column, row, get(column + shiftedColumns, row + shiftedRows));
        }
        
        
    public: // Rendering.
        // Renders a single line.
        void fillLine(std::uint8_t* lineBuffer, int y, bool skip) noexcept
        {
            // Row initialization / change.
            
            if (y == 0)
            {
                _tileY = _tileYStart;
                _tileSubY = _tileSubYStart;
                _tileDataRowBase = _tilesetData + _tileSubY * tileWidth;
            }
            else
            {
                if (_tileSubY == tileHeight - 1)
                {
                    // Onto the next row!
                    _tileSubY = 0;
                    _tileY++;
                    _tileDataRowBase = _tilesetData;
                }
                else
                {
                    // Onto the next pixel line of the same tile row!
                    _tileSubY++;
                    _tileDataRowBase += tileWidth;
                }
            }
            if ((skip) || (y < _offsetY) || (_tileY >= static_cast<int>(rows)))
                return ;
            
            // Scanline rendition.
            
            auto tileDataRowBase = _tileDataRowBase; // Won't mutate, will be read in a loop -> stored locally.
            
            // TODO: For a given row, this code's execution is the same, except for the tileDataP and tileDataPLast which are offset by one tile's line each time.
            // TODO: Could be moved into the row change section above, at the cost of RAM.
            // Points to the first tile of the row.
            const Tile* tileP = &_tiles[_tileIndex(_tileXStart, _tileY)];
            std::uint8_t* pixelP = lineBuffer + _indexStart;
            std::uint8_t* pixelPEnd = lineBuffer + _indexEnd;
            auto tileSubXStart = _tileSubXStart;
            
            if (*tileP == 0)
            {
                // Empty first tile. Gonna skip it!
                // We might have an offset, so we must take account of it (_tileSubXStart).
                auto initialSkip = tileWidth - tileSubXStart;
                
                tileP++;
                pixelP += initialSkip;
                
                // Automatically skip any other empty tiles.
                while ((pixelP < pixelPEnd) && (*tileP == 0))
                {
                    tileP++;
                    pixelP += tileWidth;
                }
                tileSubXStart = 0;
            }
            const unsigned char* tileDataPLast;
            const unsigned char* tileDataP;
            
            // Automatically skip any empty tiles.
            while ((pixelP < pixelPEnd) && (*tileP == 0))
            {
                tileP++;
                pixelP += tileWidth;
            }
            
            // Configures the initial tile.
            {
                auto tileDataPStart = tileDataRowBase + *tileP * tileSize;
                
                tileDataP = tileDataPStart + tileSubXStart;
                tileDataPLast = tileDataPStart + tileWidth - 1;
            }
            
            // Iterates over all the concerned pixels.
            for (; pixelP < pixelPEnd; pixelP++)
            {
                {
                    auto tilePixel = *tileDataP;
                    
                    if (tilePixel != 0)
                        *pixelP = tilePixel;
                }
                if (tileDataP == tileDataPLast)
                {
                    // Onto the next tile in the row!
                    tileP++;
                    
                    // Automatically skip any empty tiles.
                    while ((pixelP < pixelPEnd) && (*tileP == 0))
                    {
                        tileP++;
                        pixelP += tileWidth;
                    }
                    
                    auto tileDataPStart = tileDataRowBase + *tileP * tileSize;
                    
                    tileDataP = tileDataPStart;
                    tileDataPLast = tileDataPStart + tileWidth - 1;
                }
                else
                    tileDataP++;
            }
        }
        
        
    public: // Coords manipulation.
        // Updates the given grid coordinates so they're clamped inside the box (e.g. negative will be zero'd).
        template<typename CoordsType = int>
        CoordsType clampColumn(CoordsType column) const noexcept
        {
            return std::max<CoordsType>(0, std::min<CoordsType>(column, columns - 1));
        }
        template<typename CoordsType = int>
        CoordsType clampRow(CoordsType row) const noexcept
        {
            return std::max<CoordsType>(0, std::min<CoordsType>(row, rows - 1));
        }
        template<typename CoordsType = int>
        bool areCoordsValid(CoordsType column, CoordsType row) const noexcept
        {
            return (column >= 0) && (column < columns) && (row >= 0) && (row < rows);
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
        
        // Derived from _tilesetData
        const unsigned char* _tileDataRowBase = nullptr;

        const unsigned char* _tilesetData = nullptr;
        Tiles _tiles;
    };
}


#endif // PTUI_BASETILEMAP_HPP