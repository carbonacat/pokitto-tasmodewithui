#ifndef PTUI_PALETTETILEMAP_HPP
#   define PTUI_PALETTETILEMAP_HPP

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
    class PaletteTileMap
    {
        static_assert(lineWidthP > tileWidthP);
        
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
        // - `tilesetImage` is a 8BPP tileset where each tile is 6x6 pixels.
        void setTilesetImage(const unsigned char* tilesetImage) noexcept
        {
            _tilesetImage = tilesetImage;
        }
        
        
    public: // Tiles Access.
        // Changes a given tile in this map.
        // - Safe - If column or row are outside the map, nothing will happen.
        void set(int column, int row, Tile tile) noexcept
        {
            if (areCoordsValid(column, row))
                _tiles[_tileIndex(column, row)] = tile;
        }
        // Same than above, but allows changing the palette offset at the same time.
        void set(int column, int row, Tile tile, std::uint8_t tilePaletteOffset) noexcept
        {
            if (areCoordsValid(column, row))
            {
                auto tileIndex = _tileIndex(column, row);
                
                _tiles[tileIndex] = tile;
                _tiles[tileIndex + paletteIndexOffset] = tilePaletteOffset;
            }
        }
        
        // Only changes the palette offset of the target tile.
        // - Safe - If column or row are outside the map, nothing will happen.
        void setPaletteOffset(int column, int row, std::uint8_t tilePaletteOffset) noexcept
        {
            if (areCoordsValid(column, row))
                _tiles[_tileIndex(column, row) + paletteIndexOffset] = tilePaletteOffset;
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
        void clear(Tile tile = 0, std::uint8_t paletteOffset = 0) noexcept
        {
            std::fill(_tiles.begin(), _tiles.begin() + paletteIndexOffset, tile);
            std::fill(_tiles.begin() + paletteIndexOffset, _tiles.end(), paletteOffset);
        }
        
        // Same than above, on a defined area.
        // - `firstColumn`, `firstRow`, `lastColumn` and `lastRow` will be clamped.
        // - `lastColumn` and `lastRow` are included.
        // - Negative and reversed boxes are considered as empty ones.
        void clear(int firstColumn, int firstRow, int lastColumn, int lastRow, Tile tile = 0) noexcept
        {
            clearUnsafe(clampColumn(firstColumn), clampRow(firstRow), clampColumn(lastColumn), clampRow(lastRow),
                        tile);
        }
        
        // Same than above, but for the palette offset.
        void clearPaletteOffset(int firstColumn, int firstRow, int lastColumn, int lastRow, std::uint8_t tilePaletteOffset = 0) noexcept
        {
            clearPaletteOffsetUnsafe(clampColumn(firstColumn), clampRow(firstRow), clampColumn(lastColumn), clampRow(lastRow),
                                     tilePaletteOffset);
        }
        
        // Shift the whole map by `shiftedColumns` columns and `shiftedRows` rows.
        // - "Introduced" Tiles will be *left as is*.
        void shift(int shiftedColumns, int shiftedRows) noexcept
        {
            shiftUnsafe(0, 0, columns - 1, rows - 1,
                        shiftedColumns, shiftedRows);
        }
        
        // Same than above, on a given area.
        // - `firstColumn`, `firstRow`, `lastColumn` and `lastRow` will be clamped.
        // - `lastColumn` and `lastRow` are included.
        // - Negative and reversed boxes are considered as empty ones.
        void shift(int firstColumn, int firstRow, int lastColumn, int lastRow,
                   int shiftedColumns, int shiftedRows) noexcept
        {
            shiftUnsafe(clampColumn(firstColumn), clampRow(firstRow), clampColumn(lastColumn), clampRow(lastRow),
                        shiftedColumns, shiftedRows);
        }
        
        
    public: // Rendering.
        // Renders a single line.
        
        void fillLine(std::uint8_t* lineBuffer, int y, bool skip) noexcept
        {
            fillLineBaseNew(lineBuffer, y, skip);
        }
        
        void fillLinePaletteNew(std::uint8_t* lineBuffer, int y, bool skip) noexcept
        {
            // Row initialization / change.
            
            if (y == 0)
            {
                _tileY = _tileYStart;
                _tileSubY = _tileSubYStart;
                _tileImageRowBase = _tilesetImage + _tileSubY * tileWidth;
            }
            else
            {
                if (_tileSubY == tileHeight - 1)
                {
                    // Onto the next row!
                    _tileSubY = 0;
                    _tileY++;
                    _tileImageRowBase = _tilesetImage;
                }
                else
                {
                    // Onto the next pixel line of the same tile row!
                    _tileSubY++;
                    _tileImageRowBase += tileWidth;
                }
            }
            if ((skip) || (y < _offsetY) || (_tileY >= static_cast<int>(rows)) || (_indexStart >= _indexEnd))
                return ;
            
            // Scanline rendition.
            
            // Local access is faster than field access.
            auto tileImageRowBase = _tileImageRowBase;
            // Current pixel pointer.
            std::uint8_t* pixelP = lineBuffer + _indexStart;
            // Last pixel pointer.
            std::uint8_t* pixelPEnd = lineBuffer + _indexEnd;
            // Current tile pointer.
            const Tile* tileP = &_tiles[_tileIndex(_tileXStart, _tileY)];
            
            // Is the first tile cut in half?
            if (_tileSubXStart != 0)
            {
                // Is the tile empty?
                if (*tileP == 0)
                    pixelP += tileWidth - _tileSubXStart;
                else
                {
                    auto tileImagePStart = tileImageRowBase + *tileP * tileSize;
                    // Let's render it.
                    // Points to the current pixel in the tile.
                    const unsigned char* tileImageP = tileImagePStart + _tileSubXStart;
                    // Points right after the last pixel in the tile's row.
                    // The line buffer is guaranteed to be greater than a single tile's width by the static_assert on top of the class.
                    const unsigned char* tileImagePEnd = tileImagePStart + tileWidth;
                    // Which palette to use.
                    auto palette = _palette + tileP[paletteIndexOffset];
                    
                    for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                    {
                        auto tilePixel = palette[*tileImageP];
                        
                        if (tilePixel != 0)
                            *pixelP = tilePixel;
                    }
                }
                tileP++;
            }
            
            // Let's render the middle tiles.
            while (pixelP + tileWidth <= pixelPEnd)
            {
                // Let's skip any empty middle tiles.
                while ((*tileP == 0) && (pixelP + tileWidth < pixelPEnd))
                {
                    tileP++;
                    pixelP += tileWidth;
                }
                
                // Middle tile!
                // Let's render it.
                // Points to the current pixel in the tile.
                const unsigned char* tileImageP = tileImageRowBase + *tileP * tileSize;
                // Points right after the last pixel in the tile's row.
                // The line buffer is guaranteed to be greater than a single tile's width by the static_assert on top of the class.
                const unsigned char* tileImagePEnd = tileImageP + tileWidth;
                // Which palette to use.
                auto palette = _palette + tileP[paletteIndexOffset];
                
                // I hope this is unrolled properly.
                for (int i = 0; i < tileWidth; i++)
                {
                    auto tilePixel = palette[*tileImageP];
                    
                    if (tilePixel != 0)
                        *pixelP = tilePixel;
                    pixelP++;
                    tileImageP++;
                }
                tileP++;
            }
            
            // Let's render the last tile, if there is one.
            if ((pixelP < pixelPEnd) && (*tileP != 0))
            {
                // Last tile!
                // Let's render it.
                // Points to the current pixel in the tile.
                const unsigned char* tileImageP = tileImageRowBase + *tileP * tileSize;
                // Points right after the last pixel in the tile's row.
                // The line buffer is guaranteed to be greater than a single tile's width by the static_assert on top of the class.
                const unsigned char* tileImagePEnd = tileImageP + (pixelPEnd - pixelP);
                // Which palette to use.
                auto palette = _palette + tileP[paletteIndexOffset];
                
                for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                {
                    auto tilePixel = palette[*tileImageP];
                    
                    if (tilePixel != 0)
                        *pixelP = tilePixel;
                }
                tileP++;
            }
        }
        
        void fillLinePaletteOld(std::uint8_t* lineBuffer, int y, bool skip) noexcept
        {
            // Row initialization / change.
            
            if (y == 0)
            {
                _tileY = _tileYStart;
                _tileSubY = _tileSubYStart;
                _tileImageRowBase = _tilesetImage + _tileSubY * tileWidth;
            }
            else
            {
                if (_tileSubY == tileHeight - 1)
                {
                    // Onto the next row!
                    _tileSubY = 0;
                    _tileY++;
                    _tileImageRowBase = _tilesetImage;
                }
                else
                {
                    // Onto the next pixel line of the same tile row!
                    _tileSubY++;
                    _tileImageRowBase += tileWidth;
                }
            }
            if ((skip) || (y < _offsetY) || (_tileY >= static_cast<int>(rows)))
                return ;
            
            // Scanline rendition.
            
            auto tileImageRowBase = _tileImageRowBase; // Won't mutate, will be read in a loop -> stored locally.
            
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
            const unsigned char* tileImagePLast;
            const unsigned char* tileImageP;
            
            // Automatically skip any empty tiles.
            while ((pixelP < pixelPEnd) && (*tileP == 0))
            {
                tileP++;
                pixelP += tileWidth;
            }
            
            // Configures the initial tile.
            {
                auto tileImagePStart = tileImageRowBase + *tileP * tileSize;
                
                tileImageP = tileImagePStart + tileSubXStart;
                tileImagePLast = tileImagePStart + tileWidth - 1;
            }
            auto palette = _palette + tileP[paletteIndexOffset];
            
            // Iterates over all the concerned pixels.
            for (; pixelP < pixelPEnd; pixelP++)
            {
                // TODO: Not need to check for both "pixelP < pixelPEnd" and "tileDataP == tileDataPLast" here. Could be done with a carefully crafted end.
                {
                    auto tilePixel = palette[*tileImageP];
                    
                    if (tilePixel != 0)
                        *pixelP = tilePixel;
                }
                if (tileImageP == tileImagePLast)
                {
                    // Onto the next tile in the row!
                    tileP++;
                    palette = _palette + tileP[paletteIndexOffset];
                    
                    // Automatically skip any empty tiles.
                    while ((pixelP < pixelPEnd) && (*tileP == 0))
                    {
                        tileP++;
                        pixelP += tileWidth;
                    }
                    
                    auto tileImagePStart = tileImageRowBase + *tileP * tileSize;
                    
                    tileImageP = tileImagePStart;
                    tileImagePLast = tileImagePStart + tileWidth - 1;
                }
                else
                    tileImageP++;
            }
        }
        
        void fillLineBaseNew(std::uint8_t* lineBuffer, int y, bool skip) noexcept
        {
            // Row initialization / change.
            
            if (y == 0)
            {
                _tileY = _tileYStart;
                _tileSubY = _tileSubYStart;
                _tileImageRowBase = _tilesetImage + _tileSubY * tileWidth;
            }
            else
            {
                if (_tileSubY == tileHeight - 1)
                {
                    // Onto the next row!
                    _tileSubY = 0;
                    _tileY++;
                    _tileImageRowBase = _tilesetImage;
                }
                else
                {
                    // Onto the next pixel line of the same tile row!
                    _tileSubY++;
                    _tileImageRowBase += tileWidth;
                }
            }
            if ((skip) || (y < _offsetY) || (_tileY >= static_cast<int>(rows)) || (_indexStart >= _indexEnd))
                return ;
            
            // Scanline rendition.
            
            // Local access is faster than field access.
            auto tileImageRowBase = _tileImageRowBase;
            // Current pixel pointer.
            std::uint8_t* pixelP = lineBuffer + _indexStart;
            // Last pixel pointer.
            std::uint8_t* pixelPEnd = lineBuffer + _indexEnd;
            // Current tile pointer.
            const Tile* tileP = &_tiles[_tileIndex(_tileXStart, _tileY)];
            
            // Is the first tile cut in half?
            if (_tileSubXStart != 0)
            {
                // Is the tile empty?
                if (*tileP == 0)
                    pixelP += tileWidth - _tileSubXStart;
                else
                {
                    auto tileImagePStart = tileImageRowBase + *tileP * tileSize;
                    // Let's render it.
                    // Points to the current pixel in the tile.
                    const unsigned char* tileImageP = tileImagePStart + _tileSubXStart;
                    // Points right after the last pixel in the tile's row.
                    // The line buffer is guaranteed to be greater than a single tile's width by the static_assert on top of the class.
                    const unsigned char* tileImagePEnd = tileImagePStart + tileWidth;
                    
                    for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                    {
                        auto tilePixel = *tileImageP;
                        
                        if (tilePixel != 0)
                            *pixelP = tilePixel;
                    }
                }
                tileP++;
            }
            
            // Let's render the middle tiles.
            while (pixelP + tileWidth <= pixelPEnd)
            {
                // Let's skip any empty middle tiles.
                while ((*tileP == 0) && (pixelP + tileWidth < pixelPEnd))
                {
                    tileP++;
                    pixelP += tileWidth;
                }
                
                // Middle tile!
                // Let's render it.
                // Points to the current pixel in the tile.
                const unsigned char* tileImageP = tileImageRowBase + *tileP * tileSize;
                // Points right after the last pixel in the tile's row.
                // The line buffer is guaranteed to be greater than a single tile's width by the static_assert on top of the class.
                const unsigned char* tileImagePEnd = tileImageP + tileWidth;
                
                // I hope this is unrolled properly.
                for (int i = 0; i < tileWidth; i++)
                {
                    auto tilePixel = *tileImageP;
                    
                    if (tilePixel != 0)
                        *pixelP = tilePixel;
                    pixelP++;
                    tileImageP++;
                }
                tileP++;
            }
            
            // Let's render the last tile, if there is one.
            if ((pixelP < pixelPEnd) && (*tileP != 0))
            {
                // Last tile!
                // Let's render it.
                // Points to the current pixel in the tile.
                const unsigned char* tileImageP = tileImageRowBase + *tileP * tileSize;
                // Points right after the last pixel in the tile's row.
                // The line buffer is guaranteed to be greater than a single tile's width by the static_assert on top of the class.
                const unsigned char* tileImagePEnd = tileImageP + (pixelPEnd - pixelP);
                
                for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                {
                    auto tilePixel = *tileImageP;
                    
                    if (tilePixel != 0)
                        *pixelP = tilePixel;
                }
                tileP++;
            }
        }
        
        void fillLineBaseOld(std::uint8_t* lineBuffer, int y, bool skip) noexcept
        {
            // Row initialization / change.
            
            if (y == 0)
            {
                _tileY = _tileYStart;
                _tileSubY = _tileSubYStart;
                _tileImageRowBase = _tilesetImage + _tileSubY * tileWidth;
            }
            else
            {
                if (_tileSubY == tileHeight - 1)
                {
                    // Onto the next row!
                    _tileSubY = 0;
                    _tileY++;
                    _tileImageRowBase = _tilesetImage;
                }
                else
                {
                    // Onto the next pixel line of the same tile row!
                    _tileSubY++;
                    _tileImageRowBase += tileWidth;
                }
            }
            if ((skip) || (y < _offsetY) || (_tileY >= static_cast<int>(rows)))
                return ;
            
            // Scanline rendition.
            
            auto tileImageRowBase = _tileImageRowBase; // Won't mutate, will be read in a loop -> stored locally.
            
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
            const unsigned char* tileImagePLast;
            const unsigned char* tileImageP;
            
            // Automatically skip any empty tiles.
            while ((pixelP < pixelPEnd) && (*tileP == 0))
            {
                tileP++;
                pixelP += tileWidth;
            }
            
            // Configures the initial tile.
            {
                auto tileImagePStart = tileImageRowBase + *tileP * tileSize;
                
                tileImageP = tileImagePStart + tileSubXStart;
                tileImagePLast = tileImagePStart + tileWidth - 1;
            }
            
            // Iterates over all the concerned pixels.
            for (; pixelP < pixelPEnd; pixelP++)
            {
                // TODO: Not need to check for both "pixelP < pixelPEnd" and "tileDataP == tileDataPLast" here. Could be done with a carefully crafted end.
                {
                    auto tilePixel = *tileImageP;
                    
                    if (tilePixel != 0)
                        *pixelP = tilePixel;
                }
                if (tileImageP == tileImagePLast)
                {
                    // Onto the next tile in the row!
                    tileP++;
                    
                    // Automatically skip any empty tiles.
                    while ((pixelP < pixelPEnd) && (*tileP == 0))
                    {
                        tileP++;
                        pixelP += tileWidth;
                    }
                    
                    auto tileImagePStart = tileImageRowBase + *tileP * tileSize;
                    
                    tileImageP = tileImagePStart;
                    tileImagePLast = tileImagePStart + tileWidth - 1;
                }
                else
                    tileImageP++;
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
        
        
    public: // Palette manipulation.
    
        // Changes the palette offset for a given color.
        void setPaletteOffset(std::uint8_t color, std::uint8_t offset) noexcept
        {
            _palette[color] = offset;
        }
        
        
    protected: // Unsafe implementations.
        // clear, but columns & rows are considered valid within the grid.
        void clearUnsafe(int firstColumn, int firstRow, int lastColumn, int lastRow, Tile tile = 0) noexcept
        {
            // TODO: Optimizable with cached indexes calculation. (but is it worth it? :P)
            for (int row = firstRow; row <= lastRow; row++)
                for (int column = firstColumn; column <= lastColumn; column++)
                    set(column, row, tile);
        }
        
        // clearPaletteOffset, but columns & rows are considered valid within the grid.
        void clearPaletteOffsetUnsafe(int firstColumn, int firstRow, int lastColumn, int lastRow, std::uint8_t paletteOffset = 0) noexcept
        {
            // TODO: Optimizable with cached indexes calculation. (but is it worth it? :P)
            for (int row = firstRow; row <= lastRow; row++)
                for (int column = firstColumn; column <= lastColumn; column++)
                    setPaletteOffset(column, row, paletteOffset);
        }
        
        // shift, but columns & rows are considered valid within the grid.
        void shiftUnsafe(int firstColumn, int firstRow, int lastColumn, int lastRow,
                         int shiftedColumns, int shiftedRows) noexcept
        {
            int shiftFirstColumn;
            int shiftEndColumn;
            int shiftColumnIncrement;
            int shiftFirstRow;
            int shiftEndRow;
            int shiftRowIncrement;
            
            if (shiftedColumns < 0)
            {
                // We start from the first column.
                shiftFirstColumn = firstColumn;
                shiftEndColumn = lastColumn + 1 + shiftedColumns;
                shiftColumnIncrement = 1;
            }
            else
            {
                // We start from the last column.
                shiftFirstColumn = lastColumn;
                shiftEndColumn = firstColumn + shiftedColumns - 1;
                shiftColumnIncrement = -1;
            }
            if (shiftedRows < 0)
            {
                // We start from the first column.
                shiftFirstRow = firstRow;
                shiftEndRow = lastRow + 1 + shiftedRows;
                shiftRowIncrement = 1;
            }
            else
            {
                // We start from the last column.
                shiftFirstRow = lastRow;
                shiftEndRow = firstRow + shiftedRows - 1;
                shiftRowIncrement = -1;
            }
            for (int row = shiftFirstRow; row != shiftEndRow; row += shiftRowIncrement)
                for (int column = shiftFirstColumn; column != shiftEndColumn; column += shiftColumnIncrement)
                    set(column, row, get(column - shiftedColumns, row - shiftedRows));
        }
    
    private:
        using Tiles = std::array<Tile, columns * rows * 2>; // Doubled as the second part holds the palette offsets.
        
        static constexpr auto paletteIndexOffset = columns * rows;
    
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
        
        // Derived from _tilesetImage
        const unsigned char* _tileImageRowBase = nullptr;

        const unsigned char* _tilesetImage = nullptr;
        Tiles _tiles = {};
        
        std::uint8_t _palette[256] =
        {
            0, 1, 2, 3, 4, 5, 6, 7,
            0, 1, 2, 3, 4, 5+88, 6+88, 7,
            0, 1, 2, 3, 4, 5+136, 6+136, 7,
            0, 1, 2, 3, 4, 5+112, 6+112, 7,
            0, 1, 2, 3, 4, 5, 6, 7,
            0, 0, 2, 3, 4, 5, 6, 7,
        };
    };
}


#endif // PTUI_PALETTETILEMAP_HPP